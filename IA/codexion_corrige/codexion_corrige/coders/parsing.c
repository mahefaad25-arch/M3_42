/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   parsing.c                                         :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/08/31 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/08/31 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Checks that "str" only contains digits (no sign, no letters, no spaces).
** We reject empty strings too.
** This is how we make sure "non-integer" arguments are refused, as asked
** by the subject.
*/
static int	is_all_digits(const char *str)
{
	int	i;

	if (str == NULL || str[0] == '\0')
		return (0);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

/*
** Converts a validated numeric string into a long.
** We already know it only has digits, atoi()/manual conversion is safe here.
*/
static long	str_to_long(const char *str)
{
	long	res;
	int		i;

	res = 0;
	i = 0;
	while (str[i])
	{
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return (res);
}

/*
** Parses one mandatory positive integer argument.
** Returns 0 on success, -1 on error (prints an explicit message).
*/
static int	parse_positive(const char *str, long *out, const char *name)
{
	if (!is_all_digits(str))
	{
		fprintf(stderr, "codexion: invalid argument for %s: '%s'\n",
			name, str);
		return (-1);
	}
	*out = str_to_long(str);
	if (*out == 0)
	{
		fprintf(stderr, "codexion: %s must be strictly positive\n", name);
		return (-1);
	}
	return (0);
}

/*
** Parses the scheduler argument: must be exactly "fifo" or "edf".
*/
static int	parse_scheduler(const char *str, t_sched *out)
{
	if (strcmp(str, "fifo") == 0)
	{
		*out = CX_SCHED_FIFO;
		return (0);
	}
	if (strcmp(str, "edf") == 0)
	{
		*out = CX_SCHED_EDF;
		return (0);
	}
	fprintf(stderr, "codexion: scheduler must be 'fifo' or 'edf'\n");
	return (-1);
}

/*
** Entry point of the parsing module.
** Fills the t_params structure from argv.
** Returns 0 on success, -1 if any argument is invalid.
**
** Expected order:
** number_of_coders time_to_burnout time_to_compile time_to_debug
** time_to_refactor number_of_compiles_required dongle_cooldown scheduler
*/
int	parse_args(int argc, char **argv, t_params *p)
{
	long	tmp;

	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n",
			argv[0]);
		return (-1);
	}
	if (parse_positive(argv[1], &tmp, "number_of_coders") == -1)
		return (-1);
	p->nb_coders = (int)tmp;
	if (parse_positive(argv[2], &p->time_to_burnout, "time_to_burnout") == -1)
		return (-1);
	if (parse_positive(argv[3], &p->time_to_compile, "time_to_compile") == -1)
		return (-1);
	if (parse_positive(argv[4], &p->time_to_debug, "time_to_debug") == -1)
		return (-1);
	if (parse_positive(argv[5], &p->time_to_refactor,
			"time_to_refactor") == -1)
		return (-1);
	if (parse_positive(argv[6], &tmp, "number_of_compiles_required") == -1)
		return (-1);
	p->nb_compiles_required = (int)tmp;
	if (parse_positive(argv[7], &p->dongle_cooldown, "dongle_cooldown") == -1)
		return (-1);
	if (parse_scheduler(argv[8], &p->scheduler) == -1)
		return (-1);
	return (0);
}
