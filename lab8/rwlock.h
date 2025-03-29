#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct read_write_lock {

};

void initalizeReadWriteLock(struct read_write_lock * rw);
void readerLock(struct read_write_lock * rw);
void readerUnlock(struct read_write_lock * rw);
void writerLock(struct read_write_lock * rw);
void writerUnlock(struct read_write_lock * rw);
