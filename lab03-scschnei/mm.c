/*
 * mm-implicit.c - The slow, but memory-efficient malloc package.
 *
 * In this approach, a block is allocated by either finding a free block in the
 * heap using find first, or if none is found, increasing the size of the heap.
 * If the block found is significanly larger than the desired size, the block
 * is made into the desired allocated section and a new free block is made with
 * the rest. Blocks are coalesced in this implementation.
 *
 * This code is correct but slow
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define CHUNKSIZE (1<<12)
#define PACK(size, alloc) ((size) | (alloc))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
/* put a value at an address */
#define PUT(p, val) (*(size_t *)(p) = (val))
/* access header of block */
#define HDR_PTR(p)  ((size_t*)(((char*)(p)) - SIZE_T_SIZE))
/* access size of block */
#define GET_SIZE(p) (*(size_t*)p & ~0x7)
/* access allocation flag of block */
#define GET_ALLOC(p) (*(size_t*)p & 0x1)
/* access footer of block */
#define FTR_PTR(p) ((char *)(p) + GET_SIZE(HDR_PTR(p)) - 2 * SIZE_T_SIZE)
/* access next memory block */
#define NEXT_BLKP(p) ((char *)(p) + GET_SIZE(((char *)(p) - SIZE_T_SIZE)))
/* access previous memory block */
#define PREV_BLKP(p) ((char *)(p) - GET_SIZE(((char *)(p) - 2 * SIZE_T_SIZE)))



/*
 * Finds the first block of appropriate size or signals that none was found
 */
 void *find_first(size_t size) {
   unsigned char *p = mem_heap_lo() + 3 * SIZE_T_SIZE;
  // epilougue has size 0 so until that is hit, look for blocks
   while (GET_SIZE(p) != 0) {
     if (GET_ALLOC(p) == 0 && (size < GET_SIZE(p))) {
       return(p + SIZE_T_SIZE);
     }
       p += (GET_SIZE(p) & ~1);
   }
   return NULL;
}

/*
 * Merges blocks if a block is freed next to a free block
 */
static void *coalesce(void *ptr) {
  size_t prev_full = GET_ALLOC(FTR_PTR(PREV_BLKP(ptr)));
  size_t next_full = GET_ALLOC(HDR_PTR(NEXT_BLKP(ptr)));
  size_t new_size = GET_SIZE(HDR_PTR(ptr));
  /* if next and prev are full just return the ptr */
  if (prev_full && next_full) {
    return ptr;
  }
  /* if prev is full and next isnt then join current and next */
  else if (prev_full && !next_full) {
    new_size += GET_SIZE(HDR_PTR(NEXT_BLKP(ptr)));
    PUT(HDR_PTR(ptr), new_size);
    PUT(FTR_PTR(ptr), new_size);
  }
  /* vise versa for the opposite case */
  else if (!prev_full && next_full) {
    new_size += GET_SIZE(HDR_PTR(PREV_BLKP(ptr)));
    PUT(FTR_PTR(ptr), new_size);
    PUT(HDR_PTR(PREV_BLKP(ptr)), new_size);
    ptr = PREV_BLKP(ptr);
  }
  /* if both are empty then join all 3 */
  else {
    new_size += GET_SIZE(HDR_PTR(PREV_BLKP(ptr)));
    new_size += GET_SIZE(HDR_PTR(NEXT_BLKP(ptr)));
    PUT(HDR_PTR(PREV_BLKP(ptr)), new_size);
    PUT(FTR_PTR(NEXT_BLKP(ptr)), new_size);
    ptr = PREV_BLKP(ptr);
  }
  return ptr;
}

/* add size to the heap in the form of a free block of size extend */
static void *extend_heap(size_t extend) {
  unsigned char *start;
  size_t size = ALIGN(extend);
  if ((long)(start = mem_sbrk(size)) == -1) {
    return NULL;
  }
  PUT(HDR_PTR(start), size | 0);
  PUT(FTR_PTR(start), size | 0);
  PUT(HDR_PTR(NEXT_BLKP(start)), 1);

  return start;
}


/*
 * mm_init - Called when a new trace starts.
 */
