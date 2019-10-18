

#ifndef __SENDER_THREAD__
#define __SENDER_THREAD__

#include <cstdio>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

pthread_t sender_thread;

void *sender_thread_function(void *){
    printf("---------- sender thread ----------\n");
}


#endif