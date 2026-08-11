/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/07 10:02:10 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/12 00:04:39 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum e_scheduler
{
    SCHED_FIFO,
    SCHED_EDF
}   t_scheduler;

typedef struct s_rules
{
    int         nb_coders;
    long        time_to_burnout;
    long        time_to_compile;
    long        time_to_debug;
    long        time_to_refactor;
    int         nb_compiles_req;
    long        dongle_cooldown;
    t_scheduler scheduler;
}   t_rules;

static long ft_atol(const char *str)
{
    long    res;
    int     i;

    res = 0;
    i = 0;
    if (str[i] == '+')
        i++;
    if (!str[i])
        return (-1);
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (-1);
        res = res * 10 + (str[i] - '0');
        if (res > 2147483647)
            return (-1);
        i++;
    }
    return (res);
}

int init_rules(int argc, char **argv, t_rules *rules)
{
    if (argc != 9)
        return (1);

    rules->nb_coders = (int)ft_atol(argv[1]);
    rules->time_to_burnout = ft_atol(argv[2]);
    rules->time_to_compile = ft_atol(argv[3]);
    rules->time_to_debug = ft_atol(argv[4]);
    rules->time_to_refactor = ft_atol(argv[5]);
    rules->nb_compiles_req = (int)ft_atol(argv[6]);
    rules->dongle_cooldown = ft_atol(argv[7]);
    if (rules->nb_coders <= 0 || rules->time_to_burnout <= 0 ||
        rules->time_to_compile <= 0 || rules->time_to_debug <= 0 ||
        rules->time_to_refactor <= 0 || rules->nb_compiles_req < 0 ||
        rules->dongle_cooldown < 0)
        return (1);
    if (strcmp(argv[8], "fifo") == 0)
        rules->scheduler = SCHED_FIFO;
    else if (strcmp(argv[8], "edf") == 0)
        rules->scheduler = SCHED_EDF;
    else
        return (1);

    return (0);
}

int main(int argc, char *argv[])
{
    t_rules rules;

    if (init_rules(argc, argv, &rules) != 0)
    {
        printf("Error: Invalid arguments.\n");
        return (1);
    }

    printf("--- Configuration chargée avec succès ---\n");
    printf("Coders: %d | Burnout: %ldms | Compile: %ldms\n",
           rules.nb_coders, rules.time_to_burnout, rules.time_to_compile);
    printf("Debug: %ldms | Refactor: %ldms | Compiles req: %d\n",
           rules.time_to_debug, rules.time_to_refactor, rules.nb_compiles_req);
    printf("Cooldown: %ldms | Scheduler: %s\n",
           rules.dongle_cooldown, rules.scheduler == SCHED_FIFO ? "FIFO" : "EDF");

    return (0);
}