/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 08:30:06 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/26 08:30:07 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

long	ft_atol(const char *str)
{
	long	res;
	int		i;

	if (!str)
		return (-1);
	res = 0;
	i = 0;
	if (str[i] == '+')
		i++;
	if (!str[i])
		return (-1);
	while (str[i] != '\0')
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

int	init_rules(int argc, char **argv, t_rules *rules)
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
	if (rules->nb_coders <= 0 || rules->time_to_burnout <= 0
		|| rules->time_to_compile <= 0 || rules->time_to_debug <= 0
		|| rules->time_to_refactor <= 0 || rules->nb_compiles_req < 0
		|| rules->dongle_cooldown < 0)
		return (1);
	if (strcmp(argv[8], "fifo") == 0)
		rules->scheduler = E_SCHED_FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		rules->scheduler = E_SCHED_EDF;
	else
		return (1);
	rules->simulation_ended = false;
	if (pthread_mutex_init(&rules->log_mutex, NULL) != 0
		|| pthread_mutex_init(&rules->end_mutex, NULL) != 0)
		return (1);
	return (0);
}
