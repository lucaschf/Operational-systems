#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define SIZE 11

long pow_arr[SIZE];
long sum_value;

void *m_pow() {
    for (int i = 0; i < SIZE; i++)
        pow_arr[i] = (long) pow(2, i);

    pthread_exit(0);
}

void *m_sum() {
    sum_value = 0;

    for (int i = 0; i < SIZE; i++)
        sum_value += pow_arr[i];

    pthread_exit(0);
}

int main() {
    pthread_t thSum;
    pthread_t thPow;

    pthread_attr_t attrPow;
    pthread_attr_t attrSum;

    pthread_attr_init(&attrPow);
    pthread_attr_init(&attrSum);

    pthread_create(&thPow, &attrPow, m_pow, NULL);
    pthread_join(thPow, NULL);

    pthread_create(&thSum, &attrSum, m_sum, NULL);
    pthread_join(thSum, NULL);

    printf("Soma das potencias = %ld\n", sum_value);

    return 0;
}