/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/07 10:02:10 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/07 12:59:58 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef enum {
    SCHED_FIFO,
    SCHED_EDF
} t_scheduler;

typedef struct s_config {
    int         number_of_coders;
    long        time_to_burnout;
    long        time_to_compile;
    long        time_to_debug;
    long        time_to_refactor;
    int         number_of_compiles_required;
    long        dongle_cooldown;
    t_scheduler scheduler;
} t_config;

// Convertit une chaîne en entier positif avec gestion d'erreurs
static long parse_positive_long(const char *str, const char *arg_name) {
    char *endptr;
    long val = strtol(str, &endptr, 10);

    if (*endptr != '\0' || val <= 0 || val > INT_MAX) {
        fprintf(stderr, "Erreur: '%s' doit être un entier positif valide (%s).\n", arg_name, str);
        return -1;
    }
    return val;
}

int parse_args(int argc, char **argv, t_config *config) {
    if (argc != 9) {
        fprintf(stderr, "Usage: %s number_of_coders time_to_burnout time_to_compile "
                        "time_to_debug time_to_refactor number_of_compiles_required "
                        "dongle_cooldown scheduler\n", argv[0]);
        return 0;
    }

    config->number_of_coders = (int)parse_positive_long(argv[1], "number_of_coders");
    config->time_to_burnout = parse_positive_long(argv[2], "time_to_burnout");
    config->time_to_compile = parse_positive_long(argv[3], "time_to_compile");
    config->time_to_debug = parse_positive_long(argv[4], "time_to_debug");
    config->time_to_refactor = parse_positive_long(argv[5], "time_to_refactor");
    config->number_of_compiles_required = (int)parse_positive_long(argv[6], "number_of_compiles_required");
    config->dongle_cooldown = parse_positive_long(argv[7], "dongle_cooldown");

    // Vérification de la validité de tous les entiers
    if (config->number_of_coders < 0 || config->time_to_burnout < 0 ||
        config->time_to_compile < 0 || config->time_to_debug < 0 ||
        config->time_to_refactor < 0 || config->number_of_compiles_required < 0 ||
        config->dongle_cooldown < 0) {
        return 0;
    }

    // Parsing du scheduler (fifo ou edf)
    if (strcmp(argv[8], "fifo") == 0) {
        config->scheduler = SCHED_FIFO;
    } else if (strcmp(argv[8], "edf") == 0) {
        config->scheduler = SCHED_EDF;
    } else {
        fprintf(stderr, "Erreur: scheduler doit être 'fifo' ou 'edf' (reçu: '%s').\n", argv[8]);
        return 0;
    }

    return 1;
}

int main(int argc, char **argv) {
    t_config config;

    if (!parse_args(argc, argv, &config)) {
        return EXIT_FAILURE;
    }

    // Exemple de confirmation du parsing
    printf("Parsing réussi :\n");
    printf(" - Coders: %d\n", config.number_of_coders);
    printf(" - Burnout: %ld ms\n", config.time_to_burnout);
    printf(" - Scheduler: %s\n", config.scheduler == SCHED_FIFO ? "fifo" : "edf");

    return EXIT_SUCCESS;
}