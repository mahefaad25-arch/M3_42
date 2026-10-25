*This project has been created as part of the 42 curriculum by bramahef.*

# Codexion

## Description

**Codexion** is a concurrency simulation written in C with POSIX threads. Several coders sit around a circular co-working hub. Between each pair of neighbours lies one USB dongle (as many dongles as coders). To compile their quantum code, a coder must hold **two dongles at the same time** (left and right). Each coder loops through:

1. **compile** (holds 2 dongles),
2. **debug**,
3. **refactor**,

and starts again. If a coder does not **start** a compile within `time_to_burnout` ms of their previous compile start (or of the simulation start), they burn out and the simulation stops. The simulation also stops when every coder has compiled at least `number_of_compiles_required` times.

**Goal:** design a fair, deadlock-free and starvation-free resource-sharing protocol, with a dongle cooldown, a pluggable arbitration policy (`fifo` or `edf`), precise burnout detection (< 10 ms) and serialized logs.

**Overview of the implementation**

- one thread per coder (`pthread_create`) + one separate **monitor** thread;
- each dongle has its own mutex, condition variable and a **priority queue (binary heap)** of waiting coders;
- the heap ordering depends on the scheduler: arrival order (FIFO) or earliest deadline (EDF);
- no global variables: all shared state lives in a `t_sim` structure passed to every thread.

### Project structure

```
.
├── Makefile
├── README.md
└── src
    ├── codexion.h        structures and prototypes
    ├── main.c            thread creation / join
    ├── parsing.c         argument parsing and validation
    ├── parsing2.c        argument parsing helpers
    ├── ft_atoi.c         atoi with overflow handling
    ├── init.c            allocation / initialization / destruction
    ├── coder.c           coder routine (compile, debug, refactor)
    ├── dongle.c          dongle take / release / cooldown
    ├── dongle_waiters.c  priority queue (heap) of waiters
    ├── monitor.c         burnout and end-of-simulation detection
    └── utils.c           time, logging, stop flag, sleeping helpers
```

## Instructions

### Compilation

```bash
make        # builds ./codexion  (cc -Wall -Wextra -Werror -pthread)
make clean  # removes object files
make fclean # removes object files and the binary
make re     # fclean + all
```

The Makefile provides `NAME`, `all`, `clean`, `fclean` and `re`, and does not relink needlessly.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | Number of coders (and of dongles) |
| `time_to_burnout` | ms allowed between the start of two compiles (or from the simulation start) before burnout |
| `time_to_compile` | ms spent compiling (2 dongles held) |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring |
| `number_of_compiles_required` | The simulation stops when every coder compiled at least this many times |
| `dongle_cooldown` | ms during which a released dongle cannot be taken again |
| `scheduler` | `fifo` or `edf` (exactly) |

All arguments are mandatory. Negative numbers, non-integers, overflowing values, a wrong number of arguments or a scheduler other than `fifo` / `edf` are rejected with an error message and a non-zero exit status.

### Examples

```bash
./codexion 5 800 200 200 200 7 50 fifo
./codexion 5 800 200 200 200 7 50 edf
./codexion 1 800 200 200 200 3 50 edf     # a single coder: one dongle, burns out
```

### Log format

Every state change is printed as `timestamp_in_ms X message`:

```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
...
1204 3 burned out
```

Messages are: `has taken a dongle`, `is compiling`, `is debugging`, `is refactoring`, `burned out`.

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

A deadlock requires four conditions at once: mutual exclusion, hold and wait, no preemption, circular wait. Mutual exclusion and no preemption are inherent to the problem (a dongle is exclusive and cannot be taken from a coder). Codexion breaks **circular wait**:

