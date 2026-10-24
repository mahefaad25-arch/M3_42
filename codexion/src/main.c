/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 18:36:17 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 18:59:58 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_threads(t_sim *sim, pthread_t *monitor_th,
		int *created_coders, int *monitor_started)
{
	int	i;

	i = 0;
	*created_coders = 0;
	*monitor_started = 0;
	while (i < sim->p.nb_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
		{
			fprintf(stderr, "codexion: failed to create coder thread\n");
			return (-1);
		}
		i++;
		*created_coders = i;
	}
	if (pthread_create(monitor_th, NULL, monitor_routine, sim) != 0)
	{
		fprintf(stderr, "codexion: failed to create monitor thread\n");
		return (-1);
	}
	*monitor_started = 1;
	return (0);
}

static void	join_threads(t_sim *sim, int created_coders, int monitor_started,
		pthread_t monitor_th)
{
	int	i;

	i = 0;
	while (i < created_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	if (monitor_started)
		pthread_join(monitor_th, NULL);
}

int	main(int argc, char **argv)
{
	t_params		p;
	t_sim			sim;
	pthread_t		monitor_th;
	int				created_coders;
	int				monitor_started;

	if (parse_args(argc, argv, &p) == -1)
		return (1);
	if (sim_init(&sim, &p) == -1)
	{
		fprintf(stderr, "codexion: initialization failed (malloc)\n");
		return (1);
	}
	if (start_threads(&sim, &monitor_th, &created_coders, &monitor_started)
		== -1)
	{
		set_stop(&sim, -1);
		join_threads(&sim, created_coders, monitor_started, monitor_th);
		sim_destroy(&sim);
		return (1);
	}
	join_threads(&sim, created_coders, monitor_started, monitor_th);
	sim_destroy(&sim);
	return (0);
}
