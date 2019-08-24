/*============================================================================
 * Implementation of the RANDOM page replacement policy.
 *
 * We don't mind if paging policies use malloc() and free(), just because it
 * keeps things simpler.  In real life, the pager would use the kernel memory
 * allocator to manage data structures, and it would endeavor to allocate and
 * release as little memory as possible to avoid making the kernel slow and
 * prone to memory fragmentation issues.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmpolicy.h"
#include "virtualmem.h"
#include "queue.h"

/*============================================================================
 * Policy Implementation
 */


/* The queue of pages that are currently resident. */
static Queue *loaded;


/* Initialize the policy.  Return nonzero for success, 0 for failure. */
int policy_init(int max_resident) {
    fprintf(stderr, "Using CLRU eviction policy.\n\n");
    // initialize queue
    loaded = malloc(sizeof(Queue));
    // make sure malloc succeeded
    if (loaded == NULL) {
      printf("No memory available for loaded queue\n");
      exit(1);
    }

    /* Return nonzero if initialization succeeded. */
    return 1;
}


/* Clean up the data used by the page replacement policy. */
void policy_cleanup(void) {
    // empty queue using queue take
    while (!queue_empty(loaded)) {
      queue_take(loaded);
    }
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    // use queue append to store mapped pages
    queue_append(loaded, page);
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick(void) {
    // if queue is empty we don't need to move anything
    if (queue_empty(loaded)) {
        return;
    }
    // get head and tail of queue before moves
    QueueNode *end = loaded->tail;
    QueueNode *curr = loaded->head;
    QueueNode *next;
    // go through the queue once
    while(curr != end) {
      next = curr->next;
      // if the page is accessed move to the back of queue and reset access
      if (is_page_accessed(curr->page)) {
          page_t curr_page = curr->page;
          // remove page
          queue_remove(loaded, curr_page);
          // put it at back of queue
          queue_append(loaded, curr_page);
          //reset access and permission
          clear_page_accessed(loaded->tail->page);
          set_page_permission(loaded->tail->page, PAGEPERM_NONE);
      }
      // iterate through
      curr = next;
    }
}


/* Choose a page to evict from the collection of mapped pages.  Then, record
 * that it is evicted.  This is very simple since we are implementing a fifo
 * page-replacement policy with a time element .
 */
page_t choose_and_evict_victim_page(void) {
    page_t victim;

    /* take from head of Queue */
    victim = queue_take(loaded);

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}
