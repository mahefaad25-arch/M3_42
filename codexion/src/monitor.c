/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 01:53:56 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 01:54:26 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_burnout(t_sim *sim, long now)
{
	int		i;
	long	last;
	long	deadline;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].deadline_lock);
		last = sim->coders[i].last_compile_start;
		pthread_mutex_unlock(&sim->coders[i].deadline_lock);
		deadline = last + sim->p.time_to_burnout;
		if (now > deadline)
		{
			set_stop(sim, sim->coders[i].id);
			log_state(sim, sim->coders[i].id, "burned out");
			return (1);
		}
		i++;
	}
	return (0);
}

int	check_all_compiled_enough(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].deadline_lock);
		if (sim->coders[i].nb_compiles < sim->p.nb_compiles_required)
		{
			pthread_mutex_unlock(&sim->coders[i].deadline_lock);
			return (0);
		}
		pthread_mutex_unlock(&sim->coders[i].deadline_lock);
		i++;
	}
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	long	now;

	sim = (t_sim *)arg;
	while (!is_stopped(sim))
	{
		now = get_timestamp_ms(sim);
		if (check_burnout(sim, now))
			break ;
		if (check_all_compiled_enough(sim))
		{
			set_stop(sim, -1);
			break ;
		}
		usleep(1000);
	}
	return (NULL);
}

int	is_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_lock);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_lock);
	return (val);
}

void	set_stop(t_sim *sim, int coder_id)
{
	pthread_mutex_lock(&sim->stop_lock);
	if (sim->stop == 0)
	{
		sim->stop = 1;
		sim->stop_reason_coder = coder_id;
	}
	pthread_mutex_unlock(&sim->stop_lock);
}
