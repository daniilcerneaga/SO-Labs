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

#include <unistd.h>

/* Lab 11, Part 4: permits limit occupancy; the mutex protects the log. */
#define PLACES 3
#define CARS 6
static sem_t places;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int inside = 0;

static void *car(void *arg)
{
    const int id = *(const int *)arg;
    wait_for(&places);
    CHECK(pthread_mutex_lock(&lock));
    inside++;
    printf("car %d enters  (inside: %d)\n", id, inside);
    CHECK(pthread_mutex_unlock(&lock));

    sleep(1);

    CHECK(pthread_mutex_lock(&lock));
    inside--;
    printf("car %d leaves  (inside: %d)\n", id, inside);
    CHECK(pthread_mutex_unlock(&lock));
    SEM_CHECK(sem_post(&places));
    return NULL;
}

int main(void)
{
    SEM_CHECK(sem_init(&places, 0, PLACES));
    pthread_t cars[CARS];
    int ids[CARS];
    for (int i = 0; i < CARS; ++i) {
        ids[i] = i + 1;
        CHECK(pthread_create(&cars[i], NULL, car, &ids[i]));
    }
    for (int i = 0; i < CARS; ++i)
        CHECK(pthread_join(cars[i], NULL));
    SEM_CHECK(sem_destroy(&places));
    CHECK(pthread_mutex_destroy(&lock));
    return 0;
}
