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

#include <unistd.h>

/* Lab 12, Part 2: the predicate stores state; notification only wakes a waiter. */
static char message[32];
static int ready = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed = PTHREAD_COND_INITIALIZER;

static void *writer(void *arg)
{
    (void)arg;
    sleep(1);
    CHECK(pthread_mutex_lock(&lock));
    strcpy(message, "hello");
    ready = 1;
    printf("A: message written\n");
    CHECK(pthread_cond_signal(&changed));
    CHECK(pthread_mutex_unlock(&lock));
    return NULL;
}

static void *reader(void *arg)
{
    (void)arg;
    CHECK(pthread_mutex_lock(&lock));
    while (!ready) {
        printf("B: not ready, I sleep\n");
        CHECK(pthread_cond_wait(&changed, &lock));
    }
    printf("B: I read \"%s\"\n", message);
    CHECK(pthread_mutex_unlock(&lock));
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    CHECK(pthread_create(&b, NULL, reader, NULL));
    CHECK(pthread_create(&a, NULL, writer, NULL));
    CHECK(pthread_join(a, NULL));
    CHECK(pthread_join(b, NULL));
    CHECK(pthread_cond_destroy(&changed));
    CHECK(pthread_mutex_destroy(&lock));
    return 0;
}
