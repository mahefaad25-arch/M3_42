/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utile1.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 17:56:36 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/25 18:12:33 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	wait_for_dongle_slot(t_coder *coder, t_dongle *dongle)
{
	long			wait_time_ms;
	struct timespec	ts;

	wait_time_ms = coder->rules->dongle_cooldown;
	wait_time_ms -= get_time_in_ms() - dongle->last_released_time;
	if (wait_time_ms > 0)
	{
		ts.tv_sec = time(NULL) + (wait_time_ms / 1000);
		ts.tv_nsec = (wait_time_ms % 1000) * 1000000L;
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		return ;
	}
	pthread_cond_wait(&dongle->cond, &dongle->mutex);
}