int mm_init(void) {
  unsigned char *heap = mem_heap_lo();
  /* create memory for start of heap */
  if ((heap = mem_sbrk(4 * SIZE_T_SIZE)) == (void *)-1) {
    return -1;
  }
  PUT(heap, 0); // buffer
  PUT(heap + SIZE_T_SIZE, (2 * SIZE_T_SIZE) | 1); // prolouge header
  PUT(heap + 2 * SIZE_T_SIZE, (2 * SIZE_T_SIZE) | 1); // prolougue footer
  PUT(heap + 3 * SIZE_T_SIZE, 0 | 1); // epilougue
  heap += (4*SIZE_T_SIZE);


  char *start;
  /* create begining of heap where things can be allocated */
  if ((start = extend_heap(CHUNKSIZE/SIZE_T_SIZE)) == NULL) {
    return -1;
  }

  return 0;
}


/*
 * malloc - Allocate a block by incrementing the brk pointer.
 *      Always allocate a block whose size is a multiple of the alignment.
 */
void *malloc(size_t size) {
  if (size <= 0) {
    return NULL;
  }
  /* make sure block is of appropriate size */
  size_t newsize = ALIGN(size + 2 * SIZE_T_SIZE);
  unsigned char *p = find_first(newsize);

  /* if no free blocks just extend the heap */
  if (p == NULL) {
    if ((p = extend_heap(newsize)) == NULL) {
      return NULL;
    }
    *HDR_PTR(p) = *HDR_PTR(p) | 1;
    *FTR_PTR(p) = *FTR_PTR(p) | 1;
    return p;
  }
  else {
    /* if we find a block but it's too big we split */
    if (GET_SIZE(HDR_PTR(p)) - newsize >= 2 * SIZE_T_SIZE) {
      size_t free_size = GET_SIZE(HDR_PTR(p)) - newsize;
      PUT(HDR_PTR(p), newsize | 1);
      PUT(FTR_PTR(p), newsize | 1);
      PUT(HDR_PTR(NEXT_BLKP(p)), free_size);
      PUT(FTR_PTR(NEXT_BLKP(p)), free_size);

    }
    else {
      PUT(HDR_PTR(p), GET_SIZE(HDR_PTR(p)) | 1);
      PUT(FTR_PTR(p), GET_SIZE(FTR_PTR(p)) | 1);
    }

    return p;
  }
}

/*
 * free - We don't know how to free a block.  So we ignore this call.
 *      Computers have big memories; surely it won't be a problem.
 */
void free(void *ptr) {
  /* check if free call is valid */
  if ((long)mem_heap_lo() <= (long) ptr && (long)ptr <= (long)mem_heap_hi()) {
    /* mark as free */
    *HDR_PTR(ptr) =  *HDR_PTR(ptr) & ~1;
    *FTR_PTR(ptr) = * FTR_PTR(ptr) & ~1;
    coalesce(ptr);
  }
}

/*
 * realloc - Change the size of the block by mallocing a new block,
 *      copying its data, and freeing the old block.
 **/
void *realloc(void *oldptr, size_t size) {
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        free(oldptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(oldptr == NULL) {
        return malloc(size);
    }

    newptr = malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = *HDR_PTR(oldptr);
    if(size < oldsize) oldsize = size;
    memcpy(newptr, oldptr, oldsize);

    /* Free the old block. */
    free(oldptr);

    return newptr;
}

/*
 * calloc - Allocate the block and set it to zero.
 */
void *calloc (size_t nmemb, size_t size) {
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
}

/*
 * mm_checkheap - check prolouge and epilouge, check the address alignment,
 * check
 */
void mm_checkheap(int verbose) {
  unsigned char* p = mem_heap_lo() + 4 * SIZE_T_SIZE;
  size_t free = 0;
  if (GET_SIZE((mem_heap_hi() - 7)) != 0) {
    printf("Epilougue block incorrect\n");
  }
  if (GET_SIZE((mem_heap_lo() + 8)) != 17 || GET_SIZE((mem_heap_lo() + 16)) != 17) {
    printf("Prolougue header or footer inccorect\n");
  }

  while (GET_SIZE(HDR_PTR(p)) != 0) {
    if (GET_SIZE(HDR_PTR(p)) != GET_SIZE(FTR_PTR(p))) {
      printf("HEADERS AND FOOTERS DONT MATCH\n");
    }
    if (ALIGN(GET_SIZE(HDR_PTR(p))) != GET_SIZE(HDR_PTR(p))) {
      printf("Addresses not aligned\n");
    }
    if (GET_ALLOC(HDR_PTR(p)) == 0) {
      if (free == 1) {
        printf("coalesce not correct\n");
      }
      free = 1;
    }
    if (GET_ALLOC(HDR_PTR(p)) == 1) {
      free = 0;
    }
      p += (GET_SIZE(p) & ~1) - SIZE_T_SIZE;
  }
}
