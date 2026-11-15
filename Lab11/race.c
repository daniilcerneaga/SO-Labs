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

/* Lab 11, Part 1: intentionally unsynchronized. Build without optimization.
 * gcc -Wall -Wextra -Werror -O0 -pthread -o race race.c
 * The data race is deliberate; this is not a correct shared counter.
 */
#define N 1000000
static long counter = 0;

static void *add(void *arg)
{
    (void)arg;
    for (int i = 0; i < N; ++i)
        counter++;
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    CHECK(pthread_create(&a, NULL, add, NULL));
    CHECK(pthread_create(&b, NULL, add, NULL));
    CHECK(pthread_join(a, NULL));
    CHECK(pthread_join(b, NULL));
    printf("counter = %ld (expected %d)\n", counter, 2 * N);
    return 0;
}
