/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 01:54:00 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 01:54:17 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	print_invalid(const char *name)
{
	fprintf(stderr, "codexion: invalid or too large argument for %s\n", name);
	return (-1);
}

static int	parse_digit(unsigned int *value, unsigned int max_int, char c,
		const char *name)
{
	unsigned int	digit;

	if (c < '0' || c > '9')
		return (print_invalid(name));
	digit = c - '0';
	if (*value > (max_int - digit) / 10)
		return (print_invalid(name));
	*value = *value * 10 + digit;
	return (0);
}

static int	parse_number(const char *str, unsigned int *out,
		const char *name)
{
	unsigned int	max_int;
	int				i;

	if (str == NULL || str[0] == '\0')
		return (print_invalid(name));
	*out = 0;
	max_int = (unsigned int)-1 / 2;
	i = 0;
	while (str[i])
	{
		if (parse_digit(out, max_int, str[i], name) == -1)
			return (-1);
		i++;
	}
	return (0);
}

static int	parse_positive(const char *str, int *out, const char *name,
		int allow_zero)
{
	unsigned int	value;

	if (parse_number(str, &value, name) == -1)
		return (-1);
	if (!allow_zero && value == 0)
		return (print_invalid(name));
	*out = ft_atoi(str);
	return (0);
}

static int	parse_times(char **argv, t_params *p)
{
	int	value;

	if (parse_positive(argv[3], &value, "time_to_compile", 1) == -1)
		return (-1);
	p->time_to_compile = value;
	if (parse_positive(argv[4], &value, "time_to_debug", 1) == -1)
		return (-1);
	p->time_to_debug = value;
	if (parse_positive(argv[5], &value, "time_to_refactor", 1) == -1)
		return (-1);
	p->time_to_refactor = value;
	return (0);
}

static int	parse_first_group(char **argv, t_params *p)
{
	int	value;

	if (parse_positive(argv[1], &p->nb_coders, "number_of_coders", 0) == -1)
		return (-1);
	if (parse_positive(argv[2], &value, "time_to_burnout", 0) == -1)
		return (-1);
	p->time_to_burnout = value;
	return (parse_times(argv, p));
}

static int	parse_scheduler(const char *str, t_params *p)
{
	if (strcmp(str, "fifo") == 0)
		p->scheduler = CX_SCHED_FIFO;
	else if (strcmp(str, "edf") == 0)
		p->scheduler = CX_SCHED_EDF;
	else
	{
		fprintf(stderr, "codexion: scheduler must be 'fifo' or 'edf'\n");
		return (-1);
	}
	return (0);
}

static int	parse_second_group(char **argv, t_params *p)
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
		fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n", argv[0]);
		return (-1);
	}
	if (parse_first_group(argv, p) == -1)
		return (-1);
	return (parse_second_group(argv, p));
}
