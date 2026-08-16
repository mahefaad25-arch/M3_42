#ifndef CODERS_H
# define CODERS_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <sys/time.h>
# include <stdbool.h>
# include <unistd.h>

typedef enum e_scheduler
{
    E_SCHED_FIFO,
    E_SCHED_EDF
}   t_scheduler;

typedef struct s_rules
{
    int             nb_coders;
    long            time_to_burnout;
    long            time_to_compile;
    long            time_to_debug;
    long            time_to_refactor;
    int             nb_compiles_req;
    long            dongle_cooldown;
    t_scheduler     scheduler;
    long            start_time;
    bool            simulation_ended;
    pthread_mutex_t log_mutex;
    pthread_mutex_t end_mutex;
}   t_rules;

typedef struct s_coder t_coder;

typedef struct s_request
{
    t_coder *coder;
    long    request_time;
    long    deadline;
}   t_request;

typedef struct s_heap
{
    t_request   *elements;
    int         capacity;
    int         size;
}   t_heap;

typedef struct s_dongle
{
    int             id;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            is_busy;
    long            last_released_time;
    t_heap          queue;
}   t_dongle;

struct s_coder
{
    int         id;
    int         compiles_count;
    long        last_compile_start;
    t_dongle    *left_dongle;
    t_dongle    *right_dongle;
    t_rules     *rules;
    pthread_t   thread;
};

/* --- Prototypes Parsing (parsing.c) --- */
long    ft_atol(const char *str);
int     init_rules(int argc, char **argv, t_rules *rules);

/* --- Prototypes Heap (heap.c) --- */
t_heap  create_heap(int capacity);
void    free_heap(t_heap *heap);
void    heap_push(t_heap *heap, t_request req, t_scheduler scheduler);
t_request heap_pop(t_heap *heap, t_scheduler scheduler);

/* --- Prototypes Dongles (dongle.c) --- */
int     init_dongles(t_dongle **dongles, int nb_coders);
void    assign_dongles_to_coders(t_coder *coders, t_dongle *dongles, int nb_coders);
void    take_dongle(t_coder *coder, t_dongle *dongle);
void    release_dongle(t_coder *coder, t_dongle *dongle);
void    free_dongles(t_dongle *dongles, int nb_coders);

/* --- Prototypes Utils (utils.c) --- */
long    get_time_in_ms(void);
void    log_status(t_coder *coder, const char *status);
void    smart_usleep(long ms, t_rules *rules);

#endif