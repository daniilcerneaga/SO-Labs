#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* pthread functions return an error number, rather than setting errno. */
#define CHECK(call) do { \
    int error = (call); \
    if (error != 0) { \
        fprintf(stderr, "%s: %s\n", #call, strerror(error)); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#include <stdatomic.h>

/* Lab 11, optional experiment: an atomic read-modify-write operation. */
#define N 1000000
static atomic_long counter = 0;

static void *add(void *arg)
{
    (void)arg;
    for (int i = 0; i < N; ++i)
        atomic_fetch_add(&counter, 1);
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    CHECK(pthread_create(&a, NULL, add, NULL));
    CHECK(pthread_create(&b, NULL, add, NULL));
    CHECK(pthread_join(a, NULL));
    CHECK(pthread_join(b, NULL));
    printf("counter = %ld (expected %d)\n", atomic_load(&counter), 2 * N);
    return 0;
}
