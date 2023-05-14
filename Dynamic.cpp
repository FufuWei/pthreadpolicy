#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_SIZE 1000

int n;
double a[MAX_SIZE][MAX_SIZE + 1];
pthread_t threads[MAX_SIZE];
int thread_args[MAX_SIZE];

void* worker(void* arg) {
    int id = *(int*)arg;
    int i, j, k;
    double div, temp;

    for (i = id; i < n; i += n) {
        div = a[i][i];
        for (j = i + 1; j <= n; j++) {
            a[i][j] /= div;
        }
        a[i][i] = 1.0;
        for (j = i + 1; j < n; j++) {
            div = a[j][i];
            for (k = i + 1; k <= n; k++) {
                a[j][k] -= div * a[i][k];
            }
            a[j][i] = 0.0;
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int i, j;
    double elapsed_time;
    struct timeval start_time, end_time;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        exit(1);
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        exit(1);
    }

    fscanf(fp, "%d", &n);
    for (i = 0; i < n; i++) {
        for (j = 0; j <= n; j++) {
            fscanf(fp, "%lf", &a[i][j]);
        }
    }

    gettimeofday(&start_time, NULL);

    for (i = 0; i < n; i++) {
        thread_args[i] = i;
        pthread_create(&threads[i], NULL, worker, &thread_args[i]);
    }

    for (i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    for (i = n - 1; i >= 0; i--) {
        for (j = i - 1; j >= 0; j--) {
            a[j][n] -= a[i][n] * a[j][i];
            a[j][i] = 0.0;
        }
    }

    gettimeofday(&end_time, NULL);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
        (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    printf("Elapsed time: %.3f seconds\n", elapsed_time);

    for (i = 0; i < n; i++) {
        printf("x[%d] = %lf\n", i, a[i][n]);
    }

    fclose(fp);
    return 0;
}