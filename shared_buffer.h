#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H

#define BUFFER_SIZE 10

typedef struct {
    char buffer[BUFFER_SIZE];
    int start;
    int end;
    int full; // if start and end are equal, full determines whether it is empty or full
} word_buffer;



#endif /* SHARED_MEMORY_H */

