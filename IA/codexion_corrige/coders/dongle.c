/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   dongle.c                                          :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Initializes one dongle: free, no owner, mutex/cond ready. Its
** waiting list (see dongle_waiters.c) starts empty automatically,
** since it is a static array zero-initialized by the C standard.
*/
void	dongle_init(t_dongle *d, int id)
{
	d->id = id;
	d->taken = 0;
	d->owner = -1;
	/* Far enough in the past that (now - free_since_ms) >= cooldown is
	** always true at t=0: a dongle that was never taken yet must be
	** immediately available, not stuck behind a "phantom" cooldown. */
	d->free_since_ms = -1000000000;
	d->waiters_count = 0;
	pthread_mutex_init(&d->lock, NULL);
	pthread_cond_init(&d->cond, NULL);
}

/*
** Destroys a dongle's mutex and condition variable.
*/
void	dongle_destroy(t_dongle *d)
{
	pthread_mutex_destroy(&d->lock);
	pthread_cond_destroy(&d->cond);
}

/*
** Returns 1 if the dongle can be granted to this coder right now:
** it must be free, its cooldown must have elapsed, and it must be
** this coder's turn according to the scheduler.
*/
static int	can_take_now(t_sim *sim, int dongle_idx, int coder_id)
{
	t_dongle	*d;
	long		now;

	d = &sim->dongles[dongle_idx];
	now = get_timestamp_ms(sim);
	return (d->taken == 0
		&& (now - d->free_since_ms) >= sim->p.dongle_cooldown
		&& is_my_turn(sim, dongle_idx, coder_id));
}

/*
** Tries to take one dongle. Blocks (waiting on the condition
** variable) until the dongle is free, its cooldown elapsed and it
** is this coder's turn according to the scheduler.
** "deadline" is the coder's current burnout deadline, used by edf.
** Returns 0 on success, -1 if the simulation stopped meanwhile.
*/
int	dongle_try_take(t_sim *sim, int dongle_idx, int coder_id, long deadline)
{
	t_dongle	*d;
	long		arrival;

	d = &sim->dongles[dongle_idx];
	pthread_mutex_lock(&d->lock);
	arrival = get_timestamp_ms(sim);
	waiter_add(sim, dongle_idx, coder_id, arrival, deadline);
	while (!is_stopped(sim) && !can_take_now(sim, dongle_idx, coder_id))
		short_timed_wait(&d->cond, &d->lock);
	waiter_remove(sim, dongle_idx, coder_id);
	if (is_stopped(sim))
	{
		pthread_mutex_unlock(&d->lock);
		return (-1);
	}
	d->taken = 1;
	d->owner = coder_id;
	pthread_mutex_unlock(&d->lock);
	return (0);
}

/*
** Releases a dongle: marks it free, stores the release timestamp
** (used for the cooldown) and wakes up every coder waiting for it.
*/
void	dongle_release(t_sim *sim, int dongle_idx)
{
	t_dongle	*d;

	d = &sim->dongles[dongle_idx];
	pthread_mutex_lock(&d->lock);
	d->taken = 0;
	d->owner = -1;
	d->free_since_ms = get_timestamp_ms(sim);
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}
