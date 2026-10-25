/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing2.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 18:55:41 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 19:11:15 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	parse_second_group(char **argv, t_params *p)
{
	int	value;

	if (parse_positive(argv[6], &p->nb_compiles_required,
			"number_of_compiles_required", 1) == -1)
		return (-1);
	if (parse_positive(argv[7], &value, "dongle_cooldown", 1) == -1)
		return (-1);
	p->dongle_cooldown = value;
	return (parse_scheduler(argv[8], p));
}

int	parse_args(int argc, char **argv, t_params *p)
{
	if (argc != 9)
	{
		fprintf(stderr,
			"Usage: %s number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n",
			argv[0]);
		return (-1);
	}
	if (parse_first_group(argv, p) == -1)
		return (-1);
	return (parse_second_group(argv, p));
}

void	set_fork_order(t_coder *c, int *first, int *second)
{
	if (c->id % 2 == 0)
	{
		usleep(1000);
		*first = c->left_dongle;
		*second = c->right_dongle;
	}
	else
	{
		*first = c->right_dongle;
		*second = c->left_dongle;
	}
}
