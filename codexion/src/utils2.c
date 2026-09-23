/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 07:30:55 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/23 07:31:25 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_burned(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_lock);
	val = (sim->stop == 1);
	pthread_mutex_unlock(&sim->stop_lock);
	return (val);
}

int	is_completed(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_lock);
	val = (sim->stop == 2);
	pthread_mutex_unlock(&sim->stop_lock);
	return (val);
}

void	set_completed(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_lock);
	if (sim->stop == 0)
		sim->stop = 2;
	pthread_mutex_unlock(&sim->stop_lock);
}
