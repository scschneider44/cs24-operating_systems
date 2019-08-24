/*!
 * A simple test program for exercising the threads API.
 *
 * ----------------------------------------------------------------
 *
 * This program includes two looping functions, one that subtracts one from
 * the arguement passed and one that adds two to the arguement passed
 */
#include <stdio.h>
#include <stdlib.h>
#include "sthread.h"


/*
 * Thread 1 subtracts 1 from arg.
 * Thread 2 adds 2 to arg".
 */

/*! This thread-function subtracts 1 over and over again */
static void loop1(void *arg) {
    while(1) {

        printf("%d\n", *(int *)arg);
        *(int *)arg -= 1;
        sthread_yield();
    }
}

/*! This thread-function adds 2 over and over again */
static void loop2(void *arg) {
    while(1) {
        printf("%d\n", *(int *)arg);
        *(int *)arg += 2;
        sthread_yield();
    }
}

/*
 * The main function starts the two loops in separate threads,
 * the start the thread scheduler.
 */
int main(int argc, char **argv) {
    // make an int pointer to hold the argument
    int *arg = malloc(sizeof(int));
    // make sure malloc works
    if (arg == NULL) {
      printf("No memory for arg pointer\n");
      return;
    }
    // initialize argument to run
    *arg = 1;
    // create the threads
    sthread_create(loop1, (void *) arg);
    sthread_create(loop2, (void *) arg);
    // run the program
    sthread_start();
    // free the pointer
    free(arg);
    return 0;
}
