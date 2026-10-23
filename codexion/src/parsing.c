/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 00:00:00 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/23 00:00:00 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <limits.h>

static long	str_to_long(const char *str)
{
	long	res;
	int		i;

	if (str == NULL || str[0] == '\0')
		return (-1);
	res = 0;
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (-1);
		if (res > (LONG_MAX - (str[i] - '0')) / 10)
			return (-1);
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return (res);
}

static int	parse_positive(const char *str, long *out, const char *name)
{
	*out = str_to_long(str);
	if (*out < 0)
	{
		fprintf(stderr, "codexion: invalid or too large argument for %s\n",
			name);
		return (-1);
	}
	if ((strcmp(name, "number_of_coders") == 0
			|| strcmp(name, "time_to_burnout") == 0) && *out == 0)
	{
		fprintf(stderr, "codexion: %s must be greater than 0\n", name);
		return (-1);
	}
	return (0);
}

static int	parse_first_group(char **argv, t_params *p)
{
	long	value;

	if (parse_positive(argv[1], &value, "number_of_coders") == -1
		|| value > INT_MAX)
		return (-1);
	p->nb_coders = (int)value;
	if (parse_positive(argv[2], &p->time_to_burnout,
			"time_to_burnout") == -1)
		return (-1);
	if (parse_positive(argv[3], &p->time_to_compile,
			"time_to_compile") == -1)
		return (-1);
	return (parse_positive(argv[4], &p->time_to_debug, "time_to_debug")
		== 0 && parse_positive(argv[5], &p->time_to_refactor,
			"time_to_refactor") == 0);
}

static int	parse_second_group(char **argv, t_params *p)
{
	long	value;

	if (parse_positive(argv[6], &value,
			"number_of_compiles_required") == -1 || value > INT_MAX)
	{
		fprintf(stderr, "codexion: number_of_compiles_required is too large\n");
		return (-1);
	}
	p->nb_compiles_required = (int)value;
	if (parse_positive(argv[7], &p->dongle_cooldown, "dongle_cooldown") == -1)
		return (-1);
	if (strcmp(argv[8], "fifo") == 0)
		p->scheduler = CX_SCHED_FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		p->scheduler = CX_SCHED_EDF;
	// else if (strcmp(argv[8], "lifo") == 0)
	// 	p->scheduler = CX_SCHED_LIFO;
	else
	{
		fprintf(stderr,
			"codexion: scheduler must be 'fifo', 'edf' or 'lifo'\n");
		return (-1);
	}
	return (0);
}

int	parse_args(int argc, char **argv, t_params *p)
{
	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n", argv[0]);
		return (-1);
	}
	if (parse_first_group(argv, p) != 1)
		return (-1);
	return (parse_second_group(argv, p));
}
