#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_SIZE 1000

int N; // 矩阵大小
double A[MAX_SIZE][MAX_SIZE]; // 矩阵
double b[MAX_SIZE]; // 右边的向量
double x[MAX_SIZE]; // 解向量

int num_threads; // 线程数
pthread_barrier_t barrier; // barrier同步对象

// 执行高斯消元的线程函数
void* gauss_elimination(void* arg) {
    long id = (long)arg;
    int i, j, k;
    double factor;

    // 每个线程负责的行范围
    int start = id * (N / num_threads);
    int end = (id == num_threads - 1) ? N : (id + 1) * (N / num_threads);

    // 高斯消元
    for (k = 0; k < N - 1; k++) {
        // 避免多个线程同时读写同一个变量
        pthread_barrier_wait(&barrier);

        if (k >= start && k < end) {
            for (i = k + 1; i < N; i++) {
                factor = A[i][k] / A[k][k];
                for (j = k + 1; j < N; j++) {
                    A[i][j] -= factor * A[k][j];
                }
                b[i] -= factor * b[k];
            }
        }

        // 等待所有线程执行完毕
        pthread_barrier_wait(&barrier);
    }

    // 回带求解
    if (end == N) {
        x[N - 1] = b[N - 1] / A[N - 1][N - 1];
    }

    for (k = N - 2; k >= 0; k--) {
        if (k >= start && k < end) {
            x[k] = b[k];
            for (j = k + 1; j < N; j++) {
                x[k] -= A[k][j] * x[j];
            }
            x[k] /= A[k][k];
        }
        // 等待所有线程执行完毕
        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <matrix size> <num threads>\n", argv[0]);
        return 0;
    }

    N = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    if (N > MAX_SIZE || num_threads > N) {
        printf("Invalid input!\n");
        return 0;
    }

    // 初始化矩阵和向量
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            A[i][j] = (double)rand() / RAND_MAX * 10.0;
        }
        b[i] = (double)rand() / RAND_MAX * 10.0;
    }

    // 初始化barrier
    pthread_barrier_init(&barrier, NULL, num_threads);

    // 创建线程
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, gauss_elimination, (void*)i);
    }

    // 计算时间
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // 等待所有线程执行完毕
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);
    double time_used = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time used: %.6f seconds.\n", time_used);

    // 释放资源
    free(threads);
    pthread_barrier_destroy(&barrier);

    return 0;
}
