/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:26:39 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 18:26:40 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

/*
** Thread-safe read of this coder's burnout deadline
** (last_compile_start + time_to_burnout), needed by edf.
*/
static long	get_deadline(t_coder *c)
{
	long	last;

	pthread_mutex_lock(&c->deadline_lock);
	last = c->last_compile_start;
	pthread_mutex_unlock(&c->deadline_lock);
	return (last + c->sim->p.time_to_burnout);
}

/*
** Takes the two dongles needed to compile.
** They are always requested in the same global order (the smaller
** dongle index first, computed once in init.c). This breaks the
** circular wait of Coffman's conditions: no cycle of coders can
** each hold one dongle while waiting for the next one, so the
** classic dining-philosophers deadlock cannot happen.
** Returns 0 on success, -1 if the simulation stopped meanwhile.
*/
static int	take_both_dongles(t_coder *c)
{
	long	deadline;

	deadline = get_deadline(c);
	if (dongle_try_take(c->sim, c->first_dongle, c->id, deadline) == -1)
		return (-1);
	log_state(c->sim, c->id, "has taken a dongle");
	if (dongle_try_take(c->sim, c->second_dongle, c->id, deadline) == -1)
	{
		dongle_release(c->sim, c->first_dongle);
		return (-1);
	}
	log_state(c->sim, c->id, "has taken a dongle");
	return (0);
}

/*
** Special case imposed by the subject: with a single coder there is
** a single dongle on the table. Compiling needs two, so this coder
** can never compile and will inevitably burn out. It takes the only
** dongle, then waits for the monitor to detect its burnout.
*/
static void	single_coder_case(t_coder *c)
{
	if (dongle_try_take(c->sim, c->first_dongle, c->id, get_deadline(c)) == -1)
		return ;
	log_state(c->sim, c->id, "has taken a dongle");
	while (!is_stopped(c->sim))
		usleep(500);
	dongle_release(c->sim, c->first_dongle);
}

/*
** Life of a coder: compile (two dongles), debug, refactor, repeat.
** Loops until the simulation is stopped, either because someone
** burned out or because every coder reached the required number of
** compiles (both decided by the monitor thread).
*/
void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = (t_coder *)arg;
	if (c->sim->p.nb_coders == 1)
	{
		single_coder_case(c);
		return (NULL);
	}
	while (!is_stopped(c->sim))
	{
		if (take_both_dongles(c) == -1)
			break ;
		do_compile(c);
		do_debug_and_refactor(c);
	}
	return (NULL);
}