- **Asymmetric acquisition order.** Even-numbered coders take their left dongle first, odd-numbered coders take their right dongle first (`set_fork_order`). The chain "everyone holds their first dongle and waits for the second" can therefore not close into a cycle.
- **Desynchronisation.** Even coders sleep 1 ms before their first grab, so that neighbours do not all hit the same dongles at exactly the same instant.
- **Release on failure.** If the second dongle cannot be obtained (for example because the simulation stopped), the first one is released immediately, so no dongle stays held by a coder that gave up.
- **Single coder case.** With one coder there is only one dongle; the coder takes it and can never obtain a second one, so they burn out at `time_to_burnout`. The program handles this without deadlocking: the monitor detects the burnout and the coder thread exits when the stop flag is raised.

### Starvation prevention

- Each dongle owns a **priority queue (binary heap)** implemented by hand (`dongle_waiters.c`, no standard priority queue). A coder wanting a dongle inserts a `t_waiter {coder_id, deadline_ms, order}`.
- A coder may take the dongle only if it is free, its cooldown has elapsed **and** it is at the top of the heap.
- **FIFO**: priority = lowest arrival `order` (a per-dongle counter), so requests are served in arrival order.
- **EDF**: priority = lowest `deadline = last_compile_start + time_to_burnout`, so the coder closest to burnout is served first.
- **Deterministic tie-breaker**: equal deadlines are broken by arrival order, then by coder id, so EDF is fully deterministic even in edge cases.
- Heap operations (`sift_up`, `sift_down`, removal by coder id) are done while holding the dongle mutex.

### Cooldown handling

Each dongle stores `free_since_ms`, the timestamp of its last release. A dongle can be granted only if `now - free_since_ms >= dongle_cooldown`. The initial value is very negative, so a dongle is available at the start. Waiting coders re-check the condition regularly with a short timed wait (1 ms), so the end of a cooldown is noticed even though no other thread signals it.

### Precise burnout detection

