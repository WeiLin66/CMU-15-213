/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*********************************************************
* Macros for block operation
 ********************************************************/
#define WSIZE                                       4
#define DSIZE                                       8
#define CHUNKSIZE                                   (1 << 12)

#define MAX(x, y)                                   ((x) > (y) ? (x) : (y))

/* create header or footer message */
#define PACK(size,  alloc)                          ((size) | (alloc))

/* read & write a word at address p */
#define GET(p)                                      (*(unsigned int*)(p))
#define PUT(p, val)                                 (*(unsigned int*)(p) = (val))

/* get size and allocated bit of a block */
#define GET_SIZE(p)                                 (GET(p) & ~0x7)
#define GET_ALLOC(p)                                (GET(p) & 0x1)

/* get header and footer */
#define HDRP(bp)                                    ((char *)(bp) - WSIZE)
#define FTRP(bp)                                    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* get previous and next meta block by caculating current block point */
#define NEXT_BLKP(bp)                               ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) 
#define PREV_BLKP(bp)                               ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
/*********************************************************
* Macros for replacement policies
 ********************************************************/

#define FIRST_FIT       0
#define NEXT_FIT        1
#define BEST_FIT        2

#define REPLACEMENT     FIRST_FIT
// #define NEXT_FITx

/*********************************************************
* Macros for alignment operations
 ********************************************************/
/* single word (4) or double word (8) alignment */
#define ALIGNMENT       8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size)     (((size) + (ALIGNMENT-1)) & ~0x7)

/* alignment size define */
#define SIZE_T_SIZE     (ALIGN(sizeof(size_t)))

#define MINIMUN_BLOCK   16

/*********************************************************
* Global variables
 ********************************************************/
static char* heap_listp = NULL;

#if (REPLACEMENT == NEXT_FIT)
static char* rover = NULL;
#endif

/*********************************************************
* Function prototypes for internal helper for routines
 ********************************************************/
static void* extend_heap(size_t words);
static void place(void* bp, size_t asize);
static void* find_fit(size_t asize);
static void* coalesce(void* bp);
static void printblock(void* bp);
static void checkheap(int verbose);
static void checkblock(void* bp);

/**
 * @bried merge blocks when conducts free operation
 */ 
static void* coalesce(void* bp){

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

    size_t size = GET_SIZE(HDRP(bp));
    char* ret_pointer = bp;

    /* case 1: none of the previos and next blocks aren free */
    if(prev_alloc && next_alloc){

    }
    /* case 1: previous block is free */
    else if(!prev_alloc && next_alloc){

        unsigned int total_size = size + GET_SIZE(HDRP(PREV_BLKP(bp)));
        ret_pointer = PREV_BLKP(bp);

        PUT(FTRP(bp), PACK(total_size, 0));
        PUT(HDRP(ret_pointer), PACK(total_size, 0));        
    }
    /* case 2: next block is free */
    else if(prev_alloc && !next_alloc){

        unsigned int total_size = size + GET_SIZE(HDRP(NEXT_BLKP(bp)));

        PUT(FTRP(NEXT_BLKP(bp)), PACK(total_size, 0));
        PUT(HDRP(bp), PACK(total_size, 0));        
    }
    /* case 3: both previous and next block are free */
    else if(!prev_alloc && !next_alloc){

        unsigned int total_size = size +            \
            GET_SIZE(HDRP(PREV_BLKP(bp))) +         \
            GET_SIZE(HDRP(NEXT_BLKP(bp)));

        ret_pointer = PREV_BLKP(bp);

        PUT(FTRP(NEXT_BLKP(bp)), PACK(total_size, 0));
        PUT(HDRP(ret_pointer), PACK(total_size, 0));        
    }

    #if (REPLACEMENT == NEXT_FIT)

    #endif

    return ret_pointer;
}

/**
 * @brief extend heap with free block and return its block pointer 
 * note that one word represent 4 bytes
 */ 
