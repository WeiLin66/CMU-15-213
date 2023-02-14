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
    "zteam",
    /* First member's full name */
    "Hau-Wei Lin",
    /* First member's email address */
    "linhoway@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*********************************************************
* Macros for data structures
 ********************************************************/

#define IMPLICIT                                    0
#define EXPLICIT                                    1
#define SEGEGRATE                                   2
#define STRUCTURE                                   IMPLICIT

#if (STRUCTURE == EXPLICIT)
static char* exlicit_free_list_head = NULL;
#endif
/*********************************************************
* Macros for replacement policies
 ********************************************************/

#define FIRST_FIT                                   0
#define NEXT_FIT                                    1
#define BEST_FIT                                    2
#define REPLACEMENT                                 BEST_FIT

/*********************************************************
* Macros for block operation
 ********************************************************/

#define WSIZE                                       4
#define DSIZE                                       8
#define CHUNKSIZE                                   (1 << 12)

#define MAX(x, y)                                   ((x) > (y) ? (x) : (y))

/* create header or footer message */
#define PACK(size, alloc)                           ((size) | (alloc))

/* read & write a word at address p */
#define GET(p)                                      (*(unsigned int*)(p))
#define PUT(p, val)                                 (*(unsigned int*)(p) = (val))

/* get size and allocated bit of a block */
#define GET_SIZE(p)                                 (GET(p) & ~0x7)
#define GET_ALLOC(p)                                (GET(p) & 0x1)

/* get header and footer from data block pointer */
#define HDRP(bp)                                    ((char *)(bp) - WSIZE)
#define FTRP(bp)                                    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* get previous and next data block by adding or subtracting block size */
#define NEXT_BLKP(bp)                               ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) 
#define PREV_BLKP(bp)                               ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* get previous block's footer */
#define PREV_FTRP(bp)                               ((char *)(bp) - DSIZE)

/*********************************************************
* Macros for getting and setting explicit block's pointers
 ********************************************************/

#if (STRUCTURE == EXPLICIT)
#define GET_NEXT_FREE_BLKP(bp)                          (*(char **)(bp))
#define GET_PREV_FREE_BLKP(bp)                          (*((char **)(bp) + 1))
#define PUT_NEXT_FREE_BLKP(bp, val)                     (*(unsigned int *)(bp) = (unsigned int)(val))
#define PUT_PREV_FREE_BLKP(bp, val)                     (*((unsigned int *)(bp) + 1) = (unsigned int)(val))
#endif

/*********************************************************
* Macros for alignment operations
 ********************************************************/

/* single word (4) or double word (8) alignment */
#define ALIGNMENT               DSIZE

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size)             (((size) + (ALIGNMENT-1)) & ~0x7)

/* alignment size define */
#define SIZE_T_SIZE             (ALIGN(sizeof(size_t)))

/* explicit list contains two extra pointers */
#if (STRUCTURE == EXPLICIT)
#define  EXTRA_LENGTH           DSIZE
#else
#define  EXTRA_LENGTH           0
#endif

/* minimun alignment block size */
#define MINIMUN_BLOCK           DSIZE + SIZE_T_SIZE + EXTRA_LENGTH

/*********************************************************
* Macros for debugging message
 ********************************************************/

#define SHOW_WARNING()      printf("[Warning] [File: %s] [Func: %s] [Line: %u]\n", __FILE__, __FUNCTION__, __LINE__)

#define BLOCK_DETAIL(bp)    printblock((bp))

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
__inline static int checkheap_boundary(void* bp);
#if (STRUCTURE == EXPLICIT)
static void insert(void* bp);
static void remove(void* bp);
static void checklist(int verbose);
#endif

/**
 * @brief extend heap with free block and return its block pointer 
 * note that one word represent 4 bytes
 * 
 * @param words how many words does the kernel should extend the heap
 */ 
static void* extend_heap(size_t words){

    char* bp;
    ssize_t size;
 
    /* since the block is 8 bytes alignment, we need to make sure that
     * the space we intend to enlarge is 2*words   
     */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;

    if((long)(bp = (mem_sbrk(size))) == -1){

        SHOW_WARNING();
        return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0));           // header
    PUT(FTRP(bp), PACK(size, 0));           // footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   // Epilogue block

#if (STRCTURE == EXPLICIT)
    PUT_NEXT_FREE_BLKP(bp, 0);
    PUT_PREV_FREE_BLKP(bp, 0);
#endif

    return coalesce(bp);    // the end of the previous block could be free
}

/** 
 * @brief place - Place block of asize bytes at start of free block bp 
 * and split if remainder would be at least minimum block size
 * 
 * @param bp data block pointer
 * @param asize asize of block to be allocated
 */
static void place(void* bp, size_t asize){

    if(bp == NULL || asize <= 0){

        SHOW_WARNING();
        return;
    }

    size_t size = GET_SIZE(HDRP(bp));

    /* split */
    if((size - asize) >= MINIMUN_BLOCK){

        /* current block */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

#if (STRUCTURE == EXPLICIT)
        remove(bp);
#endif

        bp = NEXT_BLKP(bp);

        /* new block */
        PUT(HDRP(bp), PACK((size-asize), 0));
        PUT(FTRP(bp), PACK((size-asize), 0));

#if (STRUCTURE == EXPLICIT)
        insert(bp);
#endif
    }
    /* no need for spliting */
    else{

        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));

