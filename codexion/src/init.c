/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 18:36:08 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 18:36:09 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongles(t_sim *sim)
{
	int	i;
	int	nb_dongles;

	nb_dongles = sim->p.nb_coders;
	sim->dongles = malloc(sizeof(t_dongle) * nb_dongles);
	if (sim->dongles == NULL)
		return (-1);
	i = 0;
	while (i < nb_dongles)
	{
		dongle_init(&sim->dongles[i], i);
		i++;
	}
	return (0);
}

static int	init_coders(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->p.nb_coders);
	if (sim->coders == NULL)
		return (-1);
	i = 0;
	while (i < sim->p.nb_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].nb_compiles = 0;
		sim->coders[i].last_compile_start = 0;
		pthread_mutex_init(&sim->coders[i].deadline_lock, NULL);
		sim->coders[i].sim = sim;
		sim->coders[i].left_dongle = i;
		sim->coders[i].right_dongle = (i - 1 + sim->p.nb_coders)
			% sim->p.nb_coders;
		i++;
	}
	return (0);
}

static int	init_resources(t_sim *sim)
{
	int	i;

	if (init_dongles(sim) == -1)
		return (-1);
	if (init_coders(sim) == -1)
	{
		i = 0;
		while (i < sim->p.nb_coders)
			dongle_destroy(&sim->dongles[i++]);
		free(sim->dongles);
		return (-1);
	}
	return (0);
}

int	sim_init(t_sim *sim, t_params *p)
{
	sim->p = *p;
	gettimeofday(&sim->start_time, NULL);
	sim->stop = 0;
	sim->stop_reason_coder = -1;
	pthread_mutex_init(&sim->log_lock, NULL);
	pthread_mutex_init(&sim->stop_lock, NULL);
	return (init_resources(sim));
}

void	sim_destroy(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		dongle_destroy(&sim->dongles[i]);
		pthread_mutex_destroy(&sim->coders[i].deadline_lock);
		i++;
	}
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->stop_lock);
	free(sim->dongles);
	free(sim->coders);
}