static void* extend_heap(size_t words){

    char* bp;
    ssize_t size;
 
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;

    if((long)(bp = (mem_sbrk(size))) == -1){

        return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

/** 
 * @brief place - Place block of asize bytes at start of free block bp 
 * and split if remainder would be at least minimum block size
 */
static void place(void* bp, size_t asize){

    if(bp == NULL || asize <= 0){

        return;
    }

    size_t size = GET_SIZE(HDRP(bp));

    if((size - asize) >= (2*DSIZE)){ // split

        /* current block */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        bp = NEXT_BLKP(bp);

        /* new block */
        PUT(HDRP(bp), PACK((size-asize), 0));
        PUT(FTRP(bp), PACK((size-asize), 0));
    }else{ // no need for spliting

        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
}

/**
 * @brief find a free block that meets the asize requirment
 */ 
static void* find_fit(size_t asize){

    #if (REPLACEMENT == FIRST_FIT)
        void* bp = NULL;

        for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){

            if(!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize){

                return bp;
            }
        }

        return NULL; /* No fit */
    #elif (REPLACEMENT == NEXT_FIT)
        // need implementation
    #else /* best fit */
        // need implementation
    #endif

    return NULL;
}

/**
 * @brief print specific block's header and footer info
 */ 
static void printblock(void* bp){

    size_t hsize, halloc, fsize, falloc;

    checkheap(0);

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));  
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));  

    if (hsize == 0) {

        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp, 
           hsize, (halloc ? 'a' : 'f'), 
           fsize, (falloc ? 'a' : 'f')); 
}

/**
 * @brief check block's validity
 */ 
static void checkblock(void* bp){
    
    if((size_t)bp % 8){

        printf("Error: %p is not doubleword aligned\n", bp);
    }

    if(GET(HDRP(bp)) != GET(FTRP(bp))){

        printf("Error: header does not match footer\n");
    }    
}

static void checkheap(int verbose){

    char* bp = heap_listp;

    if(verbose){

        printf("Heap (%p):\n", heap_listp);
    }

    if(GET_SIZE(HDRP(heap_listp)) != DSIZE || !GET_ALLOC(HDRP(heap_listp))){

        printf("Bad prologue header\n");
    }

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){

        if(verbose){

            printblock(bp);
        }

        checkblock(bp);
    }

    if(verbose){

        printblock(bp);
    }

    if(GET_SIZE(HDRP(bp)) || !GET_ALLOC(HDRP(bp))){

        printf("Bad epilouge header\n");
    }
}

/*********************************************************
* this section contains major finctions:
* int mm_init(void)
* void* mm_malloc(size_t size)
* void mm_free(void* ptr)
* void* realloc(void* ptr, size_t size)
* void mm_checkheap(void)
 ********************************************************/
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){

    if((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1){

        return -1;  // return -1 if exceed the boundary of heap
    }

    /* initialize 4 blocks */
    PUT(heap_listp, 0);                             /* Alignment padding */
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));        /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        /* Epilogue header */

    /* move pointer heap_listp */
    heap_listp += (2*WSIZE);

    /* next fit pointer */
    #if (REPLACEMENT == NEXT_FIT)
         // rover = heap_listp;
    #endif

    /* each block will be at least 8 bytes */
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL){

        return -1;
    }

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){

    if(heap_listp == NULL){

        mm_init();
    }

    if(size <= 0){

        return NULL;
    }

    size_t asize;
    size_t extendsize;
    char* bp;    

    /* 8 bytes alignment */
    asize = ALIGN(size);

    /* payload size plus header and footer */
    asize += DSIZE;

    if((bp = find_fit(asize)) != NULL){

        place(bp, asize);
        return bp;
    }

    /* if no fit found, then get external memory form kernel and plus block */
    extendsize = MAX(asize, CHUNKSIZE);

    if((bp = extend_heap(extendsize/WSIZE)) != NULL){

        place(bp, asize);
        return bp;
    }

    return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* bp){

    /* error handles */
    if(bp == NULL){

        return;
    }

    if((char*)bp < (char*)mem_heap_lo() || (char*)bp >= (char*)mem_heap_hi()){

        return;
    }

    if(heap_listp == NULL){

        mm_init();
    }

    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);    
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size){

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    /* if size equal to zero then conduct mm_free() and return NULL */
    if(size == 0){

        mm_free(ptr);
        return NULL;
    }

    /* if ptr equal to NULL then this is just mm_malloc() */
    if(ptr == NULL){

        return mm_malloc(size);
    }

    /* allocate a new block */
    newptr = mm_malloc(size);

    if (newptr == NULL){
    
        return NULL;
    }

    copySize = GET_SIZE(HDRP(oldptr)); // get old block size
    
    /* if the goal of mm_realloc() is to shrink block size */
    if (size < copySize){
        
        copySize = size;
    }
    
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    
    return newptr;
}

/* 
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose){

    checkheap(verbose);
}

