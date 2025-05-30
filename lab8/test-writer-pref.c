#include "pthread.h"
#include "rwlock.h"

long idx;
long *readerAcquireTime;
long *readerReleaseTime;
long *writerAcquireTime;
long *writerReleaseTime;

struct read_write_lock rwlock;
pthread_mutex_t spinlock;  // pthread_spinlock_t -> pthread_mutex_t로 변경

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

    pthread_mutex_lock(&spinlock);  // pthread_spin_lock -> pthread_mutex_lock
    readerAcquireTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);  // pthread_spin_unlock -> pthread_mutex_unlock

    usleep(10000);

    pthread_mutex_lock(&spinlock);  // pthread_spin_lock -> pthread_mutex_lock
    readerReleaseTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);  // pthread_spin_unlock -> pthread_mutex_unlock

    // Releasing the Lock
    readerUnlock(&rwlock);

    return NULL;
}

void *writer(void* arg) {
    int threadNumber = *((int *)arg);

    // Occupying the Lock
    writerLock(&rwlock);

    pthread_mutex_lock(&spinlock);  // pthread_spin_lock -> pthread_mutex_lock
    writerAcquireTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);  // pthread_spin_unlock -> pthread_mutex_unlock

    usleep(10000);

    pthread_mutex_lock(&spinlock);  // pthread_spin_lock -> pthread_mutex_lock
    writerReleaseTime[threadNumber] = idx;
    idx++;
    pthread_mutex_unlock(&spinlock);  // pthread_spin_unlock -> pthread_mutex_unlock

    // Releasing the Lock
    writerUnlock(&rwlock);

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t *threads;

    initializeReadWriteLock(&rwlock);
    pthread_mutex_init(&spinlock, NULL);  // pthread_spin_init -> pthread_mutex_init

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
 
    long *max_reader_acquire_time_first_half = max_element(readerAcquireTime, read_num_threads);
    long *min_reader_release_time_first_half = min_element(readerReleaseTime, read_num_threads);
    long *max_reader_release_time_first_half = max_element(readerReleaseTime, read_num_threads);
    long *min_reader_acquire_time_second_half = min_element(&(readerAcquireTime[read_num_threads]), read_num_threads);
    long *max_reader_acquire_time_second_half = max_element(&(readerAcquireTime[read_num_threads]), read_num_threads);
    long *min_reader_release_time_second_half = min_element(&(readerReleaseTime[read_num_threads]), read_num_threads);
    long *min_writer_acquire_time = min_element(writerAcquireTime, write_num_threads);
    long *max_writer_release_time = max_element(writerReleaseTime, write_num_threads);

    // check if all readers get lock immediately in second half
    if  ((read_num_threads > 0) && (*max_reader_acquire_time_second_half > *min_reader_release_time_second_half)){
        printf("Reader should not wait to acquire lock in second half\n");
        exit(0);
    }

    // check if no reader is holding while a writer is still holding lock
    if ((read_num_threads > 0) && (write_num_threads > 0) && (*min_reader_acquire_time_second_half < *max_writer_release_time)){
        printf("Reader can not acquire lock when writer is holding a lock\n");
        exit(0);
    }

    // check if writer exited immediately
    for (int i = 0; i < write_num_threads; i++)
        if ((writerReleaseTime[i] - writerAcquireTime[i]) != 1)
        {
            printf("No reader/writer is allowed when a writer holds lock\n");
            exit(0);
        }

    printf("PASSED\n");
}
