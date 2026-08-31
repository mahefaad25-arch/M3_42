/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   codexion.h                                        :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
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

/*
** ---- Scheduler policy for the dongles ----
** FIFO : first request in, first served
** EDF  : Earliest Deadline First
** deadline = last_compile_start + time_to_burnout
*/
typedef enum e_sched
{
	CX_SCHED_FIFO,
	CX_SCHED_EDF
}	t_sched;

/*
** ---- One dongle on the table ----
** A dongle can be taken by only one coder at a time.
** After being released it stays unavailable during "cooldown" ms.
** Coders that want the dongle wait on "cond" and are served according
** to the arrival order they registered in the waiting list (a small
** array based priority list, sorted either by arrival time (fifo)
** or by deadline (edf)).
*/
typedef struct s_dongle
{
	int				id;
	int				taken;			/* 1 if currently held by a coder */
	int				owner;			/* coder number currently holding it, -1 if free */
	long			free_since_ms;	/* timestamp (relative) since it became free */
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

/*
** ---- Shared simulation parameters (read only after parsing) ----
*/
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

/*
** ---- Global simulation state shared between all threads ----
*/
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

/*
** ---- One coder (one thread) ----
*/
typedef struct s_coder
{
	int			id;					/* from 1 to nb_coders */
	pthread_t	thread;
	int			nb_compiles;		/* number of times this coder compiled */
	long		last_compile_start;	/* timestamp of the beginning of the
										   last compile, or start of sim */
	pthread_mutex_t	deadline_lock;	/* protects last_compile_start,
										   read by monitor & self */
	t_sim		*sim;
	int			left_dongle;		/* index of left dongle */
	int			right_dongle;		/* index of right dongle */
}	t_coder;

/* parsing.c */
int		parse_args(int argc, char **argv, t_params *p);

/* utils.c */
long	time_diff_ms(struct timeval *start, struct timeval *end);
long	get_timestamp_ms(t_sim *sim);
void	log_state(t_sim *sim, int coder_id, const char *msg);
int		is_stopped(t_sim *sim);
void	set_stop(t_sim *sim, int coder_id);
void	ft_usleep_ms(long ms);

/* dongle.c */
void	dongle_init(t_dongle *d, int id);
void	dongle_destroy(t_dongle *d);
int		dongle_try_take(t_sim *sim, int dongle_idx, int coder_id, long deadline);
void	dongle_release(t_sim *sim, int dongle_idx);

/* dongle_waiters.c */
void	waiter_add(int dongle_id, int coder_id, long arrival, long deadline);
void	waiter_remove(int dongle_id, int coder_id);
int		is_my_turn(t_sim *sim, int dongle_id, int coder_id);
void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock);

/* coder.c */
void	*coder_routine(void *arg);

/* monitor.c */
void	*monitor_routine(void *arg);

/* init.c */
int		sim_init(t_sim *sim, t_params *p);
void	sim_destroy(t_sim *sim);

#endif
