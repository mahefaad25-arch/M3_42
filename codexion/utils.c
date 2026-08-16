#include "coders.h"

long get_time_in_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void log_status(t_coder *coder, const char *status)
{
    long current_time;

    pthread_mutex_lock(&coder->rules->end_mutex);
    if (coder->rules->simulation_ended)
    {
        pthread_mutex_unlock(&coder->rules->end_mutex);
        return;
    }
    pthread_mutex_unlock(&coder->rules->end_mutex);

    pthread_mutex_lock(&coder->rules->log_mutex);
    current_time = get_time_in_ms() - coder->rules->start_time;
    printf("%ld %d %s\n", current_time, coder->id, status);
    pthread_mutex_unlock(&coder->rules->log_mutex);
}