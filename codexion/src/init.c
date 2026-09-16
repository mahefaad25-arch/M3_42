/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:27:11 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 18:27:12 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

/*
** Allocates the dongles. There are as many dongles as coders (one
** between each pair of neighbours), and exactly one dongle when
** there is a single coder.
** Each dongle gets its own priority queue sized for the worst case.
*/
static int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->p.nb_coders);
	if (sim->dongles == NULL)
		return (-1);
	i = 0;
	while (i < sim->p.nb_coders)
	{
		if (dongle_init(&sim->dongles[i], i, sim->p.nb_coders,
				sim->p.scheduler) == -1)
		{
			while (--i >= 0)
				dongle_destroy(&sim->dongles[i]);
			free(sim->dongles);
			return (-1);
		}
		i++;
	}
	return (0);
}

/*
** Gives coder i (0-indexed, number i + 1) its two dongles.
** Left is i, right is (i - 1 + n) % n, which matches the circular
** seating of the subject. We then store them sorted by index
** (first < second) so that every coder acquires them in the same
** global order: this is the deadlock prevention.
*/
static void	assign_dongles(t_coder *c, int i, int n)
{
	int	left;
	int	right;

	left = i;
	right = (i - 1 + n) % n;
	if (n == 1)
	{
		c->first_dongle = 0;
		c->second_dongle = 0;
		return ;
	}
	if (left < right)
	{
		c->first_dongle = left;
		c->second_dongle = right;
	}
	else
	{
		c->first_dongle = right;
		c->second_dongle = left;
	}
}

/*
** Allocates and initializes the coders.
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
		sim->coders[i].sim = sim;
		pthread_mutex_init(&sim->coders[i].deadline_lock, NULL);
		assign_dongles(&sim->coders[i], i, sim->p.nb_coders);
		i++;
	}
	return (0);
}

/*
** Builds the whole simulation state. Everything lives in this
** structure, passed by pointer to the threads, so the program never
** needs a global variable (forbidden by the subject).
*/
int	sim_init(t_sim *sim, t_params *p)
{
	sim->p = *p;
	sim->stop = 0;
	sim->burned_coder = -1;
	gettimeofday(&sim->start_time, NULL);
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
** Frees everything: dongle queues, mutexes, condition variables and
** the two arrays. No memory leak is left behind.
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
