*This project has been created as part of the 42 curriculum by login1.*

# Codexion

## Description

Codexion is a multithreaded C simulation inspired by the classic dining
philosophers problem. Instead of philosophers and forks, **coders** sit in a
circular co-working hub and share **USB dongles** placed between them.
Compiling quantum code requires two dongles at the same time (one in each
hand), so the coders compete for a scarce shared resource.

A coder repeats three phases: **compiling** (holding two dongles),
**debugging**, then **refactoring**. If a coder does not manage to start
compiling again within `time_to_burnout` milliseconds of their last compile
(or of the start of the simulation), they **burn out** and the simulation
stops. The simulation also stops successfully once every coder has compiled
at least `number_of_compiles_required` times.

The goal is to practise concurrent programming with POSIX threads: mutexes,
condition variables, deadlock and starvation avoidance, and the
implementation of a fair scheduling policy (FIFO or EDF) backed by a
priority queue.

## Instructions

### Compilation

```bash
make
```

The Makefile sits at the root of the repository and compiles the sources
from `src/` with `cc -Wall -Wextra -Werror -pthread`. It provides the
`all`, `clean`, `fclean` and `re` rules and does not relink unnecessarily.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                      | Meaning                                                               |
|-------------------------------|-----------------------------------------------------------------------|
| `number_of_coders`            | Number of coders, and number of dongles                               |
| `time_to_burnout` (ms)        | Time without compiling before a coder burns out                       |
| `time_to_compile` (ms)        | Time spent compiling, holding two dongles                             |
| `time_to_debug` (ms)          | Time spent debugging                                                  |
| `time_to_refactor` (ms)       | Time spent refactoring                                                |
| `number_of_compiles_required` | Simulation ends once every coder reached this count                   |
| `dongle_cooldown` (ms)        | Time a dongle stays unavailable after being released                  |
| `scheduler`                   | `fifo` or `edf`                                                       |

All arguments are mandatory. Negative numbers, non-integers, zero values and
any scheduler other than `fifo` or `edf` are rejected with an explicit error
message.

### Usage examples

```bash
./codexion 5 3000 150 150 150 3 30 fifo   # ends cleanly, nobody burns out
./codexion 5 3000 150 150 150 3 30 edf    # same, with deadline-based arbitration
./codexion 1 500 200 200 200 3 30 fifo    # a single coder always burns out
./codexion 4 400 200 200 200 10 50 fifo   # parameters too tight: burnout
```

Expected log format:

```
0 1 has taken a dongle
2 1 has taken a dongle
2 1 is compiling
202 1 is debugging
402 1 is refactoring
1505 4 burned out
```

## Technical choices

- **One thread per coder** plus one **monitor thread**, all sharing a single
  `t_sim` structure passed by pointer. No global variable is used anywhere,
  as required by the subject.
- **A binary heap per dongle.** Each dongle owns its own priority queue of
  pending requests (`t_heap` in `src/heap.c` and `src/heap_ops.c`), written
  from scratch since no standard library priority queue may be used.
  `heap_push` and `heap_remove` cost O(log n), and `heap_top_id` gives in
  O(1) the coder the dongle must serve next.
- **The heap ordering encodes the scheduling policy.** With `fifo` the
  smallest arrival timestamp wins; with `edf` the smallest burnout deadline
  wins. Because timestamps are in milliseconds, two deadlines can be equal,
  so the comparison falls back on arrival time and finally on the coder
  number, which is unique. This tie-breaker makes the EDF policy fully
  deterministic, as the subject requires.
- **Single coder case.** With one coder there is one dongle on the table.
  Compiling needs two, so that coder can never compile and burns out. This
  is handled explicitly rather than left to deadlock.

## Blocking cases handled

