#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGESIZE 4096   // size of memory to allocate from OS
#define MINALLOC 8      // allocations will be 8 bytes or multiples of it

static char *memory; // to be allocated maddr via mmap
static char *current_position = NULL; // location of mblock

typedef struct Block {
    size_t size;
    struct Block *next     
} Block;

int init_alloc() {
    // 4KB mpage
    memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        return -1;
    }
    return 0;
}

int cleanup(){
    if (munmap(memory, PAGESIZE) == -1) {
        return -1;
    } 
    return 0;
}

char *alloc(int size) {
    // malloc multiple of 8 bytes

    if (size % MINALLOC != 0 || size <= 0) {
        return NULL;
    }

    if (current_position == NULL) {
        current_position = memory; // alloc starting point
    }

    // check enough mem
    if (current_position + size > memory + PAGESIZE) {
        return NULL;
    }

    // alloc mem
    char *allocated_mem = current_position;
    current_position += size; // update current position
    return allocated_mem;
}

void dealloc(char *allocated_mem) {
    Block *block = (Block *)(allocated_mem - sizeof(Block)); // find Block itself

    if (munmap((void *)block, block->size + sizeof(Block)) == -1) {
        printf("Error during memory unmap.\n");
    }
}