- A dedicated **monitor thread** loops every ~1 ms. For each coder it reads `last_compile_start` and `nb_compiles` (under that coder's `deadline_lock`) and compares `now` with `last_compile_start + time_to_burnout`.
- The check period (1 ms) keeps the burnout log well under the required 10 ms delay.
- Coders sleep through `ft_usleep_ms`, which wakes up every 200 µs and checks the stop flag, so a burnout (or the end of the simulation) interrupts all sleeping threads quickly instead of waiting for a whole `time_to_compile` / `time_to_debug` / `time_to_refactor`.
- A coder that already reached the required compile count is never declared burned out.

### Log serialization

- All output goes through `log_state`, which holds `log_lock` for the whole `printf`, so two messages can never be interleaved on one line.
- Once the simulation is stopped, `log_state` drops every message except `burned out`, so nothing is printed after the burnout line.

### Clean termination and memory

- Every thread is joined, then all mutexes and condition variables are destroyed and all heap memory (`dongles`, `coders`) is freed. There are no global variables.
- If a thread creation fails, the stop flag is raised, the already-created threads are joined and everything is destroyed.

## Thread synchronization mechanisms

### Primitives used

| Primitive | Where | Protects / role |
|---|---|---|
| `pthread_mutex_t lock` (per dongle) | `t_dongle` | The dongle state: `taken`, `owner`, `free_since_ms`, and its waiters heap |
| `pthread_cond_t cond` (per dongle) | `t_dongle` | Waiting queue: coders sleep on it until the dongle may be theirs |
| `pthread_mutex_t log_lock` | `t_sim` | Serializes `printf` calls |
| `pthread_mutex_t stop_lock` | `t_sim` | The `stop` flag and `stop_reason_coder` |
| `pthread_mutex_t deadline_lock` (per coder) | `t_coder` | `last_compile_start` and `nb_compiles`, shared between a coder and the monitor |

`pthread_cond_timedwait` (through `short_timed_wait`) is used instead of an unbounded `pthread_cond_wait` so that a waiting coder wakes up at least every millisecond to re-evaluate its conditions: end of cooldown, its turn in the heap, or the stop flag. `pthread_cond_broadcast` is used on release so every waiter re-checks whether it is now first in line.

### How a dongle is acquired

```
dongle_try_take(sim, dongle, coder_id, deadline):
    lock(dongle.lock)
    push {coder_id, deadline, order} in the heap        # under the lock
    while not stopped:
        if dongle free AND cooldown elapsed AND heap top == me: break
        timed_wait(dongle.cond, dongle.lock)             # atomically unlocks while sleeping
    remove me from the heap
    if stopped: unlock; return error
    dongle.taken = 1; dongle.owner = me
    unlock(dongle.lock)
```

The `taken` check, the heap-top check and the assignment of the dongle are done **inside the same critical section**, so two coders can never both believe they own the same dongle.

### How a dongle is released

```
dongle_release:
    lock(dongle.lock)
    taken = 0; owner = -1; free_since_ms = now
    broadcast(dongle.cond)
    unlock(dongle.lock)
```

### Race conditions prevented (examples)

- **Duplicated dongle**: ownership is decided under the dongle mutex, in one critical section (check + take). A second coder arriving in between sees `taken == 1` or is not top of the heap and goes back to waiting.
- **Heap corruption**: insertion, removal and sift operations only happen with the dongle mutex held.
- **Interleaved output**: `log_lock` is held for the whole line.
- **Coder / monitor data race**: `last_compile_start` and `nb_compiles` are written by the coder and read by the monitor, always under the coder's `deadline_lock`. The monitor therefore never reads a half-updated pair of values.
- **Stop flag race**: `stop` is only read through `is_stopped()` and written through `set_stop()`, both under `stop_lock`. `set_stop` only records the first reason, so the first event (burnout or "all compiled enough") wins.
- **Lock ordering**: `log_state` takes `log_lock` then `stop_lock` (through `is_stopped`); no code path takes them in the opposite order. A thread never holds two dongle mutexes at once (each is taken and released inside `dongle_try_take`), so no mutex-level deadlock exists. The monitor takes at most one `deadline_lock` at a time.

### Coder ⇄ monitor communication

- **Coder → monitor**: through `last_compile_start` and `nb_compiles` (protected by `deadline_lock`). The coder updates them when it starts compiling; the monitor polls them.
- **Monitor → coders**: through the shared `stop` flag. When burnout is detected, the monitor calls `set_stop` and then prints `burned out`. Every coder loop, sleep (`ft_usleep_ms`) and dongle wait (`wait_for_turn`) checks `is_stopped()` and exits promptly. Coders blocked on a dongle condition variable wake up within 1 ms thanks to the timed wait.
- **Coder → monitor/other coders at the end**: the coder that finishes the last required compile checks `check_all_compiled_enough` and raises the stop flag; the monitor also checks it each loop.

## Resources

### References

- POSIX Threads Programming (LLNL): https://hpc-tutorials.llnl.gov/posix/
- `man 3 pthread_create`, `pthread_mutex_lock`, `pthread_cond_wait`, `pthread_cond_timedwait`, `pthread_cond_broadcast`, `gettimeofday`, `usleep`
- Dining philosophers problem: https://en.wikipedia.org/wiki/Dining_philosophers_problem
- Coffman conditions (deadlock): https://en.wikipedia.org/wiki/Deadlock_(computer_science)
- Earliest deadline first scheduling: https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling
- Binary heap / priority queue: https://en.wikipedia.org/wiki/Binary_heap
- Helgrind / ThreadSanitizer to detect data races: `valgrind --tool=helgrind ./codexion ...`, `cc -fsanitize=thread`

### Use of AI

AI tools were used as an assistant, not as a replacement for understanding:

- **Code restructuring**: splitting an over-long function (`take_both_dongles`) into smaller ones to respect the Norm.
- **Documentation**: drafting and structuring this README from the subject and the source code.

All generated content was reviewed, tested and is understood by the author. *(Adjust this list so it matches exactly how you used AI.)*
