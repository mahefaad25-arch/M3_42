/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/07 10:02:10 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/07 10:22:12 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include<stdio.h>
#include <stdlib.h>

int	main(int ac, char *av[])
{
	int	i;
	i = 1;
	if (ac == 9)
	{
		while (i < ac)
		{
			printf("%d", atoi(av[i]));
			i++;
		}
	}
	else
	{
		printf("programe should number_of_coders time_to_burnout time_to_compile"
			"time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler");
	}
}