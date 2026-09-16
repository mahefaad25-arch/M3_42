/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:26:50 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 18:26:51 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

/*
** Initializes one dongle: free, no owner, empty priority queue,
** mutex and condition variable ready.
** Returns 0 on success, -1 if the queue could not be allocated.
*/
int	dongle_init(t_dongle *d, int id, int capacity, t_sched policy)
{
	d->id = id;
	d->taken = 0;
	d->owner = -1;
	d->free_since_ms = 0;
	if (heap_init(&d->queue, capacity, policy) == -1)
		return (-1);
	pthread_mutex_init(&d->lock, NULL);
	pthread_cond_init(&d->cond, NULL);
	return (0);
}

/*
** Destroys a dongle: frees its queue, its mutex and its condition
** variable.
*/
void	dongle_destroy(t_dongle *d)
{
	heap_destroy(&d->queue);
	pthread_mutex_destroy(&d->lock);
	pthread_cond_destroy(&d->cond);
}

/*
** Returns 1 when the dongle can be handed to this coder right now:
**   - nobody holds it,
**   - its cooldown has fully elapsed since it was released,
**   - and the scheduler designates this coder (root of the heap).
** The caller must already hold the dongle's mutex.
*/
static int	can_take_now(t_sim *sim, int idx, int coder_id)
{
	t_dongle	*d;
	long		now;

	d = &sim->dongles[idx];
	now = get_timestamp_ms(sim);
	return (d->taken == 0
		&& (now - d->free_since_ms) >= sim->p.dongle_cooldown
		&& heap_top_id(&d->queue) == coder_id);
}

/*
** Requests one dongle and blocks until it is granted.
** The request (arrival time + burnout deadline) is pushed into the
** dongle's priority queue, so the arbitration follows fifo or edf.
** Returns 0 once the dongle is held, -1 if the simulation stopped
** while waiting (the coder then owns nothing).
*/
int	dongle_try_take(t_sim *sim, int idx, int coder_id, long deadline)
{
	t_dongle	*d;

	d = &sim->dongles[idx];
	pthread_mutex_lock(&d->lock);
	heap_push(&d->queue, coder_id, get_timestamp_ms(sim), deadline);
	while (!is_stopped(sim) && !can_take_now(sim, idx, coder_id))
		short_timed_wait(&d->cond, &d->lock);
	heap_remove(&d->queue, coder_id);
	if (is_stopped(sim))
	{
		pthread_cond_broadcast(&d->cond);
		pthread_mutex_unlock(&d->lock);
		return (-1);
	}
	d->taken = 1;
	d->owner = coder_id;
	pthread_mutex_unlock(&d->lock);
	return (0);
}

/*
** Releases a dongle: it becomes free, we remember when (that is
** what starts its cooldown) and we wake every waiting coder so the
** next one designated by the scheduler can take it.
*/
void	dongle_release(t_sim *sim, int idx)
{
	t_dongle	*d;

	d = &sim->dongles[idx];
	pthread_mutex_lock(&d->lock);
	d->taken = 0;
	d->owner = -1;
	d->free_since_ms = get_timestamp_ms(sim);
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}
