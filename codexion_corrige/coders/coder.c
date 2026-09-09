/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 18:42:23 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/09 18:42:24 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

/*
** Thread-safe read of a coder's current burnout deadline
** (last_compile_start + time_to_burnout).
*/
static long	get_deadline(t_coder *c)
{
	long	last;
	long	deadline;

	pthread_mutex_lock(&c->deadline_lock);
	last = c->last_compile_start;
	pthread_mutex_unlock(&c->deadline_lock);
	deadline = last + c->sim->p.time_to_burnout;
	return (deadline);
}

/*
** Thread-safe update of last_compile_start, called right when the
** coder actually starts compiling (both dongles acquired).
*/
static void	set_compile_start(t_coder *c, long ts)
{
	pthread_mutex_lock(&c->deadline_lock);
	c->last_compile_start = ts;
	pthread_mutex_unlock(&c->deadline_lock);
}

/*
** Acquires both dongles (left then right) in a fixed, deterministic
** order (lower index first) to avoid the classic circular-wait
** deadlock described by Coffman's conditions (the "dining
** philosophers" trap). Returns 0 on success, -1 if the simulation
** stopped while waiting.
*/
static int	take_both_dongles(t_coder *c)
{
	int		first;
	int		second;
	long	deadline;

	deadline = get_deadline(c);
	if (c->left_dongle < c->right_dongle)
	{
		first = c->left_dongle;
		second = c->right_dongle;
	}
	else
	{
		first = c->right_dongle;
		second = c->left_dongle;
	}
	if (dongle_try_take(c->sim, first, c->id, deadline) == -1)
		return (-1);
	log_state(c->sim, c->id, "has taken a dongle");
	if (dongle_try_take(c->sim, second, c->id, deadline) == -1)
	{
		dongle_release(c->sim, first);
		return (-1);
	}
	log_state(c->sim, c->id, "has taken a dongle");
	return (0);
}

static int	take_dongles_single_coder(t_coder *c)
{
	long	deadline;

	deadline = get_deadline(c);
	if (dongle_try_take(c->sim, c->left_dongle, c->id, deadline) == -1)
		return (-1);
	log_state(c->sim, c->id, "has taken a dongle");
	while (!is_stopped(c->sim))
		ft_usleep_ms(1);
	dongle_release(c->sim, c->left_dongle);
	return (-1);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = (t_coder *)arg;
	while (!is_stopped(c->sim))
	{
		if (c->sim->p.nb_coders == 1)
		{
			take_dongles_single_coder(c);
			break ;
		}
		if (take_both_dongles(c) == -1)
			break ;
		set_compile_start(c, get_timestamp_ms(c->sim));
		log_state(c->sim, c->id, "is compiling");
		ft_usleep_ms(c->sim->p.time_to_compile);
		c->nb_compiles++;
		dongle_release(c->sim, c->left_dongle);
		dongle_release(c->sim, c->right_dongle);
		if (is_stopped(c->sim))
			break ;
		log_state(c->sim, c->id, "is debugging");
		ft_usleep_ms(c->sim->p.time_to_debug);
		if (is_stopped(c->sim))
			break ;
		log_state(c->sim, c->id, "is refactoring");
		ft_usleep_ms(c->sim->p.time_to_refactor);
	}
	return (NULL);
}