#if (STRUCTURE == EXPLICIT)
        remove(bp);
#endif
    }
}

/**
 * @brief find a free block that meets the asize requirment
 * 
 * @param asize asize of block that is intended to find
 */ 
static void* find_fit(size_t asize){

#if (REPLACEMENT == FIRST_FIT)
    char* bp = heap_listp;

#if (STRUCTURE == IMPLICIT)
    for(; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
#elif (STRUCTURE == EXPLICIT)
    bp = exlicit_free_list_head;
    for(; bp != NULL; bp = GET_NEXT_FREE_BLKP(bp)){
#endif
        if(!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize){

            return bp;
        }
    }

    return NULL; /* No fit found */
#elif (REPLACEMENT == NEXT_FIT)
    char *oldrover = rover;

#if (STRUCTURE == IMPLICIT)
    /* Search from the rover to the end of list */
    for (; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover)){
#elif (STRUCTURE == EXPLICIT)
    for(; rover != NULL; rover = GET_NEXT_FREE_BLKP(rover)){
#endif
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover)))){
            
            return rover;
        }
    }

#if (STRUCTURE == IMPLICIT)
    /* search from start of list to old rover */
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover)){
#elif (STRUCTURE == EXPLICIT)
    for(rover = exlicit_free_list_head; rover != NULL; rover = GET_NEXT_FREE_BLKP(rover)){
#endif
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover)))){
        
            return rover;
        }
    }

    return NULL;  /* no fit found */
#else /* best fit */
    char* bp = heap_listp;
    char* minibp = NULL;

#if (STRUCTURE == IMPLICIT)
    for(; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
#elif (STRUCTURE == EXPLICIT)
    bp = exlicit_free_list_head;
    for(; bp != NULL; bp = GET_NEXT_FREE_BLKP(bp)){
#endif
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            
            if(minibp == NULL){

                minibp = bp;
            }else{

                minibp = (GET_SIZE(HDRP(bp)) < GET_SIZE(HDRP(minibp))) ? bp : minibp;
            }
        }
    }

    return minibp;
#endif

    SHOW_WARNING();
    return NULL;
}

/**
 * @brief merge blocks when conducts free operation
 * 
 * @param bp data block pointer
 */ 
static void* coalesce(void* bp){

    if(bp == NULL){

        SHOW_WARNING();
        return bp;
    }

    size_t prev_alloc = GET_ALLOC(PREV_FTRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* case 1: none of the previos and next blocks aren free */
    if(prev_alloc && next_alloc){
        
        // no need for coalescing
    }
    /* case 2: previous block is free */
    else if(!prev_alloc && next_alloc){

#if (STRUCTURE == EXPLICIT)
        remove(PREV_BLKP(bp));
#endif          
        size += GET_SIZE(PREV_FTRP(bp));

        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /* case 3: next block is free */
    else if(prev_alloc && !next_alloc){

#if (STRUCTURE == EXPLICIT)
        remove(PREV_BLKP(NEXT_BLKP(bp)));
#endif           
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));

        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(bp), PACK(size, 0));        
    }
    /* case 4: both previous and next block are free */
    else if(!prev_alloc && !next_alloc){

#if (STRUCTURE == EXPLICIT)
        remove(PREV_BLKP(bp));
        remove(PREV_BLKP(NEXT_BLKP(bp)));
#endif
        size += GET_SIZE(PREV_FTRP(bp)) + GET_SIZE(HDRP(NEXT_BLKP(bp)));

        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

#if (REPLACEMENT == NEXT_FIT)
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp))){
    
        rover = bp;
    }
#endif

#if (STRUCTURE == EXPLICIT)
    insert(bp); // insert block to free list
#endif  

    return bp;
}

/**
 * @brief print specific block's header and footer info
 * 
 * @param bp data block pointer
 */ 
static void printblock(void* bp){

    if(bp == NULL){

        SHOW_WARNING();
        return;
    }

    size_t hsize, halloc, fsize, falloc;

    checkheap(0);

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));  
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));  

    if (hsize == 0) {

        SHOW_WARNING();
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp, 
           hsize, (halloc ? 'a' : 'f'), 
           fsize, (falloc ? 'a' : 'f')); 
}

/**
 * @brief checks each block's correctness on the heap
 * 
 * @param verbose a flag to show extra info of block
 */
static void checkheap(int verbose){

    char* bp = heap_listp;

    if(verbose){

        printf("Heap (%p):\n", heap_listp);
    }

    if(GET_SIZE(HDRP(heap_listp)) != DSIZE || !GET_ALLOC(HDRP(heap_listp))){

        SHOW_WARNING();
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

        SHOW_WARNING();
        printf("Bad epilouge header\n");
    }
}

/**
 * @brief check block's validity
 */ 
