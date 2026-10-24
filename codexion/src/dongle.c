/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 00:00:00 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 01:53:39 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	dongle_init(t_dongle *d, int id)
{
	d->id = id;
	d->taken = 0;
	d->owner = -1;
	d->free_since_ms = -1000000000;
	d->waiters_count = 0;
	d->next_order = 0;
	pthread_mutex_init(&d->lock, NULL);
	pthread_cond_init(&d->cond, NULL);
}

void	dongle_destroy(t_dongle *d)
{
	pthread_mutex_destroy(&d->lock);
	pthread_cond_destroy(&d->cond);
}

static int	wait_for_turn(t_sim *sim, t_dongle *d, int coder_id)
{
	long		now;

	while (!is_stopped(sim))
	{
		now = get_timestamp_ms(sim);
		if (d->taken == 0 && (d->free_since_ms < 0
				|| now - d->free_since_ms >= sim->p.dongle_cooldown)
			&& d->waiters_count > 0
			&& d->waiters[0].coder_id == coder_id)
			break ;
		short_timed_wait(&d->cond, &d->lock);
	}
	waiter_remove(sim, d, coder_id);
	if (is_stopped(sim))
		return (-1);
	d->taken = 1;
	d->owner = coder_id;
	return (0);
}

int	dongle_try_take(t_sim *sim, int dongle_idx, int coder_id, long deadline)
{
	t_dongle	*d;
	t_waiter	request;

	d = &sim->dongles[dongle_idx];
	pthread_mutex_lock(&d->lock);
	request.coder_id = coder_id;
	request.deadline_ms = deadline;
	request.order = d->next_order++;
	if (waiter_add(sim, d, &request) == -1)
	{
		pthread_mutex_unlock(&d->lock);
		return (-1);
	}
	if (wait_for_turn(sim, d, coder_id) == -1)
	{
		pthread_mutex_unlock(&d->lock);
		return (-1);
	}
	pthread_mutex_unlock(&d->lock);
	return (0);
}

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
