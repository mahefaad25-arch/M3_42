/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 18:42:29 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/09 18:42:39 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

typedef enum e_sched
{
	CX_SCHED_FIFO,
	CX_SCHED_EDF
}	t_sched;

/*
** ---- Waiting list entry for one dongle ----
** At most two coders can ever wait for a given dongle at the same
** time: its left neighbour and its right neighbour (the topology
** guarantees this, see init_coders). We size the waiting list to
** that exact bound instead of a value unrelated to nb_coders.
*/
# define MAX_DONGLE_WAITERS 2

typedef struct s_waiter
{
	int		coder_id;
	long	arrival_ms;
	long	deadline_ms;
}	t_waiter;

typedef struct s_dongle
{
	int				id;
	int				taken;
	int				owner;
	long			free_since_ms;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	t_waiter		waiters[MAX_DONGLE_WAITERS];
	int				waiters_count;
}	t_dongle;

typedef struct s_params
{
	int		nb_coders;
	long	time_to_burnout;
	long	time_to_compile;
	long	time_to_debug;
	long	time_to_refactor;
	int		nb_compiles_required;
	long	dongle_cooldown;
	t_sched	scheduler;
}	t_params;

typedef struct s_sim
{
	t_params		p;
	struct timeval	start_time;
	t_dongle		*dongles;			/* array of nb_coders dongles */
	struct s_coder	*coders;			/* array of nb_coders coders */
	pthread_mutex_t	log_lock;			/* protects printf so lines never mix */
	pthread_mutex_t	stop_lock;			/* protects "stop" flag */
	int				stop;				/* 1 -> simulation must stop */
	int				stop_reason_coder;	/* who burned out (for info) */
}	t_sim;

typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	int			nb_compiles;
	long		last_compile_start;
	pthread_mutex_t	deadline_lock;
	t_sim		*sim;
	int			left_dongle;
	int			right_dongle;
}	t_coder;

int		parse_args(int argc, char **argv, t_params *p);

long	time_diff_ms(struct timeval *start, struct timeval *end);
long	get_timestamp_ms(t_sim *sim);
void	log_state(t_sim *sim, int coder_id, const char *msg);
int		is_stopped(t_sim *sim);
void	set_stop(t_sim *sim, int coder_id);
void	ft_usleep_ms(long ms);

void	dongle_init(t_dongle *d, int id);
void	dongle_destroy(t_dongle *d);
int		dongle_try_take(t_sim *sim, int dongle_idx, int coder_id, long deadline);
void	dongle_release(t_sim *sim, int dongle_idx);


void	waiter_add(t_dongle *d, int coder_id, long arrival, long deadline);
void	waiter_remove(t_dongle *d, int coder_id);
int		is_my_turn(t_sim *sim, t_dongle *d, int coder_id);
void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock);

void	*coder_routine(void *arg);

void	*monitor_routine(void *arg);

int		sim_init(t_sim *sim, t_params *p);
void	sim_destroy(t_sim *sim);

#endif
