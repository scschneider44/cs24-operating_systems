/*!
 * A simple test program for exercising the threads API.
 *
 * This program creates 4 threads that should be run different amounts of times
 * to explore the functionality of the threads
 */
#include <stdio.h>
#include <stdlib.h>
#include "sthread.h"

/*! This thread-function runs and yields a thread the number of times that
* corresponds with the arg */
static void loop1(void *arg) {
    int iters = *(int *)arg;
    // iterate arg times
    for(int i = 0; i < iters; i++) {
        printf("Iteration number: %d\n", i);
        sthread_yield();
    }
    printf("Loop done\n");
}


/*
 * The main function starts the two loops in separate threads,
 * the start the thread scheduler.
 */
int main(int argc, char **argv) {
    // make pointers and initialize them
    int *arg1 = malloc(sizeof(int));
    int *arg2 = malloc(sizeof(int));
    int *arg3 = malloc(sizeof(int));
    int *arg4 = malloc(sizeof(int));
    *arg1 = 5;
    *arg2 = 15;
    *arg3 = 25;
    *arg4 = 35;
    // create threads
    sthread_create(loop1, (void *) arg1);
    sthread_create(loop1, (void *) arg2);
    sthread_create(loop1, (void *) arg3);
    sthread_create(loop1, (void *) arg4);
    // run program
    sthread_start();
    //free pointers
    free(arg1);
    free(arg2);
    free(arg3);
    free(arg4);
    return 0;
}
