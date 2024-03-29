#ifndef QUEUE_H
#define QUEUE_H

#include "virtualmem.h"


/*! A single node of a queue of threads. */
typedef struct _queuenode {
    page_t page;
    struct _queuenode *prev;
    struct _queuenode *next;
} QueueNode;


/*!
 * A queue of threads.  This type is used to keep track of threads in
 * various states within the user-space threaading library.
 */
typedef struct _queue {
    QueueNode *head;  /*!< The first thread in the queue. */
    QueueNode *tail;  /*!< The last thread in the queue. */
} Queue;


int queue_empty(Queue *queuep);
void queue_append(Queue *queuep, page_t page);
page_t queue_take(Queue *queuep);
int queue_remove(Queue *queuep, page_t page);


#endif /* QUEUE_H */
