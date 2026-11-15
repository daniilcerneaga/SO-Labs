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

#include <errno.h>
#include <semaphore.h>

#define SEM_CHECK(call) do { \
    if ((call) == -1) { perror(#call); exit(EXIT_FAILURE); } \
} while (0)

static void wait_for(sem_t *semaphore)
{
    int result;
    do { result = sem_wait(semaphore); } while (result == -1 && errno == EINTR);
    SEM_CHECK(result);
}

/* Lab 11, Part 2: a semaphore with one permit provides mutual exclusion. */
#define N 1000000
static long counter = 0;
static sem_t s;

static void *add(void *arg)
{
    (void)arg;
    for (int i = 0; i < N; ++i) {
        wait_for(&s);
        counter++;
        SEM_CHECK(sem_post(&s));
    }
    return NULL;
}

int main(void)
{
    SEM_CHECK(sem_init(&s, 0, 1));
    pthread_t a, b;
    CHECK(pthread_create(&a, NULL, add, NULL));
    CHECK(pthread_create(&b, NULL, add, NULL));
    CHECK(pthread_join(a, NULL));
    CHECK(pthread_join(b, NULL));
    printf("counter = %ld (expected %d)\n", counter, 2 * N);
    SEM_CHECK(sem_destroy(&s));
    return 0;
}
