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


/* Initialize the policy. */
int policy_init(int max_resident) {
    fprintf(stderr, "Using FIFO eviction policy.\n\n");
    // initialize the queue
    loaded = malloc(sizeof(Queue));
    // check if malloc succeeded
    if (loaded == NULL) {
      printf("No memory available for loaded queue\n");
      exit(1);
    }

    /* Return nonzero if initialization succeeded. */
    return 1;
}


/* Clean up the data used by the page replacement policy. */
void policy_cleanup(void) {
    // Empty queue using queue take
    while (!queue_empty(loaded)) {
      queue_take(loaded);
    }
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    // append to queue to map
    queue_append(loaded, page);
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick(void) {
    /* Do nothing! */
}


/* Choose a page to evict from the collection of mapped pages.  Then, record
 * that it is evicted.  This is very simple since we are implementing a fifo
 * page-replacement policy.
 */
page_t choose_and_evict_victim_page(void) {
    page_t victim;

    /* Take page from head of queue */
    victim = queue_take(loaded);

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}
