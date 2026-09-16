/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:00:00 by student           #+#    #+#             */
/*   Updated: 2026/09/16 00:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Difference between two timevals, in milliseconds.
*/
long	time_diff_ms(struct timeval *start, struct timeval *end)
{
	long	sec;
	long	usec;

	sec = end->tv_sec - start->tv_sec;
	usec = end->tv_usec - start->tv_usec;
	return (sec * 1000 + usec / 1000);
}

/*
** Current timestamp in milliseconds, relative to the start of the
** simulation (so the very first log line starts near 0).
*/
long	get_timestamp_ms(t_sim *sim)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return (time_diff_ms(&sim->start_time, &now));
}

/*
** Prints one state change. The log mutex guarantees two threads can
** never interleave their output on the same line.
** Once the simulation is stopped, only the "burned out" line is
** still allowed through: nothing may be printed after the end.
*/
void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	long	ts;

	pthread_mutex_lock(&sim->log_lock);
	if (is_stopped(sim) && strcmp(msg, "burned out") != 0)
	{
		pthread_mutex_unlock(&sim->log_lock);
		return ;
	}
	ts = get_timestamp_ms(sim);
	printf("%ld %d %s\n", ts, coder_id, msg);
	pthread_mutex_unlock(&sim->log_lock);
}

/*
** Thread-safe read of the stop flag.
*/
int	is_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_lock);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_lock);
	return (val);
}

/*
** Thread-safe write of the stop flag. Only the first caller wins,
** so the coder that actually burned out is the one recorded.
*/
void	set_stop(t_sim *sim, int coder_id)
{
	pthread_mutex_lock(&sim->stop_lock);
	if (sim->stop == 0)
	{
		sim->stop = 1;
		sim->burned_coder = coder_id;
	}
	pthread_mutex_unlock(&sim->stop_lock);
}
