#ifndef __RECEIVER_THREAD__
#define __RECEIVER_THREAD__

#include <cstdio>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

pthread_t receiver_thread;

void *receiver_thread_function(void *){
    printf("---------- receiver thread ---------\n");
    int cnt=0;
    while(cnt<100000){
        cnt++;
        printf("1");
    }
}


#endif