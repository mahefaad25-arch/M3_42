/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 00:00:00 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 01:53:31 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <time.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

typedef enum e_sched
{
	CX_SCHED_FIFO,
	CX_SCHED_EDF,
	// CX_SCHED_LIFO
}	t_sched;

# define MAX_DONGLE_WAITERS 2

typedef struct s_waiter
{
	int		coder_id;
	long	deadline_ms;
	unsigned long long	order;
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
	unsigned long long	next_order;
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
	t_dongle		*dongles;
	struct s_coder	*coders;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	stop_lock;
	int				stop;
	int				stop_reason_coder;
}	t_sim;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	int				nb_compiles;
	long			last_compile_start;
	pthread_mutex_t	deadline_lock;
	t_sim			*sim;
	int				left_dongle;
	int				right_dongle;
}	t_coder;

int		parse_args(int argc, char **argv, t_params *p);
int		check_all_compiled_enough(t_sim *sim);
int		ft_atoi(const char *str);

long	time_diff_ms(struct timeval *start, struct timeval *end);
long	get_timestamp_ms(t_sim *sim);
void	log_state(t_sim *sim, int coder_id, const char *msg);
int		is_stopped(t_sim *sim);
void	set_stop(t_sim *sim, int coder_id);
void	ft_usleep_ms(long ms);

void	dongle_init(t_dongle *d, int id);
void	dongle_destroy(t_dongle *d);
int		dongle_try_take(t_sim *sim, int dongle_idx, int coder_id,
			long deadline);
void	dongle_release(t_sim *sim, int dongle_idx);

int		waiter_add(t_sim *sim, t_dongle *d, const t_waiter *request);
void	waiter_remove(t_sim *sim, t_dongle *d, int coder_id);
void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock);

void	*coder_routine(void *arg);

void	*monitor_routine(void *arg);

int		sim_init(t_sim *sim, t_params *p);
void	sim_destroy(t_sim *sim);

#endif
