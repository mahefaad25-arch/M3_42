/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:27:36 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 18:27:37 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

/*
** Sleeps for "ms" milliseconds, but in 1 ms slices and checking the
** stop flag between each one. A plain usleep(ms * 1000) would keep
** a coder asleep for a long time after a burnout was detected,
** which would delay the end of the program.
*/
void	ft_usleep_ms(t_sim *sim, long ms)
{
	long	start;

	start = get_timestamp_ms(sim);
	while (get_timestamp_ms(sim) - start < ms)
	{
		if (is_stopped(sim))
			return ;
		usleep(500);
	}
}

/*
** Waits at most ~1 ms on a condition variable, then returns.
** We wake up either because dongle_release() broadcasted, or
** because the timeout expired: in both cases the caller re-checks
** its condition in its own loop. The short timeout is what lets us
** re-evaluate the cooldown, which no broadcast can signal.
*/
void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock)
{
	struct timespec	ts;
	struct timeval	now;

	gettimeofday(&now, NULL);
	ts.tv_sec = now.tv_sec;
	ts.tv_nsec = (now.tv_usec + 1000) * 1000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_nsec -= 1000000000;
		ts.tv_sec += 1;
	}
	pthread_cond_timedwait(cond, lock, &ts);
}
