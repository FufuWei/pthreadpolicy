#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define N 1000

int A[N][N + 1];
sem_t sem[N];

void* thread_func(void* arg) {
    int i = *(int*)arg;

    // Gauss elimination
    for (int k = 0; k < N; k++) {
        if (k == i) continue;
        double coeff = (double)A[k][i] / A[i][i];
        for (int j = i; j < N + 1; j++) {
            A[k][j] -= coeff * A[i][j];
        }
    }

    sem_post(&sem[i]);

    return NULL;
}

int main() {
    // Generate random matrix
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N + 1; j++) {
            A[i][j] = rand();
        }
    }

    // Initialize semaphores
    for (int i = 0; i < N; i++) {
        sem_init(&sem[i], 0, 0);
    }

    // Measure time
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Create threads
    pthread_t threads[N];
    int thread_args[N];
    for (int i = 0; i < N; i++) {
        thread_args[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < N; i++) {
        sem_wait(&sem[i]);
    }

    // Measure time
    gettimeofday(&end, NULL);
    double time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Time: %lf ms\n", time);

    // Destroy semaphores
    for (int i = 0; i < N; i++) {
        sem_destroy(&sem[i]);
    }

    return 0;
}
