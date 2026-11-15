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

/* Lab 12, Part 3: reader preference. A continuous reader stream can starve writers. */
#define READERS 3
#define WRITERS 2
static int value = 0;
static int readers = 0;
static sem_t rlock, room;

static void *reader(void *arg)
{
    const int id = *(const int *)arg;
    wait_for(&rlock);
    if (++readers == 1)
        wait_for(&room);
    /* Keep the diagnostic counter read under rlock too. */
    printf("reader %d reads %d   (readers inside: %d)\n", id, value, readers);
    SEM_CHECK(sem_post(&rlock));
    usleep(200000);

    wait_for(&rlock);
    if (--readers == 0)
        SEM_CHECK(sem_post(&room));
    SEM_CHECK(sem_post(&rlock));
    return NULL;
}

static void *writer(void *arg)
{
    const int id = *(const int *)arg;
    wait_for(&room);
    value += 10;
    printf("writer %d writes %d  (alone)\n", id, value);
    usleep(200000);
    SEM_CHECK(sem_post(&room));
    return NULL;
}

int main(void)
{
    SEM_CHECK(sem_init(&rlock, 0, 1));
    SEM_CHECK(sem_init(&room, 0, 1));
    pthread_t r[READERS], w[WRITERS];
    int reader_ids[READERS], writer_ids[WRITERS] = {1, 2};
    CHECK(pthread_create(&w[0], NULL, writer, &writer_ids[0]));
    for (int i = 0; i < READERS; ++i) {
        reader_ids[i] = i + 1;
        CHECK(pthread_create(&r[i], NULL, reader, &reader_ids[i]));
    }
    CHECK(pthread_create(&w[1], NULL, writer, &writer_ids[1]));
    for (int i = 0; i < READERS; ++i)
        CHECK(pthread_join(r[i], NULL));
    for (int i = 0; i < WRITERS; ++i)
        CHECK(pthread_join(w[i], NULL));
    SEM_CHECK(sem_destroy(&rlock));
    SEM_CHECK(sem_destroy(&room));
    return 0;
}
