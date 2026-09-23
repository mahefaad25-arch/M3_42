/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:26:33 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/18 14:19:51 by bramahef         ###   ########.fr       */
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
	c->phase = CX_COMPILING;
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
	pthread_mutex_lock(&c->deadline_lock);
	c->phase = CX_DEBUGGING;
	pthread_mutex_unlock(&c->deadline_lock);
	log_state(c->sim, c->id, "is debugging");
	ft_usleep_ms(c->sim, c->sim->p.time_to_debug);
	pthread_mutex_lock(&c->deadline_lock);
	c->phase = CX_REFACTORING;
	pthread_mutex_unlock(&c->deadline_lock);
	log_state(c->sim, c->id, "is refactoring");
	ft_usleep_ms(c->sim, c->sim->p.time_to_refactor);
}

void	release_startup(t_sim *sim)
{
	pthread_mutex_lock(&sim->startup_lock);
	if (!sim->startup_released)
	{
		sim->startup_released = 1;
		pthread_cond_broadcast(&sim->startup_cond);
	}
	pthread_mutex_unlock(&sim->startup_lock);
}

void	wait_startup(t_coder *c)
{
	if (c->id == 1)
		return ;
	pthread_mutex_lock(&c->sim->startup_lock);
	while (!c->sim->startup_released && !is_burned(c->sim))
		pthread_cond_wait(&c->sim->startup_cond, &c->sim->startup_lock);
	pthread_mutex_unlock(&c->sim->startup_lock);
}