static void checkblock(void* bp){

    if(bp == NULL){

        SHOW_WARNING();
        return;
    }
    
    if((size_t)bp % 8){

        SHOW_WARNING();
        printf("Error: %p is not doubleword aligned\n", bp);
    }

    if(GET(HDRP(bp)) != GET(FTRP(bp))){

        SHOW_WARNING();
        printf("Error: header does not match footer\n");
    }    
}

/**
 * @brief function for checking if the pointer is valid
 * 
 * @param bp data block pointer
 * @return 0 if pointer bp is within the heap, -1 if it's not
 */
__inline static int checkheap_boundary(void* bp){

    return (bp < mem_heap_lo()) ? -1 : 
    ((char*)bp > (char*)mem_heap_hi() - 3 - SIZE_T_SIZE + WSIZE) ? -1 : 0;
}

#if (STRUCTURE == EXPLICIT)
/**
 * @brief insert block at the top of the free list (LIFO)
 * 
 * @param bp data block address
 */
static void insert(void* bp){

    if(bp == NULL){

        SHOW_WARNING();
        return;
    }

    /* LIFO */
    if(exlicit_free_list_head != NULL){

        /* adjust next block pointers */
        PUT_PREV_FREE_BLKP(exlicit_free_list_head, bp);  // NULL --> bp

        /* adjust current block */
        PUT_PREV_FREE_BLKP(bp, 0);  // NULL
        PUT_NEXT_FREE_BLKP(bp, exlicit_free_list_head);
    }

    exlicit_free_list_head = bp;
}

/**
 * @brief remove block from the free list
 * 
 * @param bp data block address
 */
static void remove(void* bp){

    if(bp == NULL){

        SHOW_WARNING();
        return;
    }

    if(exlicit_free_list_head == NULL){

        SHOW_WARNING();
        return;
    }

    /* LIFO */
    unsigned int* old_bp = exlicit_free_list_head;
    unsigned int* next_bp = GET_NEXT_FREE_BLKP(exlicit_free_list_head);

    PUT_PREV_FREE_BLKP(next_bp, 0);
    exlicit_free_list_head = next_bp;

    PUT_NEXT_FREE_BLKP(old_bp, 0);
}

/**
 * @brief check free list's correctness
 * 
 * @param verbose a flag to show extra info of block
 */ 
static void checklist(int verbose){

    if(bp == NULL){

        SHOW_WARNING();
        return;
    }

    char* bp = exlicit_free_list_head;

    for(; bp != NULL; bp = GET_NEXT_FREE_BLKP(bp)){

        if(verbose){

            printblock(bp);
        }

        checkblock(bp);        
    }
}
#endif

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

        SHOW_WARNING();
        return -1;
    }

    /* initialize 4 blocks */
    PUT(heap_listp, PACK(0, 0));                    /* Alignment padding */
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));        /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        /* Epilogue header */

    /* move pointer heap_listp */
    heap_listp += (2*WSIZE);

    /* next fit pointer */
#if (REPLACEMENT == NEXT_FIT)
    rover = heap_listp;
#endif

    /* each block will be at least 8 bytes */
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL){

        SHOW_WARNING();
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

        SHOW_WARNING();
        mm_init();
    }

    if(size <= 0){

        SHOW_WARNING();
        return NULL;
    }

    size_t asize;
    size_t extendsize;
    char* bp;    

    /* 8 bytes alignment */
    asize = ALIGN(size);

    /* payload size plus header and footer */
    asize += DSIZE;

    /* plus next and previous pointers */
    asize += EXTRA_LENGTH;

    /* check alignment */
    if(asize < size || asize % SIZE_T_SIZE != 0){

        SHOW_WARNING();
        return NULL;
    }

    /* find qualified free block */
    if((bp = find_fit(asize)) != NULL){

        place(bp, asize);
        return bp + EXTRA_LENGTH;
    }

    /* if no fit found, then get external memory form kernel and extend heap */
    extendsize = MAX(asize, CHUNKSIZE);

    if((bp = extend_heap(extendsize/WSIZE)) != NULL){

        place(bp, asize);
        return bp + EXTRA_LENGTH;
    }

    SHOW_WARNING();
    return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* bp){

    if(bp == NULL){

        SHOW_WARNING();
        return;
    }

    if(heap_listp == NULL){

        SHOW_WARNING();
        mm_init();
    }

    bp = (char*)bp - EXTRA_LENGTH;

    size_t size = GET_SIZE(HDRP(bp));
    size_t alloc = GET_ALLOC(HDRP(bp));

    /* block must be previously allocated and within the range of heap */
    if(alloc == 0 || checkheap_boundary(bp)){

        SHOW_WARNING();
        return;
    }

    /* adjust header and footer allocated bit */
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    /* coalescing blocks(if any) */
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
    
        SHOW_WARNING();
        return NULL;
    }

    copySize = GET_SIZE(HDRP(oldptr)); // get old block size

    copySize = (copySize - DSIZE - EXTRA_LENGTH > size) ? size : copySize - DSIZE - EXTRA_LENGTH;
    
    /* copy content to the new block */
    memcpy(newptr, oldptr, copySize);

    /* free the old block */
    mm_free(oldptr);
    
    return newptr;
}

/* 
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose){

    checkheap(verbose);
}

