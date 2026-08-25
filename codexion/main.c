/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   main.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/07 10:02:10 by username         #+#    #+#              */
/*   Updated: 2026/08/25 17:46:27 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	main(int argc, char **argv)
{
	t_rules	rules;

	if (init_rules(argc, argv, &rules) != 0)
	{
		printf("Error: Invalid arguments.\n");
		return (1);
	}
	rules.start_time = get_time_in_ms();
	printf("--- Configuration chargée avec succès ---\n");
	printf("Coders: %d | Burnout: %ldms | Compile: %ldms\n",
		rules.nb_coders, rules.time_to_burnout, rules.time_to_compile);
	printf("Debug: %ldms | Refactor: %ldms | Compiles req: %d\n",
		rules.time_to_debug, rules.time_to_refactor, rules.nb_compiles_req);
	printf("Cooldown: %ldms | Scheduler: ", rules.dongle_cooldown);
	if (rules.scheduler == E_SCHED_FIFO)
		printf("FIFO\n");
	else
		printf("EDF\n");
	pthread_mutex_destroy(&rules.log_mutex);
	pthread_mutex_destroy(&rules.end_mutex);
	return (0);
}
