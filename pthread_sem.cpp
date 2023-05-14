#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define MAX_N 1000
#define MAX_THREAD 8

int N; // 矩阵大小
double A[MAX_N][MAX_N]; // 矩阵
double b[MAX_N]; // 右侧矩阵
double x[MAX_N]; // 解向量

pthread_t tid[MAX_THREAD]; // 线程id数组
sem_t sem[MAX_N]; // 信号量数组

void* worker(void* arg) {
    int i = *(int*)arg;
    int j, k;
    double pivot;

    for (k = 0; k < N; k++) {
        if (k % MAX_THREAD != i) continue; // 确定该线程负责的列
        for (j = k + 1; j < N; j++) {
            pivot = A[j][k] / A[k][k]; // 计算消元因子
            A[j][k] = 0; // 清除该位置的元素
            for (int l = k + 1; l < N; l++) {
                A[j][l] -= pivot * A[k][l]; // 消元
            }
            b[j] -= pivot * b[k]; // 消元
        }
        sem_post(&sem[k]); // 发送信号
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <matrix size>\n", argv[0]);
        return 0;
    }

    N = atoi(argv[1]);
    if (N > MAX_N) {
        printf("Matrix size should be no more than %d\n", MAX_N);
        return 0;
    }

    // 初始化矩阵
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) {
                A[i][j] = 1.0;
            }
            else {
                A[i][j] = 0.0;
            }
        }
        b[i] = 1.0 * i;
    }

    // 创建信号量
    for (int i = 0; i < N; i++) {
        sem_init(&sem[i], 0, 0); // 初始值为0
    }

    // 创建线程
    struct timeval start, end;
    gettimeofday(&start, NULL); // 开始计时
    for (int i = 0; i < MAX_THREAD; i++) {
        int* arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&tid[i], NULL, worker, arg);
    }

    // 等待线程完成
    for (int i = 0; i < MAX_THREAD; i++) {
        pthread_join(tid[i], NULL);
    }
    gettimeofday(&end, NULL); // 结束计时
    double time_use = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec); // 计算时间差，单位为微秒

    // 销毁信号量
    for (int i = 0; i < N; i++) {
        sem_destroy(&sem[i]);
    }

    // 回代求解
    for (int i = N - 1; i >= 0; i--) {
        x[i] = b[i] / A[i][i];
        for (int j = 0; j < i; j++) {
            b[j] -= A[j][i] * x[i];
        }
    }

    // 输出结果
    for (int i = 0; i < N; i++) {
        printf("%lf ", x[i]);
    }
    printf("\n");

    printf("Time used: %lf ms\n", time_use / 1000); // 输出时间

    return 0;
}