/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 22:43:52 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/16 22:43:56 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Initialise la mémoire et les mutex/cond de tous les dongles
int init_dongles(t_dongle **dongles, int nb_coders)
{
    int i;

    *dongles = malloc(sizeof(t_dongle) * nb_coders);
    if (!*dongles)
        return (1);

    i = 0;
    while (i < nb_coders)
    {
        (*dongles)[i].id = i + 1;
        (*dongles)[i].is_busy = false;
        (*dongles)[i].last_released_time = 0;
        if (pthread_mutex_init(&(*dongles)[i].mutex, NULL) != 0 ||
            pthread_cond_init(&(*dongles)[i].cond, NULL) != 0)
            return (1);
        (*dongles)[i].queue = create_heap(nb_coders + 1);
        i++;
    }
    return (0);
}

// Assigne les pointeurs gauche/droite à chaque coder
void assign_dongles_to_coders(t_coder *coders, t_dongle *dongles, int nb_coders)
{
    int i;

    i = 0;
    while (i < nb_coders)
    {
        coders[i].id = i + 1;
        if (nb_coders == 1)
        {
            // Un seul coder = un seul dongle sur la table
            coders[i].left_dongle = &dongles[0];
            coders[i].right_dongle = NULL;
        }
        else
        {
            coders[i].left_dongle = &dongles[i];
            coders[i].right_dongle = &dongles[(i + 1) % nb_coders];
        }
        i++;
    }
}

// Vérifie si le cooldown du dongle est expiré
static bool is_cooldown_active(t_dongle *dongle, t_rules *rules)
{
    long current_time;

    if (rules->dongle_cooldown == 0)
        return (false);
    current_time = get_time_in_ms();
    return ((current_time - dongle->last_released_time) < rules->dongle_cooldown);
}

// Prise de dongle : gère la priorité (FIFO/EDF) et le cooldown
void take_dongle(t_coder *coder, t_dongle *dongle)
{
    t_request req;

    if (!dongle)
        return ;

    pthread_mutex_lock(&dongle->mutex);

    // Préparer la requête pour la file de priorité
    req.coder = coder;
    req.request_time = get_time_in_ms();
    req.deadline = coder->last_compile_start + coder->rules->time_to_burnout; //

    // Ajouter le coder dans la file du dongle
    heap_push(&dongle->queue, req, coder->rules->scheduler);

    // Attendre que le dongle soit libre, hors cooldown, ET que ce soit le tour du coder
    while (1)
    {
        pthread_mutex_lock(&coder->rules->end_mutex);
        if (coder->rules->simulation_ended)
        {
            pthread_mutex_unlock(&coder->rules->end_mutex);
            pthread_mutex_unlock(&dongle->mutex);
            return ;
        }
        pthread_mutex_unlock(&coder->rules->end_mutex);

        // Vérifier si ce coder est en tête de file (le plus prioritaire)
        if (!dongle->is_busy && 
            dongle->queue.size > 0 && 
            dongle->queue.elements[0].coder->id == coder->id &&
            !is_cooldown_active(dongle, coder->rules))
        {
            // Retirer le coder de la file
            heap_pop(&dongle->queue, coder->rules->scheduler);
            dongle->is_busy = true;
            pthread_mutex_unlock(&dongle->mutex);
            
            // Log de prise de dongle
            log_status(coder, "has taken a dongle"); //
            return ;
        }

        // Si le cooldown est la seule raison du blocage, on attend jusqu'à sa fin
        if (!dongle->is_busy && 
            dongle->queue.size > 0 && 
            dongle->queue.elements[0].coder->id == coder->id &&
            is_cooldown_active(dongle, coder->rules))
        {
            struct timespec ts;
            long wait_time_ms = coder->rules->dongle_cooldown - 
                                (get_time_in_ms() - dongle->last_released_time);
            
            if (wait_time_ms > 0)
            {
                ts.tv_sec = time(NULL) + (wait_time_ms / 1000);
                ts.tv_nsec = (wait_time_ms % 1000) * 1000000;
                pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
                continue;
            }
        }

        // Sinon, attente classique via variable de condition
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
    }
}

// Libère le dongle, enregistre l'heure de relâchement pour le cooldown et réveille les autres
void release_dongle(t_coder *coder, t_dongle *dongle)
{
    (void)coder;
    if (!dongle)
        return ;

    pthread_mutex_lock(&dongle->mutex);
    dongle->is_busy = false;
    dongle->last_released_time = get_time_in_ms(); // Marquer l'heure pour le cooldown[cite: 2]
    
    // Réveiller tous les threads en attente sur ce dongle
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->mutex);
}

// Libération de la mémoire et destruction des mutex/cond
void free_dongles(t_dongle *dongles, int nb_coders)
{
    int i;

    if (!dongles)
        return ;

    i = 0;
    while (i < nb_coders)
    {
        pthread_mutex_destroy(&dongles[i].mutex);
        pthread_cond_destroy(&dongles[i].cond);
        free_heap(&dongles[i].queue);
        i++;
    }
    free(dongles);
}