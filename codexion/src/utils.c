/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 00:00:00 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 01:54:05 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	time_diff_ms(struct timeval *start, struct timeval *end)
{
	long	sec;
	long	usec;

	sec = end->tv_sec - start->tv_sec;
	usec = end->tv_usec - start->tv_usec;
	return (sec * 1000 + usec / 1000);
}

long	get_timestamp_ms(t_sim *sim)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return (time_diff_ms(&sim->start_time, &now));
}

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

void	ft_usleep_ms(long ms)
{
	struct timeval	start;
	struct timeval	now;

	gettimeofday(&start, NULL);
	while (1)
	{
		gettimeofday(&now, NULL);
		if (time_diff_ms(&start, &now) >= ms)
			break ;
		usleep(200);
	}
}

void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock)
{
	struct timespec	ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += 1000000L;
	if (ts.tv_nsec >= 1000000000L)
	{
		ts.tv_nsec -= 1000000000L;
		ts.tv_sec += 1;
	}
	pthread_cond_timedwait(cond, lock, &ts);
}
