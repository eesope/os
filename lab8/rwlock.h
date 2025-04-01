#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct read_write_lock {
    pthread_mutex_t lock_mutex; // for every locks
    pthread_cond_t read_cond; // condition var for read thread
    pthread_cond_t write_cond; // condition var for write thread
    int active_readers;            
    int waiting_writers;     
    int active_writer; // is writer working & holding lock
};

void initializeReadWriteLock(struct read_write_lock * rw);
void readerLock(struct read_write_lock * rw);
void readerUnlock(struct read_write_lock * rw);
void writerLock(struct read_write_lock * rw);
void writerUnlock(struct read_write_lock * rw);
