*This project has been created as part of the 42 curriculum by mahefaad25.*

# Codexion

## Description

Codexion is a concurrency simulation written in C. Each coder runs in its own
POSIX thread and competes with neighboring coders for shared USB dongles.
Coders need both adjacent dongles to compile, then move through debugging and
refactoring. The simulation arbitrates dongle requests with FIFO or EDF
(Earliest Deadline First) and stops when a coder burns out or every coder has
completed the required number of compiles.

## Instructions

Build from the repository root:

```sh
make
```

The executable is created at `codexion/codexion`. Run it with all eight
mandatory arguments:

```sh
./codexion/codexion number_of_coders time_to_burnout time_to_compile \
  time_to_debug time_to_refactor number_of_compiles_required \
  dongle_cooldown fifo
```

The final argument must be exactly `fifo` or `edf`. Numeric arguments must be
non-negative integers; the number of coders and the burnout time must be
greater than zero. Times are expressed in milliseconds.

The root Makefile also provides:

- `make clean` to remove object files.
- `make fclean` to remove object files and the executable.
- `make re` to rebuild the project.

## Resources

- The POSIX Threads (pthreads) documentation: <https://pubs.opengroup.org/onlinepubs/9699919799/>
- The Linux `pthread_mutex` manual: <https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html>
- The Linux `pthread_cond` manual: <https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html>
- The project subject, `codexion/src/en.subject.pdf`.

AI was used to review the subject requirements, identify compliance gaps,
assist with implementing and checking the FIFO/EDF heap and argument
validation, and draft this documentation. The implementation and test results
must be reviewed and understood by the project author.

## Blocking cases handled

- **Deadlock:** each coder acquires its two dongles in increasing dongle-index
  order, preventing circular wait.
- **Starvation:** FIFO preserves request arrival order; EDF orders requests by
  burnout deadline and uses arrival time and coder ID as deterministic
  tie-breakers.
- **Cooldown:** a released dongle remains unavailable until its cooldown has
  elapsed. A dongle that has never been released is initially available.
- **Burnout:** a dedicated monitor checks coder deadlines at 1 ms intervals
  and stops the simulation when one is reached.
- **Mixed log lines:** a mutex serializes state-change output.
- **One-coder case:** the single coder can take the only dongle but cannot
  compile without a second dongle, so the monitor eventually detects burnout.

## Thread synchronization mechanisms

- Each dongle owns a `pthread_mutex_t` protecting its ownership, cooldown, and
  request heap. Its `pthread_cond_t` wakes waiting coders when the dongle is
  released. Timed waits let a coder recheck cooldown and stop state.
- Each coder has a mutex protecting its compile count and last compile-start
  timestamp. The monitor takes this mutex when reading those fields, avoiding
  data races.
- A simulation mutex protects the stop flag and another protects log output.
  This gives the monitor and coder threads a synchronized way to communicate
  shutdown and prevents concurrent log messages from interleaving.
- FIFO and EDF use a binary min-heap per dongle. Its capacity is two because
  each dongle is shared by at most the two coders on its sides.
