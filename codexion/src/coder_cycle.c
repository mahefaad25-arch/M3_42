/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:00:00 by student           #+#    #+#             */
/*   Updated: 2026/09/16 00:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** The compiling phase: the coder already holds both dongles here.
** Starting to compile is what resets its burnout timer, so
** last_compile_start is updated before the log line. The compile
** counter is incremented under the same mutex, because the monitor
** thread reads it to know when the simulation is over.
*/
void	do_compile(t_coder *c)
{
	pthread_mutex_lock(&c->deadline_lock);
	c->last_compile_start = get_timestamp_ms(c->sim);
	pthread_mutex_unlock(&c->deadline_lock);
	log_state(c->sim, c->id, "is compiling");
	ft_usleep_ms(c->sim, c->sim->p.time_to_compile);
	pthread_mutex_lock(&c->deadline_lock);
	c->nb_compiles++;
	pthread_mutex_unlock(&c->deadline_lock);
	dongle_release(c->sim, c->first_dongle);
	dongle_release(c->sim, c->second_dongle);
}

/*
** The two solo phases that follow a compile: no dongle is held, so
** nothing is shared and the coder simply waits.
*/
void	do_debug_and_refactor(t_coder *c)
{
	log_state(c->sim, c->id, "is debugging");
	ft_usleep_ms(c->sim, c->sim->p.time_to_debug);
	log_state(c->sim, c->id, "is refactoring");
	ft_usleep_ms(c->sim, c->sim->p.time_to_refactor);
}
