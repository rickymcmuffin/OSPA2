#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define BUFFER_SIZE 1024

typedef struct {
    char buffer[BUFFER_SIZE];
    int start;
    int end;
    int full; // if start and end are equal, full determines whether it is empty or full
} word_buffer;

word_buffer g_buffer;

#endif /* SHARED_MEMORY_H */

