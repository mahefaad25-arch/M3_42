/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/07 10:02:10 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/16 22:48:06 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int main(int argc, char *argv[])
{
    t_rules rules;

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
    printf("Cooldown: %ldms | Scheduler: %s\n",
           rules.dongle_cooldown, rules.scheduler == E_SCHED_FIFO ? "FIFO" : "EDF");

    pthread_mutex_destroy(&rules.log_mutex);
    pthread_mutex_destroy(&rules.end_mutex);
    return (0);
}