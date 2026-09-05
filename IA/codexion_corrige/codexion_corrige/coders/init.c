/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   init.c                                            :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Allocates and initializes every dongle. As stated in the subject:
** "There is one dongle between each pair of coders", so with N
** coders sitting in a circle there are N dongles (one on each side).
** With a single coder there must be exactly one dongle on the table.
*/
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

/*
** Allocates and initializes every coder. Coder i (0-indexed) has
** number i + 1. Its left dongle is at index i, its right dongle is
** at index (i - 1 + nb_coders) % nb_coders, which reproduces the
** classic circular seating described in the subject: coder 1 sits
** next to coder number_of_coders, coder N sits between N-1 and N+1.
** In the single-coder case both hands reach for the very same
** unique dongle.
*/
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
		if (sim->p.nb_coders == 1)
		{
			sim->coders[i].left_dongle = 0;
			sim->coders[i].right_dongle = 0;
		}
		else
		{
			sim->coders[i].left_dongle = i;
			sim->coders[i].right_dongle
				= (i - 1 + sim->p.nb_coders) % sim->p.nb_coders;
		}
		i++;
	}
	return (0);
}

/*
** Sets up the whole simulation state: parameters, start time,
** dongles, coders and the mutexes protecting shared resources
** (log output and the stop flag).
** Returns 0 on success, -1 on allocation failure.
*/
int	sim_init(t_sim *sim, t_params *p)
{
	sim->p = *p;
	gettimeofday(&sim->start_time, NULL);
	sim->stop = 0;
	sim->stop_reason_coder = -1;
	pthread_mutex_init(&sim->log_lock, NULL);
	pthread_mutex_init(&sim->stop_lock, NULL);
	if (init_dongles(sim) == -1)
		return (-1);
	if (init_coders(sim) == -1)
	{
		free(sim->dongles);
		return (-1);
	}
	return (0);
}

/*
** Releases every resource allocated by sim_init: destroys all
** mutexes/condition variables and frees the dongle and coder arrays.
*/
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
