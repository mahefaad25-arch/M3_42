/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   main.c                                            :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Creates one thread per coder plus one monitor thread.
** Returns 0 on success, -1 if pthread_create failed for any thread.
*/
static int	start_threads(t_sim *sim, pthread_t *monitor_th)
{
	int	i;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			fprintf(stderr, "codexion: failed to create coder thread\n");
			return (-1);
		}
		i++;
	}
	if (pthread_create(monitor_th, NULL, monitor_routine, sim) != 0)
	{
		fprintf(stderr, "codexion: failed to create monitor thread\n");
		return (-1);
	}
	return (0);
}

/*
** Waits for every coder thread and the monitor thread to finish.
*/
static void	join_threads(t_sim *sim, pthread_t monitor_th)
{
	int	i;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(monitor_th, NULL);
}

int	main(int argc, char **argv)
{
	t_params	p;
	t_sim		sim;
	pthread_t	monitor_th;

	if (parse_args(argc, argv, &p) == -1)
		return (1);
	if (sim_init(&sim, &p) == -1)
	{
		fprintf(stderr, "codexion: initialization failed (malloc)\n");
		return (1);
	}
	if (start_threads(&sim, &monitor_th) == -1)
	{
		set_stop(&sim, -1);
		join_threads(&sim, monitor_th);
		sim_destroy(&sim);
		return (1);
	}
	join_threads(&sim, monitor_th);
	sim_destroy(&sim);
	return (0);
}
