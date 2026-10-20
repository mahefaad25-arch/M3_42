#include "codexion.h"

#include <limits.h>

static int is_all_digits(const char *str)
{
	int i;

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

static long str_to_long(const char *str)
{
	long res;
	int i;

	res = 0;
	i = 0;
	while (str[i])
	{
		if (res > (LONG_MAX - (str[i] - '0')) / 10)
			return (-1);
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return (res);
}

static int	parse_positive(const char *str, long *out, const char *name)
{
	if (!is_all_digits(str))
	{
		fprintf(stderr, "codexion: invalid argument for %s: '%s'\n",
			name, str);
		return (-1);
	}
	*out = str_to_long(str);
	if (*out < 0)
	{
		fprintf(stderr, "codexion: %s is too large\n", name);
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

static int	parse_int_argument(const char *str, int *out, const char *name)
{
	long	value;

	if (parse_positive(str, &value, name) == -1)
		return (-1);
	if (value > INT_MAX)
	{
		fprintf(stderr, "codexion: %s is too large\n", name);
		return (-1);
	}
	*out = (int)value;
	return (0);
}

static int parse_scheduler(const char *str, t_sched *out)
{
	if (strcmp(str, "fifo") == 0)
		*out = CX_SCHED_FIFO;
	else if (strcmp(str, "edf") == 0)
		*out = CX_SCHED_EDF;
	else
	{
		fprintf(stderr, "codexion: scheduler must be 'fifo' or 'edf'\n");
		return (-1);
	}
	return (0);
}

int parse_args(int argc, char **argv, t_params *p)
{
	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
						"time_to_compile time_to_debug time_to_refactor "
						"number_of_compiles_required dongle_cooldown scheduler\n",
				argv[0]);
		return (-1);
	}
	if (parse_int_argument(argv[1], &p->nb_coders, "number_of_coders") == -1)
		return (-1);
	if (parse_positive(argv[2], &p->time_to_burnout, "time_to_burnout") == -1)
		return (-1);
	if (parse_positive(argv[3], &p->time_to_compile, "time_to_compile") == -1)
		return (-1);
	if (parse_positive(argv[4], &p->time_to_debug, "time_to_debug") == -1)
		return (-1);
	if (parse_positive(argv[5], &p->time_to_refactor,
					   "time_to_refactor") == -1)
		return (-1);
	if (parse_int_argument(argv[6], &p->nb_compiles_required,
			"number_of_compiles_required") == -1)
		return (-1);
	if (parse_positive(argv[7], &p->dongle_cooldown, "dongle_cooldown") == -1)
		return (-1);
	if (parse_scheduler(argv[8], &p->scheduler) == -1)
		return (-1);
	return (0);
}
