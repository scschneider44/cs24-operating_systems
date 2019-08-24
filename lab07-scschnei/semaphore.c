/*
 * General implementation of semaphores.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "sthread.h"
#include "semaphore.h"
#include "queue.h"

/*
 * The semaphore data structure contains TODO
 */
struct _semaphore {
    int i;
    /* I used a queue as the data structure so that we can deal with threads
     * fairly and have a FIFO structure
     */
    Queue *blocked;
};

/************************************************************************
 * Top-level semaphore implementation.
 */

/*
 * Allocate a new semaphore.  The initial value of the semaphore is
 * specified by the argument.
 */
Semaphore *new_semaphore(int init) {

    Semaphore *semp = malloc(sizeof(Semaphore));
    if (semp == NULL) {
      printf("No memory available for semaphore\n");
      exit(1);
    }

    Queue *blocked_q = malloc(sizeof(Queue));
    if (blocked_q == NULL) {
      printf("No memory available for semaphore queue\n");
      exit(1);
    }

    semp->blocked = blocked_q;
    semp->i = init;

    return semp;
}

/*
 * Decrement the semaphore.
 * This operation must be atomic, and it blocks iff the semaphore is zero.
 */
void semaphore_wait(Semaphore *semp) {
    assert(semp != NULL);
    __sthread_lock(); // Make sure this is atomic
    while (semp->i == 0) {
      // If no room then block thread
      queue_append(semp->blocked, sthread_current());
      sthread_block();
      __sthread_lock(); // block calls unlock so we have to relock
    }
    semp->i--;
    // The incrementing is done so we can unlock
    __sthread_unlock();
}

/*
 * Increment the semaphore.
 * This operation must be atomic.
 */
void semaphore_signal(Semaphore *semp) {
    assert(semp != NULL);
    // Make sure this is atomic
    __sthread_lock();
    semp->i++;
    // If there is a thread in the queue, unblock it
    if (!queue_empty(semp->blocked)) {
      Thread *unblock = queue_take(semp->blocked);
      sthread_unblock(unblock);
    }
    // The incrementing and unblocking is done so we can unlock
    __sthread_unlock();
}
