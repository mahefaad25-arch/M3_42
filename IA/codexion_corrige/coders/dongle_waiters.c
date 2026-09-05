/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   dongle_waiters.c                                  :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/09/05 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** ------------------------------------------------------------------
** How the scheduler works (kept simple for a beginner algorithm):
**
** Every dongle keeps its own waiting list directly inside its
** t_dongle structure (see codexion.h): the coders currently blocked
** waiting for it. Since at most nb_coders dongles ever exist and at
** most their two neighbours can wait on any single one of them, a
** tiny fixed-size array per dongle (MAX_DONGLE_WAITERS) is always
** enough, no matter how many coders the simulation has.
**
** NO GLOBAL VARIABLE is used: the waiting list is allocated together
** with the dongle array (init.c), and is protected by the dongle's
** own mutex ("d->lock"), which every caller already holds while it
** registers, waits, and unregisters itself (see dongle_try_take() in
** dongle.c). This also fixes the previous fixed-size-of-64 bug: the
** list now scales exactly with nb_coders instead of a hardcoded cap.
**
** When a coder wants a dongle it registers itself in the waiting
** list (arrival timestamp + its deadline = last_compile_start +
** time_to_burnout), then waits until it is the "chosen one"
** according to the scheduler: oldest arrival for fifo, smallest
** deadline for edf (ties broken by arrival order, so nobody can
** starve another coder forever).
** ------------------------------------------------------------------
*/

/*
** Registers a coder as waiting for a dongle, storing its arrival
** timestamp (for fifo) and its burnout deadline (for edf).
** Caller must already hold sim->dongles[dongle_id].lock.
*/
void	waiter_add(t_sim *sim, int dongle_id, int coder_id,
		long arrival, long deadline)
{
	t_dongle	*d;
	int			i;

	d = &sim->dongles[dongle_id];
	i = d->waiters_count;
	if (i >= MAX_DONGLE_WAITERS)
		return ;
	d->waiters[i].coder_id = coder_id;
	d->waiters[i].arrival_ms = arrival;
	d->waiters[i].deadline_ms = deadline;
	d->waiters_count = i + 1;
}

/*
** Removes a coder from a dongle's waiting list (called once it
** either obtained the dongle or gave up because the simulation
** stopped). Caller must already hold sim->dongles[dongle_id].lock.
*/
void	waiter_remove(t_sim *sim, int dongle_id, int coder_id)
{
	t_dongle	*d;
	int			i;
	int			n;

	d = &sim->dongles[dongle_id];
	n = d->waiters_count;
	i = 0;
	while (i < n)
	{
		if (d->waiters[i].coder_id == coder_id)
		{
			while (i < n - 1)
			{
				d->waiters[i] = d->waiters[i + 1];
				i++;
			}
			d->waiters_count = n - 1;
			break ;
		}
		i++;
	}
}

/*
** Finds, among the current waiters of a dongle, the index of the
** one that should be served next according to the scheduler policy.
*/
static int	find_best_waiter(t_sim *sim, int dongle_id)
{
	t_dongle	*d;
	int			i;
	int			best;

	d = &sim->dongles[dongle_id];
	best = -1;
	i = 0;
	while (i < d->waiters_count)
	{
		if (best == -1)
			best = i;
		else if (sim->p.scheduler == CX_SCHED_FIFO)
		{
			if (d->waiters[i].arrival_ms < d->waiters[best].arrival_ms)
				best = i;
		}
		else if (d->waiters[i].deadline_ms < d->waiters[best].deadline_ms
				|| (d->waiters[i].deadline_ms == d->waiters[best].deadline_ms
					&& d->waiters[i].arrival_ms < d->waiters[best].arrival_ms))
			best = i;
		i++;
	}
	return (best);
}

/*
** Returns 1 if "coder_id" is the coder that should be served next
** among the current waiters of this dongle, according to the
** scheduling policy. fifo -> smallest arrival_ms. edf -> smallest
** deadline_ms (ties broken by arrival_ms).
*/
int	is_my_turn(t_sim *sim, int dongle_id, int coder_id)
{
	int	best;

	best = find_best_waiter(sim, dongle_id);
	if (best == -1)
		return (0);
	return (sim->dongles[dongle_id].waiters[best].coder_id == coder_id);
}

/*
** Waits at most ~1ms on the dongle's condition variable, then
** returns. We wake up either because dongle_release() broadcasted,
** or because the timeout expired: either way the caller re-checks
** the conditions in its own loop. Using a short timed wait (instead
** of a pure busy loop) keeps the implementation simple while still
** being responsive enough for the required 10ms burnout precision.
*/
void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock)
{
	struct timespec	ts;
	struct timeval	now_tv;

	gettimeofday(&now_tv, NULL);
	ts.tv_sec = now_tv.tv_sec;
	ts.tv_nsec = (now_tv.tv_usec + 1000) * 1000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_nsec -= 1000000000;
		ts.tv_sec += 1;
	}
	pthread_cond_timedwait(cond, lock, &ts);
}
