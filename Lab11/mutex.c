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

/* Lab 11, Part 2: one owner for the complete increment. */
#define N 1000000
static long counter = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *add(void *arg)
{
    (void)arg;
    for (int i = 0; i < N; ++i) {
        CHECK(pthread_mutex_lock(&lock));
        counter++;
        CHECK(pthread_mutex_unlock(&lock));
    }
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
    CHECK(pthread_mutex_destroy(&lock));
    return 0;
}
