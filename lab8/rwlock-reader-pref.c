#include "rwlock.h"

// allow reader while writer is waiting

void initializeReadWriteLock(struct read_write_lock * rw) {
    // start thread, mutex
    pthread_mutex_init(&rw->lock_mutex, NULL);
    pthread_cond_init(&rw->read_cond, NULL);    
    pthread_cond_init(&rw->write_cond, NULL);    
    rw->active_readers = 0;            
    rw->waiting_writers = 0;     
    rw->active_writer = 0; 
}

void readerLock(struct read_write_lock *rw) {
    // invoked by a reader thread before entering a read-only critical section

    pthread_mutex_lock(&rw->lock_mutex);

    // no new thread if there's waiting writing thread
    while (rw->active_writer > 0) {
        pthread_cond_wait(&rw->read_cond, &rw->lock_mutex);
    }
    rw->active_readers++;

    pthread_mutex_unlock(&rw->lock_mutex);
}

void readerUnlock(struct read_write_lock * rw) {
    // invoked by the reader when exiting the critical section

    pthread_mutex_lock(&rw->lock_mutex);

    rw->active_readers--;
    if (rw->active_readers == 0) {
        pthread_cond_signal(&rw->write_cond); // notify there's no more read thread
    }

    pthread_mutex_unlock(&rw->lock_mutex);
}


void writerLock(struct read_write_lock *rw) {
    pthread_mutex_lock(&rw->lock_mutex);

    rw->waiting_writers++; // flag there is writer on the line 

    // check if thread is empty
    while (rw->active_writer > 0 || rw->active_readers > 0) {
        pthread_cond_wait(&rw->write_cond, &rw->lock_mutex);
    }

    rw->waiting_writers--;
    rw->active_writer = 1;
    pthread_mutex_unlock(&rw->lock_mutex);
}

void writerUnlock(struct read_write_lock *rw) {
    pthread_mutex_lock(&rw->lock_mutex);

    rw->active_writer = 0;
    pthread_cond_broadcast(&rw->read_cond); 
    pthread_cond_signal(&rw->write_cond);  

    pthread_mutex_unlock(&rw->lock_mutex);
}
