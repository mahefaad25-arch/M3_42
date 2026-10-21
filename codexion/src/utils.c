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

int	is_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_lock);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_lock);
	return (val);
}

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