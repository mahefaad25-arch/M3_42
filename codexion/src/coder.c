/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 00:00:00 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/23 00:00:00 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	get_deadline(t_coder *c)
{
	long	deadline;

	pthread_mutex_lock(&c->deadline_lock);
	deadline = c->last_compile_start + c->sim->p.time_to_burnout;
	pthread_mutex_unlock(&c->deadline_lock);
	return (deadline);
}

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

static int	compile_phase(t_coder *c)
{
	if (take_both_dongles(c) == -1)
		return (-1);
	pthread_mutex_lock(&c->deadline_lock);
	c->last_compile_start = get_timestamp_ms(c->sim);
	pthread_mutex_unlock(&c->deadline_lock);
	log_state(c->sim, c->id, "is compiling");
	ft_usleep_ms(c->sim->p.time_to_compile);
	pthread_mutex_lock(&c->deadline_lock);
	c->nb_compiles++;
	pthread_mutex_unlock(&c->deadline_lock);
	dongle_release(c->sim, c->left_dongle);
	dongle_release(c->sim, c->right_dongle);
	if (check_all_compiled_enough(c->sim))
	{
		set_stop(c->sim, -1);
		return (-1);
	}
	return (0);
}

static int	coder_cycle(t_coder *c)
{
	if (c->sim->p.nb_coders == 1)
	{
		if (dongle_try_take(c->sim, c->left_dongle, c->id,
				c->sim->p.time_to_burnout) == -1)
			return (-1);
		log_state(c->sim, c->id, "has taken a dongle");
		while (!is_stopped(c->sim))
			ft_usleep_ms(1);
		dongle_release(c->sim, c->left_dongle);
		return (-1);
	}
	if (compile_phase(c) == -1)
		return (-1);
	log_state(c->sim, c->id, "is debugging");
	ft_usleep_ms(c->sim->p.time_to_debug);
	log_state(c->sim, c->id, "is refactoring");
	ft_usleep_ms(c->sim->p.time_to_refactor);
	return (0);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = (t_coder *)arg;
	while (!is_stopped(c->sim))
	{
		pthread_mutex_lock(&c->deadline_lock);
		if (c->nb_compiles >= c->sim->p.nb_compiles_required)
		{
			pthread_mutex_unlock(&c->deadline_lock);
			break ;
		}
		pthread_mutex_unlock(&c->deadline_lock);
		if (coder_cycle(c) == -1)
			break ;
	}
	return (NULL);
}
