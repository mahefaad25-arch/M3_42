#include "codexion.h"

static int	waiter_has_priority(t_sim *sim, const t_waiter *a,
		const t_waiter *b)
{
	if (sim->p.scheduler == CX_SCHED_FIFO && a->arrival_ms != b->arrival_ms)
		return (a->arrival_ms < b->arrival_ms);
	if (sim->p.scheduler == CX_SCHED_EDF && a->deadline_ms != b->deadline_ms)
		return (a->deadline_ms < b->deadline_ms);
	if (a->arrival_ms != b->arrival_ms)
		return (a->arrival_ms < b->arrival_ms);
	return (a->coder_id < b->coder_id);
}

static void	swap_waiters(t_waiter *a, t_waiter *b)
{
	t_waiter	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void	heap_sift_up(t_sim *sim, t_dongle *d, int index)
{
	int	parent;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!waiter_has_priority(sim, &d->waiters[index],
				&d->waiters[parent]))
			break ;
		swap_waiters(&d->waiters[index], &d->waiters[parent]);
		index = parent;
	}
}

static void	heap_sift_down(t_sim *sim, t_dongle *d, int index)
{
	int	left;
	int	right;
	int	best;

	while (index < d->waiters_count)
	{
		left = index * 2 + 1;
		right = left + 1;
		best = index;
		if (left < d->waiters_count
			&& waiter_has_priority(sim, &d->waiters[left],
				&d->waiters[best]))
			best = left;
		if (right < d->waiters_count
			&& waiter_has_priority(sim, &d->waiters[right],
				&d->waiters[best]))
			best = right;
		if (best == index)
			break ;
		swap_waiters(&d->waiters[index], &d->waiters[best]);
		index = best;
	}
}

int	waiter_add(t_sim *sim, t_dongle *d, int coder_id, long arrival,
		long deadline)
{
	int	index;

	index = d->waiters_count;
	if (index >= MAX_DONGLE_WAITERS)
	{
		fprintf(stderr, "codexion: dongle waiter heap capacity exceeded\n");
		return (-1);
	}
	d->waiters[index].coder_id = coder_id;
	d->waiters[index].arrival_ms = arrival;
	d->waiters[index].deadline_ms = deadline;
	d->waiters_count++;
	heap_sift_up(sim, d, index);
	return (0);
}

void	waiter_remove(t_sim *sim, t_dongle *d, int coder_id)
{
	int	i;
	int	last_index;

	i = 0;
	while (i < d->waiters_count && d->waiters[i].coder_id != coder_id)
		i++;
	if (i == d->waiters_count)
		return ;
	last_index = d->waiters_count - 1;
	d->waiters[i] = d->waiters[last_index];
	d->waiters_count--;
	if (i >= d->waiters_count)
		return ;
	if (i > 0 && waiter_has_priority(sim, &d->waiters[i],
			&d->waiters[(i - 1) / 2]))
		heap_sift_up(sim, d, i);
	else
		heap_sift_down(sim, d, i);
}

int	is_my_turn(t_sim *sim, t_dongle *d, int coder_id)
{
	(void)sim;
	return (d->waiters_count > 0 && d->waiters[0].coder_id == coder_id);
}

void	short_timed_wait(pthread_cond_t *cond, pthread_mutex_t *lock)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_nsec += 1000000L;
	if (ts.tv_nsec >= 1000000000L)
	{
		ts.tv_nsec -= 1000000000L;
		ts.tv_sec += 1;
	}
	pthread_cond_timedwait(cond, lock, &ts);
}
