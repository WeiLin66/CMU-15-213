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
#define WSIZE                                      4
#define DSIZE                                      8    // double size
#define CHUNKSIZE                                  (1 << 12)   // extended heap size

#define MAX(x, y)                                  ((x) > (y) ? (x) : (y))

/* create header or footer message */
#define PACK(size,  alloc)                         ((size) | (alloc))

/* read & write header or footer message */
#define GET(p)                                     (*((u_int64_t*)p))  // read 64 bits unsigned info form header
#define PUT(val)                                   (*((u_int64_t*)p) = (val) )   // write 64 bits info to header

/* get block size and allocated condition */
#define GET_SIZE(p)                                (GET(p) & ~0x7)
#define GET_ALLOC(p)                               (GET(p) & 0x1)

/* get header and footer */
#define HDRP(bp)                                    (u_int64_t*)(((u_int64_t*)bp) - DSIZE)
#define FTRP(bp)                                    (u_int64_t*)(((uint8_t*)bp + GET_SIZE(HDRP(bp))) - (2 * DSIZE))

/* get previous and next block by caculating current block point */
#define NEXT_BLKP(bp)                               (void*)((uint8_t*)bp + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)                               (void*)((uint8_t*)bp - (2 * DSIZE))
/*********************************************************
* Macros for replacement policies
 ********************************************************/
#define NEXT_FITx

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
    return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














