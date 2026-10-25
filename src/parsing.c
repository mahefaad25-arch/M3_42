/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef <bramahef@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 18:36:25 by bramahef          #+#    #+#             */
/*   Updated: 2026/10/24 19:06:48 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	print_invalid(const char *name)
{
	fprintf(stderr, "codexion: invalid or too large argument for %s\n", name);
	return (-1);
}

int	parse_positive(const char *str, int *out, const char *name,
		int allow_zero)
{
	unsigned int	value;
	unsigned int	max_int;
	int				i;

	if (str == NULL || str[0] == '\0')
		return (print_invalid(name));
	value = 0;
	max_int = (unsigned int)-1 / 2;
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (print_invalid(name));
		if (value > (max_int - (unsigned int)(str[i] - '0')) / 10)
			return (print_invalid(name));
		value = value * 10 + (str[i] - '0');
		i++;
	}
	if (!allow_zero && value == 0)
		return (print_invalid(name));
	*out = ft_atoi(str);
	return (0);
}

int	parse_scheduler(const char *str, t_params *p)
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

int	parse_first_group(char **argv, t_params *p)
{
	int	value;

	if (parse_positive(argv[1], &p->nb_coders, "number_of_coders", 0) == -1)
		return (-1);
	if (parse_positive(argv[2], &value, "time_to_burnout", 0) == -1)
		return (-1);
	p->time_to_burnout = value;
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
