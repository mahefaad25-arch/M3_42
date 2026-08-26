/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 08:36:44 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/26 08:37:07 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	init_dongles(t_dongle **dongles, int nb_coders)
{
	int	index;

	if (nb_coders <= 0)
		return (1);
	*dongles = malloc(sizeof(t_dongle) * (size_t)nb_coders);
	if (!*dongles)
		return (1);
	index = 0;
	while (index < nb_coders)
	{
		(*dongles)[index].id = index + 1;
		(*dongles)[index].is_busy = false;
		(*dongles)[index].last_released_time = 0;
		if (pthread_mutex_init(&(*dongles)[index].mutex, NULL) != 0
			|| pthread_cond_init(&(*dongles)[index].cond, NULL) != 0)
			return (1);
		(*dongles)[index].queue = create_heap(nb_coders + 1);
		index++;
	}
	return (0);
}

void	assign_dongles_to_coders(t_coder *coders, t_dongle *dongles,
		int nb_coders)
{
	int	index;

	index = 0;
	while (index < nb_coders)
	{
		coders[index].id = index + 1;
		if (nb_coders == 1)
		{
			coders[index].left_dongle = &dongles[0];
			coders[index].right_dongle = NULL;
		}
		else
		{
			coders[index].left_dongle = &dongles[index];
			coders[index].right_dongle = &dongles[(index + 1) % nb_coders];
		}
		index++;
	}
}

static bool	is_cooldown_active(t_dongle *dongle, t_rules *rules)
{
	long	current_time;

	if (rules->dongle_cooldown == 0)
		return (false);
	current_time = get_time_in_ms();
	return (((current_time
				- dongle->last_released_time) < (rules->dongle_cooldown)));
}

static bool	should_take_dongle(t_coder *coder, t_dongle *dongle)
{
	if (!dongle || dongle->is_busy || dongle->queue.size <= 0)
		return (false);
	if (!dongle->queue.elements[0].coder)
		return (false);
	if (dongle->queue.elements[0].coder->id != coder->id)
		return (false);
	if (is_cooldown_active(dongle, coder->rules))
		return (false);
	return (true);
}

static bool	should_wait_for_cooldown(t_coder *coder, t_dongle *dongle)
{
	if (!dongle || dongle->is_busy || dongle->queue.size <= 0)
		return (false);
	if (!dongle->queue.elements[0].coder)
		return (false);
	if (dongle->queue.elements[0].coder->id != coder->id)
		return (false);
	return (is_cooldown_active(dongle, coder->rules));
}
