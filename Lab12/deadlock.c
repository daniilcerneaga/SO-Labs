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

/* Lab 12, Part 4: FIXED version. Both threads acquire A before B. */
static pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;

static void *one(void *arg)
{
    (void)arg;
    CHECK(pthread_mutex_lock(&A));
    printf("one: I have A, I want B\n");
    sleep(1);
    CHECK(pthread_mutex_lock(&B));
    printf("one: I have both\n");
    CHECK(pthread_mutex_unlock(&B));
    CHECK(pthread_mutex_unlock(&A));
    return NULL;
}

static void *two(void *arg)
{
    (void)arg;
    CHECK(pthread_mutex_lock(&A));
    printf("two: I have A, I want B\n");
    sleep(1);
    CHECK(pthread_mutex_lock(&B));
    printf("two: I have both\n");
    CHECK(pthread_mutex_unlock(&B));
    CHECK(pthread_mutex_unlock(&A));
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    CHECK(pthread_create(&t1, NULL, one, NULL));
    CHECK(pthread_create(&t2, NULL, two, NULL));
    CHECK(pthread_join(t1, NULL));
    CHECK(pthread_join(t2, NULL));
    printf("done\n");
    CHECK(pthread_mutex_destroy(&A));
    CHECK(pthread_mutex_destroy(&B));
    return 0;
}
