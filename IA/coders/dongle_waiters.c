/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   dongle_waiters.c                                  :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** ------------------------------------------------------------------
** How the scheduler works (kept simple for a beginner algorithm):
**
** Every dongle has its own waiting list: the coders currently
** blocked waiting for it. A tiny fixed-size array per dongle is
** enough, since at most nb_coders threads can ever wait for one
** dongle (only its two neighbours actually can).
**
** When a coder wants a dongle it registers itself in the waiting
** list (arrival timestamp + its deadline = last_compile_start +
** time_to_burnout), then waits until it is the "chosen one"
** according to the scheduler: oldest arrival for fifo, smallest
** deadline for edf (ties broken by arrival order, so nobody can
** starve another coder forever).
** ------------------------------------------------------------------
*/

# define MAX_WAITERS 64

typedef struct s_waiter
{
	int		coder_id;
	long	arrival_ms;
	long	deadline_ms;
}	t_waiter;

/* Static arrays are zero-initialized by the C standard, so every
** dongle's waiting list already starts empty (g_waiters_count[i]
** == 0): no explicit reset function is needed. */
static t_waiter			g_waiters[MAX_WAITERS][MAX_WAITERS];
static int					g_waiters_count[MAX_WAITERS];
static pthread_mutex_t		g_waiters_lock = PTHREAD_MUTEX_INITIALIZER;

/*
** Registers a coder as waiting for a dongle, storing its arrival
** timestamp (for fifo) and its burnout deadline (for edf).
*/
void	waiter_add(int dongle_id, int coder_id, long arrival, long deadline)
{
	int	i;

	pthread_mutex_lock(&g_waiters_lock);
	i = g_waiters_count[dongle_id];
	g_waiters[dongle_id][i].coder_id = coder_id;
	g_waiters[dongle_id][i].arrival_ms = arrival;
	g_waiters[dongle_id][i].deadline_ms = deadline;
	g_waiters_count[dongle_id] = i + 1;
	pthread_mutex_unlock(&g_waiters_lock);
}

/*
** Removes a coder from a dongle's waiting list (called once it
** either obtained the dongle or gave up because the simulation
** stopped).
*/
void	waiter_remove(int dongle_id, int coder_id)
{
	int	i;
	int	n;

	pthread_mutex_lock(&g_waiters_lock);
	n = g_waiters_count[dongle_id];
	i = 0;
	while (i < n)
	{
		if (g_waiters[dongle_id][i].coder_id == coder_id)
		{
			while (i < n - 1)
			{
				g_waiters[dongle_id][i] = g_waiters[dongle_id][i + 1];
				i++;
			}
			g_waiters_count[dongle_id] = n - 1;
			break ;
		}
		i++;
	}
	pthread_mutex_unlock(&g_waiters_lock);
}

/*
** Finds, among the current waiters of a dongle, the index of the
** one that should be served next according to the scheduler policy.
*/
static int	find_best_waiter(t_sim *sim, int dongle_id)
{
	int	i;
	int	n;
	int	best;

	n = g_waiters_count[dongle_id];
	best = -1;
	i = 0;
	while (i < n)
	{
		if (best == -1)
			best = i;
		else if (sim->p.scheduler == CX_SCHED_FIFO)
		{
			if (g_waiters[dongle_id][i].arrival_ms
				< g_waiters[dongle_id][best].arrival_ms)
				best = i;
		}
		else if (g_waiters[dongle_id][i].deadline_ms
				< g_waiters[dongle_id][best].deadline_ms
			|| (g_waiters[dongle_id][i].deadline_ms
					== g_waiters[dongle_id][best].deadline_ms
				&& g_waiters[dongle_id][i].arrival_ms
					< g_waiters[dongle_id][best].arrival_ms))
			best = i;
		i++;
	}
	return (best);
}

/*
** Returns 1 if "coder_id" is the coder that should be served next
** among the current waiters of this dongle, according to the
** scheduling policy. fifo -> smallest arrival_ms. edf -> smallest
** deadline_ms (ties broken by arrival_ms).
*/
int	is_my_turn(t_sim *sim, int dongle_id, int coder_id)
{
	int	best;

	pthread_mutex_lock(&g_waiters_lock);
	best = find_best_waiter(sim, dongle_id);
	pthread_mutex_unlock(&g_waiters_lock);
	if (best == -1)
		return (0);
	return (g_waiters[dongle_id][best].coder_id == coder_id);
}

/*
** Waits at most ~1ms on the dongle's condition variable, then
** returns. We wake up either because dongle_release() broadcasted,
** or because the timeout expired: either way the caller re-checks
** the conditions in its own loop. Using a short timed wait (instead
** of a pure busy loop) keeps the implementation simple while still
** being responsive enough for the required 10ms burnout precision.
*/
void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock)
{
	struct timespec	ts;
	struct timeval	now_tv;

	gettimeofday(&now_tv, NULL);
	ts.tv_sec = now_tv.tv_sec;
	ts.tv_nsec = (now_tv.tv_usec + 1000) * 1000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_nsec -= 1000000000;
		ts.tv_sec += 1;
	}
	pthread_cond_timedwait(cond, lock, &ts);
}
