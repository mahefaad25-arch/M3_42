/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:27:26 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 18:27:27 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

/*
** Wakes every coder blocked on a dongle, so they can notice that
** the simulation is over and return instead of waiting forever.
*/
static void	wake_everyone(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].lock);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].lock);
		i++;
	}
}

/*
** Checks every coder's deadline. A coder burns out as soon as
** "now" reaches last_compile_start + time_to_burnout. We stop the
** simulation and print the burnout line immediately, which keeps us
** well inside the 10 ms precision required by the subject.
** Returns 1 if a burnout happened.
*/
static int	check_burnout(t_sim *sim, long now)
{
	int		i;
	long	last;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].deadline_lock);
		last = sim->coders[i].last_compile_start;
		pthread_mutex_unlock(&sim->coders[i].deadline_lock);
		if (now - last >= sim->p.time_to_burnout)
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
** Returns 1 once every coder has compiled at least
** number_of_compiles_required times: the simulation then ends
** successfully, without any burnout.
*/
static int	all_compiled_enough(t_sim *sim)
{
	int	i;
	int	done;

	i = 0;
	while (i < sim->p.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].deadline_lock);
		done = sim->coders[i].nb_compiles;
		pthread_mutex_unlock(&sim->coders[i].deadline_lock);
		if (done < sim->p.nb_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

/*
** The monitor thread. It polls the state of every coder every
** 500 us, which is far more often than the 10 ms tolerance, so a
** burnout is always reported in time.
*/
void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (!is_stopped(sim))
	{
		if (check_burnout(sim, get_timestamp_ms(sim)))
			break ;
		if (all_compiled_enough(sim))
		{
			set_completed(sim);
			break ;
		}
		usleep(500);
	}
	wake_everyone(sim);
	return (NULL);
}
