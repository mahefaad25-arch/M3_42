#include "codexion.h"

void	dongle_init(t_dongle *d, int id)
{
	d->id = id;
	d->taken = 0;
	d->owner = -1;

	d->free_since_ms = -1000000000;
	d->waiters_count = 0;
	pthread_mutex_init(&d->lock, NULL);
	pthread_cond_init(&d->cond, NULL);
}

void	dongle_destroy(t_dongle *d)
{
	pthread_mutex_destroy(&d->lock);
	pthread_cond_destroy(&d->cond);
}

static int	can_take_now(t_sim *sim, int dongle_idx, int coder_id)
{
	t_dongle	*d;
	long		now;

	d = &sim->dongles[dongle_idx];
	now = get_timestamp_ms(sim);
	return (d->taken == 0
		&& (d->free_since_ms < 0
			|| now - d->free_since_ms >= sim->p.dongle_cooldown)
		&& is_my_turn(sim, d, coder_id));
}

int	dongle_try_take(t_sim *sim, int dongle_idx, int coder_id, long deadline)
{
	t_dongle	*d;
	long		arrival;

	d = &sim->dongles[dongle_idx];
	pthread_mutex_lock(&d->lock);
	arrival = get_timestamp_ms(sim);
	if (waiter_add(sim, d, coder_id, arrival, deadline) == -1)
	{
		pthread_mutex_unlock(&d->lock);
		return (-1);
	}
	while (!is_stopped(sim) && !can_take_now(sim, dongle_idx, coder_id))
		short_timed_wait(&d->cond, &d->lock);
	waiter_remove(sim, d, coder_id);
	if (is_stopped(sim))
	{
		pthread_mutex_unlock(&d->lock);
		return (-1);
	}
	d->taken = 1;
	d->owner = coder_id;
	pthread_mutex_unlock(&d->lock);
	return (0);
}

void	dongle_release(t_sim *sim, int dongle_idx)
{
	t_dongle	*d;

	d = &sim->dongles[dongle_idx];
	pthread_mutex_lock(&d->lock);
	d->taken = 0;
	d->owner = -1;
	d->free_since_ms = get_timestamp_ms(sim);
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}
