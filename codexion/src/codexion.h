/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:00:00 by student           #+#    #+#             */
/*   Updated: 2026/09/18 14:17:27 by bramahef         ###   ########.fr       */
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
** Scheduler policy used by a dongle to pick who it serves next.
** CX_FIFO : First In, First Out (smallest arrival timestamp).
** CX_EDF  : Earliest Deadline First
**           deadline = last_compile_start + time_to_burnout
*/
typedef enum e_sched
{
	CX_FIFO,
	CX_EDF
}	t_sched;

/*
** One pending request for a dongle: who asks, when it asked, and
** what its burnout deadline is. This is the element stored in the
** priority queue (binary heap) of each dongle.
*/
typedef struct s_req
{
	int		coder_id;
	long	arrival_ms;
	long	deadline_ms;
}	t_req;

/*
** Binary heap (min-heap) used as the priority queue of a dongle.
** The ordering depends on the scheduler policy: see req_is_before().
** Memory is malloc'ed at init and freed at the end (no leaks).
*/
typedef struct s_heap
{
	t_req	*data;
	int		size;
	int		capacity;
	t_sched	policy;
}	t_heap;

/*
** One dongle on the table. Its whole state (taken/owner/cooldown
** timestamp) AND its waiting queue are protected by "lock".
** Waiting coders sleep on "cond" and are woken when it is released.
*/
typedef struct s_dongle
{
	int				id;
	int				taken;
	int				owner;
	long			free_since_ms;
	t_heap			queue;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

/*
** Simulation parameters, read-only once parsing is done.
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
** Shared state. Passed by pointer to every thread, so that no
** global variable is ever needed (they are forbidden by the subject).
*/
typedef struct s_sim
{
	t_params		p;
	struct timeval	start_time;
	t_dongle		*dongles;
	struct s_coder	*coders;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	stop_lock;
	int				stop;
	int				burned_coder;
}	t_sim;

/*
** Coder phases
*/
typedef enum e_coder_phase
{
	CX_IDLE,
	CX_COMPILING,
	CX_DEBUGGING,
	CX_REFACTORING
}	t_coder_phase;

/*
** One coder = one thread.
** "last_compile_start" is written by the coder and read by the
** monitor, so it has its own mutex.
*/
typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	int				nb_compiles;
	long			last_compile_start;
	pthread_mutex_t	deadline_lock;
	t_sim			*sim;
	int				first_dongle;
	int				second_dongle;
	t_coder_phase	phase;
}	t_coder;

/* parsing.c */
int		parse_args(int argc, char **argv, t_params *p);

/* utils.c */
long	time_diff_ms(struct timeval *start, struct timeval *end);
long	get_timestamp_ms(t_sim *sim);
void	log_state(t_sim *sim, int coder_id, const char *msg);
int		is_stopped(t_sim *sim);
void	set_stop(t_sim *sim, int coder_id);

/* time_utils.c */
void	ft_usleep_ms(t_sim *sim, long ms);
void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock);

/* heap.c */
int		heap_init(t_heap *heap, int capacity, t_sched policy);
void	heap_destroy(t_heap *heap);
int		req_is_before(t_heap *heap, t_req *a, t_req *b);
void	heap_swap(t_req *a, t_req *b);

/* heap_ops.c */
void	heap_sift_up(t_heap *heap, int i);
void	heap_sift_down(t_heap *heap, int i);
void	heap_push(t_heap *heap, int coder_id, long arrival, long deadline);
void	heap_remove(t_heap *heap, int coder_id);
int		heap_top_id(t_heap *heap);

/* dongle.c */
int		dongle_init(t_dongle *d, int id, int capacity, t_sched policy);
void	dongle_destroy(t_dongle *d);
int		dongle_try_take(t_sim *sim, int idx, int coder_id, long deadline);
void	dongle_release(t_sim *sim, int idx);

/* coder.c */
void	*coder_routine(void *arg);

/* coder_cycle.c */
void	do_compile(t_coder *c);
void	do_debug_and_refactor(t_coder *c);

/* monitor.c */
void	*monitor_routine(void *arg);

/* init.c */
int		sim_init(t_sim *sim, t_params *p);
void	sim_destroy(t_sim *sim);

#endif
