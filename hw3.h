#ifndef HW3_H
#define HW3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>  // Added to fix SIGPIPE error

#define BUFFER_SIZE 2048
#define MAX_CLIENTS 100

// Helper macro for checking errors
#define CHECK(condition, msg) \
    do { \
        if (condition) { \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#endif // HW3_H