- **Deadlock prevention (Coffman's conditions).** Every coder acquires its
  two dongles in the same global order: the lower dongle index first. The
  order is computed once in `assign_dongles()`. This breaks the *circular
  wait* condition, so the cycle where every coder holds one dongle and waits
  for the next one can never form. Mutual exclusion, hold-and-wait and
  no-preemption remain, but breaking a single Coffman condition is enough to
  make deadlock impossible.
- **Starvation prevention / liveness.** Requests are never served
  arbitrarily: a coder only takes a dongle when the heap designates it. With
  `edf`, the coder closest to burning out is served first, which is exactly
  what keeps coders alive when parameters are feasible. With `fifo`, the
  oldest request wins, so no coder can be overtaken indefinitely.
- **Cooldown handling.** Each dongle stores `free_since_ms`, set when it is
  released. `can_take_now()` refuses to grant it before
  `dongle_cooldown` milliseconds have elapsed. The check and the acquisition
  happen while holding the dongle's mutex, so they are atomic: two coders
  can never both conclude that the dongle is available.
- **Precise burnout detection.** The monitor thread polls every coder's
  deadline every 500 microseconds and prints the burnout line immediately,
  far inside the 10 ms tolerance. Coders sleep in 1 ms slices
  (`ft_usleep_ms`) and re-check the stop flag, so nobody keeps running or
  logging after the end.
- **Log serialization.** Every log line goes through a single mutex, so two
  messages can never interleave on one line. After the simulation stops,
  `log_state()` only lets the `burned out` line through, so no stray message
  is printed after the end.
- **Clean shutdown.** When the simulation ends, the monitor broadcasts on
  every dongle condition variable (`wake_everyone`), so coders blocked
  waiting for a dongle wake up, leave the queue, and the program can join
  all threads instead of hanging.

## Thread synchronization mechanisms

- **`pthread_mutex_t` per dongle** protects the dongle's whole state:
  `taken`, `owner`, `free_since_ms` *and* its priority queue. Because the
  queue lives inside the dongle and is only ever touched while that mutex is
  held, no separate lock and no global state is needed. Example of a race it
  prevents: without it, two coders could both read `taken == 0` and both
  believe they hold the dongle, duplicating a physical resource.
- **`pthread_cond_t` per dongle**, used through `pthread_cond_timedwait`
  with a ~1 ms timeout. A waiting coder sleeps instead of burning CPU, and
  wakes up either on the `pthread_cond_broadcast` issued by
  `dongle_release()`, or on the timeout. The timeout matters: the cooldown
  expiring is not an event anyone can signal, so it must be re-checked
  periodically. The wait is inside a `while` loop re-testing the full
  condition, which is also what makes spurious wakeups harmless.
- **`deadline_lock` per coder** protects `last_compile_start` and
  `nb_compiles`. Both are written by the coder's own thread and read by the
  monitor thread. Without this mutex the monitor could read a torn or stale
  timestamp and declare a burnout that did not happen, or miss one.
- **`log_lock`** serializes all output across every thread.
- **`stop_lock`** protects the `stop` flag, the thread-safe communication
  channel between the monitor and the coders: the monitor is the only writer
  (`set_stop`), every thread reads it through `is_stopped()`. `set_stop`
  only lets the first caller through, so the coder recorded as burned out is
  really the first one that missed its deadline.

## Resources

- `man pthread_create`, `man pthread_mutex_lock`, `man pthread_cond_wait`,
  `man pthread_cond_timedwait`, `man gettimeofday` — POSIX documentation.
- E. W. Dijkstra, *Hierarchical Ordering of Sequential Processes* (1971) —
  the original dining philosophers problem and the resource-ordering
  solution used here.
- E. G. Coffman, M. Elphick, A. Shoshani, *System Deadlocks* (1971) — the
  four conditions for deadlock.
- *Operating Systems: Three Easy Pieces*, chapters on locks and condition
  variables — free online textbook.
- C. L. Liu and J. W. Layland (1973) — the reference paper on Earliest
  Deadline First scheduling.
- Any standard algorithms course chapter on binary heaps, for the priority
  queue implemented in `src/heap.c` and `src/heap_ops.c`.

### How AI was used

An AI assistant (Claude) was used with the subject PDF as input, for the
following tasks:

- Splitting the project into modules that respect the Norm (at most 5
  functions per file, at most 25 lines per function, no global variables).
- Writing the binary heap used as the per-dongle priority queue, including
  the EDF tie-breaker rule.
- Writing the dongle acquisition logic (mutex + condition variable +
  cooldown) and the fixed-order acquisition strategy that prevents
  deadlocks.
- Writing the monitor thread and the argument validation.
- Drafting this README.

Every generated part was compiled, run under several parameter sets
(including the single-coder and forced-burnout edge cases), and re-read line
by line. The points that required the most attention, and that should be
understood before the defence, are: why fixed-order acquisition removes the
circular wait; why `pthread_cond_timedwait` is needed instead of a plain
`pthread_cond_wait` (the cooldown cannot be signalled); why `nb_compiles` and
`last_compile_start` need their own mutex (cross-thread reads); and how the
heap ordering encodes both scheduling policies. As the subject's AI
instructions recommend, this work should still be reviewed with a peer.
