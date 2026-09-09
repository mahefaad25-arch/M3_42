/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   etude_threads.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 06:17:09 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/07 06:25:25 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdio.h>

void	*counter_routine(int counter)
{
	pthread_mutex_t	lock;

	for (int i = 0; i < 100000; i++)
	{
		pthread_mutex_lock(&lock); // Début de section critique
		counter++;
		pthread_mutex_unlock(&lock); // Fin de section critique
	}
	return (NULL);
}

int	main(void)
{
	pthread_t t1, t2;
	pthread_mutex_t lock;
	int count;
	int counter;
	count = 0;

	pthread_mutex_init(&lock, NULL);

	counter = pthread_create(&t1, NULL, counter_routine(count), NULL);
	counter = pthread_create(&t2, NULL, counter_routine(count), NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	pthread_mutex_destroy(&lock);

	printf("Valeur finale du compteur : %u\n", counter); // Toujours 200000
	return (0);
}