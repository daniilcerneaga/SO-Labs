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

/* Lab 11, Part 3: the writer publishes the message before posting ready. */
static char message[32];
static sem_t ready;

static void *writer(void *arg)
{
    (void)arg;
    sleep(1);
    strcpy(message, "hello");
    printf("A: message written\n");
    SEM_CHECK(sem_post(&ready));
    return NULL;
}

static void *reader(void *arg)
{
    (void)arg;
    printf("B: waiting for the message...\n");
    wait_for(&ready);
    printf("B: I read \"%s\"\n", message);
    return NULL;
}

int main(void)
{
    SEM_CHECK(sem_init(&ready, 0, 0));
    pthread_t a, b;
    CHECK(pthread_create(&b, NULL, reader, NULL));
    CHECK(pthread_create(&a, NULL, writer, NULL));
    CHECK(pthread_join(a, NULL));
    CHECK(pthread_join(b, NULL));
    SEM_CHECK(sem_destroy(&ready));
    return 0;
}
