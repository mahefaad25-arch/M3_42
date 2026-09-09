/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_waiters.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 18:42:42 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/09 18:42:43 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

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
