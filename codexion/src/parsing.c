/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:27:32 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/23 07:33:10 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** True only if the string is a non-empty sequence of digits. This
** is what rejects negative numbers ('-' is not a digit), decimals,
** letters and empty arguments, as the subject requires.
*/
static int	is_all_digits(const char *s)
{
	int	i;

	if (s == NULL || s[0] == '\0')
		return (0);
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

/*
** Converts a string already validated as digits-only into a long,
** refusing values that would overflow an int.
*/
static int	str_to_long(const char *s, long *out)
{
	long	res;
	int		i;

	res = 0;
	i = 0;
	while (s[i])
	{
		res = res * 10 + (s[i] - '0');
		if (res > 2147483647)
			return (-1);
		i++;
	}
	*out = res;
	return (0);
}

/*
** Parses one strictly positive integer argument.
** Returns 0 on success, -1 with an explicit message otherwise.
*/
static int	parse_value(const char *s, long *out, const char *name)
{
	if (!is_all_digits(s) || str_to_long(s, out) == -1)
	{
		fprintf(stderr, "codexion: invalid value for %s: '%s'\n", name, s);
		return (-1);
	}
	if (*out == 0)
	{
		fprintf(stderr, "codexion: %s must be strictly positive\n", name);
		return (-1);
	}
	return (0);
}

/*
** The scheduler argument must be exactly "fifo" or "edf".
*/
static int	parse_scheduler(const char *s, t_sched *out)
{
	if (strcmp(s, "fifo") == 0)
		*out = CX_FIFO;
	else if (strcmp(s, "edf") == 0)
		*out = CX_EDF;
	else
	{
		fprintf(stderr, "codexion: scheduler must be 'fifo' or 'edf'\n");
		return (-1);
	}
	return (0);
}

/*
** Fills the parameters from argv. All eight arguments are
** mandatory. Returns 0 on success, -1 if anything is invalid.
*/
int	parse_args(int argc, char **argv, t_params *p)
{
	long	tmp;

	if (argc != 9)
		return (fprintf(stderr,
				"Usage: %s number_of_coders time_to_burnout "
				"time_to_compile time_to_debug time_to_refactor "
				"number_of_compiles_required dongle_cooldown "
				"scheduler\n",
				argv[0]), -1);
	if (parse_value(argv[1], &tmp, "number_of_coders") == -1)
		return (-1);
	p->nb_coders = (int)tmp;
	if (parse_value(argv[2], &p->time_to_burnout, "time_to_burnout") == -1
		|| parse_value(argv[3], &p->time_to_compile, "time_to_compile") == -1
		|| parse_value(argv[4], &p->time_to_debug, "time_to_debug") == -1
		|| parse_value(argv[5], &p->time_to_refactor, "time_to_refactor") == -1)
		return (-1);
	if (parse_value(argv[6], &tmp, "number_of_compiles_required") == -1)
		return (-1);
	p->nb_compiles_required = (int)tmp;
	if (parse_value(argv[7], &p->dongle_cooldown, "dongle_cooldown") == -1)
		return (-1);
	return (parse_scheduler(argv[8], &p->scheduler));
}
