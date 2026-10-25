/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_waiters.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 18:35:46 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 20:01:41 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	waiter_has_priority(t_sim *sim, const t_waiter *a,
		const t_waiter *b)
{
	if (sim->p.scheduler == CX_SCHED_FIFO && a->order != b->order)
		return (a->order < b->order);
	if (sim->p.scheduler == CX_SCHED_EDF && a->deadline_ms != b->deadline_ms)
		return (a->deadline_ms < b->deadline_ms);
	if (a->order != b->order)
		return (a->order < b->order);
	return (a->coder_id < b->coder_id);
}

static void	heap_sift_up(t_sim *sim, t_dongle *d, int index)
{
	int			parent;
	t_waiter	tmp;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!waiter_has_priority(sim, &d->waiters[index], &d->waiters[parent]))
			break ;
		tmp = d->waiters[index];
		d->waiters[index] = d->waiters[parent];
		d->waiters[parent] = tmp;
		index = parent;
	}
}

static void	heap_sift_down(t_sim *sim, t_dongle *d, int index)
{
	int			left;
	int			right;
	int			best;
	t_waiter	tmp;

	while (index < d->waiters_count)
	{
		left = index * 2 + 1;
		right = left + 1;
		best = index;
		if (left < d->waiters_count && waiter_has_priority(sim,
				&d->waiters[left], &d->waiters[best]))
			best = left;
		if (right < d->waiters_count && waiter_has_priority(sim,
				&d->waiters[right], &d->waiters[best]))
			best = right;
		if (best == index)
			break ;
		tmp = d->waiters[index];
		d->waiters[index] = d->waiters[best];
		d->waiters[best] = tmp;
		index = best;
	}
}

int	waiter_add(t_sim *sim, t_dongle *d, const t_waiter *request)
{
	int	index;

	index = d->waiters_count;
	if (index >= MAX_DONGLE_WAITERS)
	{
		fprintf(stderr, "codexion: dongle waiter heap capacity exceeded\n");
		return (-1);
	}
	d->waiters[index] = *request;
	d->waiters_count++;
	heap_sift_up(sim, d, index);
	return (0);
}

void	waiter_remove(t_sim *sim, t_dongle *d, int coder_id)
{
	int	i;
	int	last_index;

	i = 0;
	while (i < d->waiters_count && d->waiters[i].coder_id != coder_id)
		i++;
	if (i == d->waiters_count)
		return ;
	last_index = d->waiters_count - 1;
	d->waiters[i] = d->waiters[last_index];
	d->waiters_count--;
	if (i >= d->waiters_count)
		return ;
	if (i > 0 && waiter_has_priority(sim, &d->waiters[i],
			&d->waiters[(i - 1) / 2]))
		heap_sift_up(sim, d, i);
	else
		heap_sift_down(sim, d, i);
}
