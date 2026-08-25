/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   utils.c                                           :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/25 17:39:33 by username         #+#    #+#              */
/*   Updated: 2026/08/25 17:46:30 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

long	get_time_in_ms(void)
{
	struct timeval	v;

	gettimeofday(&v, NULL);
	return ((v.tv_sec * 1000) + (v.tv_usec / 1000));
}

void	log_status(t_coder *coder, const char *status)
{
	long	current_time;

	pthread_mutex_lock(&coder->rules->end_mutex);
	if (coder->rules->simulation_ended)
	{
		pthread_mutex_unlock(&coder->rules->end_mutex);
		return ;
	}
	pthread_mutex_unlock(&coder->rules->end_mutex);
	pthread_mutex_lock(&coder->rules->log_mutex);
	current_time = get_time_in_ms() - coder->rules->start_time;
	printf("%ld %d %s\n", current_time, coder->id, status);
	pthread_mutex_unlock(&coder->rules->log_mutex);
}
