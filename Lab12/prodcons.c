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

/* Lab 12, Part 1: reserve a slot before locking the ring buffer. */
#define SIZE 4
#define ITEMS 8
static int buf[SIZE];
static int in = 0, out = 0, count = 0;
static sem_t empty, full;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *producer(void *arg)
{
    (void)arg;
    for (int item = 1; item <= ITEMS; ++item) {
        wait_for(&empty);
        CHECK(pthread_mutex_lock(&lock));
        buf[in] = item;
        in = (in + 1) % SIZE;
        count++;
        printf("put  %d   (in buffer: %d)\n", item, count);
        CHECK(pthread_mutex_unlock(&lock));
        SEM_CHECK(sem_post(&full));
    }
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    for (int k = 0; k < ITEMS; ++k) {
        wait_for(&full);
        CHECK(pthread_mutex_lock(&lock));
        int item = buf[out];
        out = (out + 1) % SIZE;
        count--;
        printf("            take %d   (in buffer: %d)\n", item, count);
        CHECK(pthread_mutex_unlock(&lock));
        SEM_CHECK(sem_post(&empty));
        usleep(300000);
    }
    return NULL;
}

int main(void)
{
    SEM_CHECK(sem_init(&empty, 0, SIZE));
    SEM_CHECK(sem_init(&full, 0, 0));
    pthread_t p, c;
    CHECK(pthread_create(&c, NULL, consumer, NULL));
    CHECK(pthread_create(&p, NULL, producer, NULL));
    CHECK(pthread_join(p, NULL));
    CHECK(pthread_join(c, NULL));
    SEM_CHECK(sem_destroy(&empty));
    SEM_CHECK(sem_destroy(&full));
    CHECK(pthread_mutex_destroy(&lock));
    return 0;
}
