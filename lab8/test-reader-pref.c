#include <pthread.h>
#include "rwlock.h"

long idx;
long *readerAcquireTime;
long *readerReleaseTime;
long *writerAcquireTime;
long *writerReleaseTime;

struct read_write_lock rwlock;
pthread_mutex_t spinlock; // since MacOS used mutex instead of pthread_mutex_t 

long *max_element(long *arr, long num_elements) {
    long *max = arr;
    for (int i = 1; i < num_elements; i++) {
        if (arr[i] > *max) {
            max = &(arr[i]);
        }
    }
    return max;
}

long *min_element(long *arr, long num_elements) {
    long *min = arr;
    for (int i = 1; i < num_elements; i++) {
        if (arr[i] < *min) {
            min = &(arr[i]);
        }
    }
    return min;
}

void *reader(void* arg) {
    int threadNumber = *((int *)arg);

    // Occupying the Lock
    readerLock(&rwlock);

    pthread_mutex_lock(&spinlock);
    readerAcquireTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);

    usleep(10000);

    pthread_mutex_lock(&spinlock);
    readerReleaseTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);

    // Releasing the Lock
    readerUnlock(&rwlock);

    return NULL;
}

void *writer(void* arg) {
    int threadNumber = *((int *)arg);

    // Occupying the Lock
    writerLock(&rwlock);

    pthread_mutex_lock(&spinlock);
    writerAcquireTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);

    usleep(10000);

    pthread_mutex_lock(&spinlock);
    writerReleaseTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);

    // Releasing the Lock
    writerUnlock(&rwlock);

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t *threads;

    initializeReadWriteLock(&rwlock);
    pthread_mutex_init(&spinlock, NULL);  // pthread_spin_init 대신 pthread_mutex_init

    int read_num_threads;
    int write_num_threads;

    read_num_threads = atoi(argv[1]);
    write_num_threads = atoi(argv[2]);

    idx = 0;
    readerAcquireTime = (long *)malloc(read_num_threads * 2 * (sizeof(long)));
    readerReleaseTime = (long *)malloc(read_num_threads * 2 * (sizeof(long)));
    writerAcquireTime = (long *)malloc(write_num_threads * (sizeof(long)));
    writerReleaseTime = (long *)malloc(write_num_threads * (sizeof(long)));

    int num_threads = 2 * read_num_threads + write_num_threads;

    threads = (pthread_t*)malloc(num_threads * (sizeof(pthread_t)));

    int count = 0;
    for (int i = 0; i < read_num_threads; i++) {
        int *arg = (int *)malloc((sizeof(int)));
        if (arg == NULL) {
            printf("Couldn't allocate memory for thread arg.\n");
            exit(EXIT_FAILURE);
        }
        *arg = i;
        int ret = pthread_create(threads+count, NULL, reader, (void*)arg);
        if (ret) {
            printf("Error - pthread_create() return code: %d\n", ret);
            exit(EXIT_FAILURE);
        }
        count++;
    }

    for (int i = 0; i < write_num_threads; i++) {
        int *arg = (int *)malloc((sizeof(int)));
        if (arg == NULL) {
            printf("Couldn't allocate memory for thread arg.\n");
            exit(EXIT_FAILURE);
        }
        *arg = i;
        int ret = pthread_create(threads+count, NULL, writer, (void*)arg);
        if (ret) {
            printf("Error - pthread_create() return code: %d\n", ret);
            exit(EXIT_FAILURE);
        }
        count++;
    }

    for (int i = 0; i < read_num_threads; i++) {
        int *arg = (int *)malloc((sizeof(int)));
        if (arg == NULL) {
            printf("Couldn't allocate memory for thread arg.\n");
            exit(EXIT_FAILURE);
        }
        *arg = read_num_threads + i;
        int ret = pthread_create(threads+count, NULL, reader, (void*)arg);
        if (ret) {
            printf("Error - pthread_create() return code: %d\n",ret);
            exit(EXIT_FAILURE);
        }
        count++;
    }

    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Tests and other code...

    printf("PASSED\n");
}
