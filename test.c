#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define READ_SIZE 3
#define BUFFER_SIZE 1024

void *readDevice(void *args) {
    int readBytes = *((int *)args);
    // int loopCount = *((int *)(args + sizeof(int)));
    int loopCount = 15;

    for (int x = 0; x < loopCount; x++) {
        int device = open("/dev/charkmod-out", O_RDONLY);
        char fileContents[READ_SIZE + 1]; // Buffer for read contents + null terminator
        ssize_t bytesRead = read(device, fileContents, readBytes);
        fileContents[bytesRead] = '\0'; // Null-terminate the string
        close(device);
        printf("read[%d]: [%s]\n", x, fileContents);
    }
    pthread_exit(NULL);
}

void *writeDevice(void *args) {
    char *inputString = *((char **)args);
    // int loopCount = *((int *)(args + sizeof(char *)));
    int loopCount = 10;

    for (int x = 0; x < loopCount; x++) {
        int device = open("/dev/charkmod-in", O_WRONLY);
        int bytesWritten = write(device, inputString, strlen(inputString));
        close(device);
        printf("write[%d]: %d bytes written.\n", x, bytesWritten);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t writerThread, readerThread;
    int readBytes = READ_SIZE;
    int loopCount = 4;
    char *inputString = "Hello";

    // Create threads and pass arguments
    pthread_create(&writerThread, NULL, writeDevice, (void *)&inputString);
    pthread_create(&readerThread, NULL, readDevice, (void *)&readBytes);

    // Wait for threads to finish
    pthread_join(writerThread, NULL);
    pthread_join(readerThread, NULL);

    printf("Done!\n");

    // Read remaining contents from the device
    int device = open("/dev/charkmod-out", O_RDONLY);
    char remainingContents[BUFFER_SIZE];
    ssize_t bytesRead = read(device, remainingContents, BUFFER_SIZE - 1);
    close(device);

    if (bytesRead >= 0) {
        remainingContents[bytesRead] = '\0'; // Null-terminate the string
        printf("Remaining contents: [%s]\n", remainingContents);
    } else {
        perror("Failed to read remaining contents");
    }

    return 0;
}
