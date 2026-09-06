#include <stdio.h>
#include <pthread.h>

unsigned int counter = 0;
pthread_mutex_t lock;

void *counter_routine(void *arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&lock);   // Début de section critique
        counter++;
        pthread_mutex_unlock(&lock); // Fin de section critique
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    pthread_mutex_init(&lock, NULL);

    pthread_create(&t1, NULL, counter_routine, NULL);
    pthread_create(&t2, NULL, counter_routine, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&lock);

    printf("Valeur finale du compteur : %u\n", counter); // Toujours 200000
    return 0;
}