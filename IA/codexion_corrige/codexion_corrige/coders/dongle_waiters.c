/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   dongle_waiters.c                                  :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** ------------------------------------------------------------------
** How the scheduler works (kept simple for a beginner algorithm):
**
** Every dongle owns its own waiting list, stored directly inside its
** t_dongle structure (see codexion.h): no global variable is used,
** as required by the subject. At most nb_coders dongles exist, but
** at most MAX_DONGLE_WAITERS (2) coders can ever wait for one single
** dongle at the same time (only its two neighbours can), so the
** per-dongle waiting list is a tiny fixed-size array.
**
** Every access to a dongle's waiting list happens while the caller
** already holds that dongle's mutex (dongle_try_take locks d->lock
** before calling waiter_add/waiter_remove and before checking
** can_take_now/is_my_turn, and only unlocks it, briefly, inside
** short_timed_wait via pthread_cond_timedwait). This is what makes
** it safe to have no dedicated lock for the waiting list itself.
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
** Caller must already hold d->lock.
*/
void	waiter_add(t_dongle *d, int coder_id, long arrival, long deadline)
{
	int	i;

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
** stopped). Caller must already hold d->lock.
*/
void	waiter_remove(t_dongle *d, int coder_id)
{
	int	i;
	int	n;

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
** Caller must already hold d->lock.
*/
static int	find_best_waiter(t_sim *sim, t_dongle *d)
{
	int	i;
	int	n;
	int	best;

	n = d->waiters_count;
	best = -1;
	i = 0;
	while (i < n)
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
** deadline_ms (ties broken by arrival_ms). Caller must already hold
** d->lock.
*/
int	is_my_turn(t_sim *sim, t_dongle *d, int coder_id)
{
	int	best;

	best = find_best_waiter(sim, d);
	if (best == -1)
		return (0);
	return (d->waiters[best].coder_id == coder_id);
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
