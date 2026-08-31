/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   monitor.c                                         :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Checks every coder's deadline (last_compile_start + time_to_burnout).
** If "now" is past a coder's deadline, that coder just burned out:
** we log it immediately (within the required 10ms precision) and
** stop the whole simulation.
** Returns 1 if a burnout was detected, 0 otherwise.
*/
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
		if (now >= deadline)
		{
			set_stop(sim, sim->coders[i].id);
			log_state(sim, sim->coders[i].id, "burned out");
			return (1);
		}
		i++;
	}
	return (0);
}

/*
** Checks whether every coder already compiled at least
** number_of_compiles_required times. If so, the simulation must
** stop successfully (no burnout).
*/
static int	check_all_compiled_enough(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		if (sim->coders[i].nb_compiles < sim->p.nb_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

/*
** The monitor thread: watches over all coders, sleeping in very
** short slices (1ms) so that a burnout is always detected and
** logged within the 10ms tolerance window required by the subject.
*/
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
