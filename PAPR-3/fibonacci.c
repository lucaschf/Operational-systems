#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long *fibonacciSeries;
long seriesSize;

void *fibonacci(void *terms) {
    if ((seriesSize = atoi(terms)) > 0) {
        fibonacciSeries = (long *) malloc(seriesSize * sizeof(long));

        fibonacciSeries[0] = 0;

        if (seriesSize >= 2) {
            fibonacciSeries[1] = 1;

            for (int i = 2; i < seriesSize; i++)
                fibonacciSeries[i] = fibonacciSeries[i - 1] + fibonacciSeries[i - 2];
        }
    }

    pthread_exit(0);
}

void showSeries() {
    if (seriesSize > 0) {
        for (int i = 0; i < seriesSize; i++) {
            printf("%ld\n", fibonacciSeries[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    pthread_attr_t attrThFibonacci;
    pthread_t thFibonacci;

    if (argc != 2) {
        printf("Informe a quantidade de termos da serie.\n");
        return 0;
    }

    pthread_attr_init(&attrThFibonacci);
    pthread_create(&thFibonacci, &attrThFibonacci, fibonacci, argv[1]);
    pthread_join(thFibonacci, NULL);
    showSeries();

    return 0;
}

