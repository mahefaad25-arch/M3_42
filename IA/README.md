*This project has been created as part of the 42 curriculum by student.*

# Codexion

## Description

Codexion is a multithreaded C simulation inspired by the classic "dining
philosophers" problem. Instead of philosophers and forks, we have **coders**
sitting in a circular co-working hub, sharing **USB dongles** stored on a
table between them. Compiling "quantum code" requires two dongles at once
(one in each hand), so coders must compete fairly for this scarce shared
resource without ever getting stuck (deadlock) or left starving too long
(burnout).

The goal of the project is to practice concurrent programming in C using
POSIX threads (`pthread`): mutexes, condition variables, and the design of a
fair scheduling policy (FIFO or EDF) for a shared resource with a cooldown
period.

Each coder repeatedly goes through three phases:

1. **Compiling** — needs both the left and right dongle at the same time.
2. **Debugging** — solo activity, no dongle needed.
3. **Refactoring** — solo activity, no dongle needed.

If a coder does not manage to start compiling again before
`time_to_burnout` milliseconds have passed since their last compile (or
since the start of the simulation), they **burn out** and the whole
simulation stops.

## Instructions

### Compilation

```bash
cd coders
make
```

This produces the `codexion` executable. The Makefile follows the 42 Norm
requirements: it uses `cc` with `-Wall -Wextra -Werror -pthread`, and
provides the `all`, `clean`, `fclean` and `re` rules.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                     | Meaning                                                              |
|-------------------------------|-----------------------------------------------------------------------|
| `number_of_coders`            | Number of coders (and number of dongles)                             |
| `time_to_burnout` (ms)        | Time without compiling before a coder burns out                      |
| `time_to_compile` (ms)        | Time spent compiling (holding both dongles)                          |
| `time_to_debug` (ms)          | Time spent debugging                                                 |
| `time_to_refactor` (ms)       | Time spent refactoring                                               |
| `number_of_compiles_required` | Simulation stops successfully once every coder reached this count    |
| `dongle_cooldown` (ms)        | Time a dongle stays unavailable after being released                 |
| `scheduler`                   | `fifo` or `edf` — how dongles are granted when several coders compete |

Example:

```bash
./codexion 5 800 200 200 200 3 50 fifo
```

All arguments are mandatory and validated: negative numbers, non-integers,
or an unknown scheduler value are rejected with an explicit error message
and the program exits without running the simulation.

### Example output

```
0 1 has taken a dongle
2 1 has taken a dongle
2 1 is compiling
202 1 is debugging
402 1 is refactoring
```

## Blocking cases handled

- **Deadlock prevention (Coffman's conditions):** each coder always
  acquires its two dongles in the same global order (the dongle with the
  smaller index first). This breaks the *circular wait* condition that
  causes the classic dining-philosophers deadlock, where every coder holds
  one dongle and waits forever for the other.
- **Starvation prevention:** each dongle keeps a waiting list of the
  coders currently requesting it. With `fifo`, the oldest request is
  always served first; with `edf`, the request with the closest burnout
  deadline is served first (ties broken by arrival order). This guarantees
  that, given feasible parameters, no coder waits forever behind others.
- **Cooldown handling:** every dongle remembers the timestamp at which it
  was released. A coder is only allowed to take it again once
  `dongle_cooldown` milliseconds have elapsed, checked under the dongle's
  own mutex so the check-and-take sequence stays atomic.
- **Precise burnout detection:** a dedicated monitor thread polls every
  coder's deadline every millisecond and immediately logs and stops the
  simulation as soon as a deadline is missed, respecting the 10 ms
  precision required by the subject.
- **Log serialization:** all log lines go through a single global mutex
  (`log_lock`) so two threads can never interleave their output within the
  same line.

## Thread synchronization mechanisms

- **`pthread_mutex_t` per dongle** protects its state (`taken`, `owner`,
  `free_since_ms`) so two coders can never believe they both hold the same
  dongle at once — every read/modify/write sequence happens while the
  mutex is locked.
- **`pthread_cond_t` per dongle**, used with `pthread_cond_timedwait`,
  lets a waiting coder sleep instead of busy-looping while still waking up
  quickly (every ~1 ms) to re-check whether the dongle became available,
  its cooldown elapsed, and it is its turn. `pthread_cond_broadcast` is
  called by `dongle_release()` so every waiter re-evaluates the condition
  as soon as a dongle is freed.
- **A dedicated waiting-list mutex** (`g_waiters_lock` in
  `dongle_waiters.c`) protects the small per-dongle arrival/deadline lists
  used to decide, under `fifo` or `edf`, which coder should be served
  next — this is what makes the arbitration policy fair and race-free.
- **`deadline_lock` per coder** protects `last_compile_start`, which is
  written by the coder's own thread and read by the monitor thread; this
  prevents a race where the monitor could read a half-updated timestamp.
- **`log_lock`** serializes all `printf` calls across every thread.
- **`stop_lock`** protects the global `stop` flag, checked by every coder
  and the monitor to know when the simulation must end (on burnout or once
  every coder reached `number_of_compiles_required`).

## Resources

- `man pthread_create`, `man pthread_mutex_lock`, `man pthread_cond_wait`,
  `man pthread_cond_timedwait` — official POSIX threads documentation.
- *The Dining Philosophers Problem*, E. W. Dijkstra (1965) — the classic
  concurrency problem this project is inspired by.
- *Operating Systems: Three Easy Pieces*, chapters on concurrency (locks,
  condition variables, semaphores) — free online textbook, useful to
  understand mutexes/condition variables from first principles.
- Wikipedia — *Earliest deadline first scheduling* and *Coffman
  conditions*, for the theoretical background behind the `edf` scheduler
  and deadlock prevention.

### How AI was used

An AI assistant (Claude) was used to help design and write the initial
version of this project, given the subject PDF as input. Specifically, it
was used for:

- Translating the subject's requirements into a C project structure
  (splitting the code into `parsing.c`, `dongle.c`, `dongle_waiters.c`,
  `coder.c`, `monitor.c`, `init.c`, `main.c`, `utils.c`) that respects the
  42 Norm (max 5 functions per file, max ~25 lines per function).
- Writing the dongle acquisition/release logic (mutex + condition
  variable + cooldown + fifo/edf fair arbitration) and the fixed-order
  acquisition strategy used to avoid deadlocks.
- Writing the burnout-detection monitor thread and the argument-parsing
  validation.
- Drafting this README.

All AI-generated code was then compiled, run, and manually re-read line by
line to make sure every part is understood: in particular the reasoning
behind fixed-order dongle acquisition (why it prevents deadlocks), the use
of `pthread_cond_timedwait` instead of a plain `pthread_cond_wait` (needed
to re-check the cooldown and the scheduler's turn periodically), and the
per-coder `deadline_lock` (needed because the monitor thread reads a value
that the coder thread writes). As recommended by the subject's AI
instructions, this project should still be discussed with a peer before
being defended, to catch anything a solo review might have missed.
