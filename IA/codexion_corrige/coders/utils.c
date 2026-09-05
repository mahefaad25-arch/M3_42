/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   utils.c                                           :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Returns the difference, in milliseconds, between two timeval structures.
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
** Returns the current timestamp (in ms) relative to the beginning
** of the simulation.
*/
long	get_timestamp_ms(t_sim *sim)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return (time_diff_ms(&sim->start_time, &now));
}

/*
** Prints one state change log line, protected by a mutex so that two
** threads can never interleave their output on the same line.
** Format: "timestamp_in_ms X <message>"
*/
void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	long	ts;

	pthread_mutex_lock(&sim->log_lock);
	/* We do NOT print if the simulation already stopped, except for the
	** burnout message itself which is what triggers the stop. This avoids
	** printing extra lines after the end of the simulation. */
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
** Thread-safe read of the "stop" flag.
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
** Thread-safe write of the "stop" flag. Only the first caller "wins"
** (keeps the first coder_id that triggered the stop, e.g. burnout).
*/
void	set_stop(t_sim *sim, int coder_id)
{
	pthread_mutex_lock(&sim->stop_lock);
	if (sim->stop == 0)
	{
		sim->stop = 1;
		sim->stop_reason_coder = coder_id;
	}
	pthread_mutex_unlock(&sim->stop_lock);
}

