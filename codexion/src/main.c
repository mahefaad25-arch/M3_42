/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:27:17 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 23:29:59 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_threads(t_sim *sim, pthread_t *monitor, int *created)
{
	int	i;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
		{
			*created = i;
			fprintf(stderr, "codexion: cannot create coder thread\n");
			return (-1);
		}
		i++;
	}
	*created = i;
	if (pthread_create(monitor, NULL, monitor_routine, sim) != 0)
	{
		fprintf(stderr, "codexion: cannot create monitor thread\n");
		return (-1);
	}
	return (0);
}

static void	join_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_params	p;
	t_sim		sim;
	pthread_t	monitor;
	int			created;

	if (parse_args(argc, argv, &p) == -1)
		return (1);
	if (sim_init(&sim, &p) == -1)
		return (fprintf(stderr, "codexion: initialization failed\n"), 1);
	created = 0;
	if (start_threads(&sim, &monitor, &created) == -1)
	{
		set_stop(&sim, -1);
		join_coders(&sim, created);
		sim_destroy(&sim);
		return (1);
	}
	join_coders(&sim, created);
	pthread_join(monitor, NULL);
	sim_destroy(&sim);
	return (0);
}
