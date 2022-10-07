#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "cachelab.h"


typedef struct{

    uint32_t vaild;
    uint32_t tag;
    uint64_t timeStamp; // for LRU
}cacheLine;

#define BUFFER_LENGTH   1024
#define OP_INFO         1

static cacheLine** virtual_cache = NULL;
static FILE* fp;
static uint8_t file_read_Buffer[BUFFER_LENGTH]={0};
static uint8_t s=0, E=0, b=0;
static int miss=0, hits=0, evictions=0;
static uint64_t ticks=0;

/**
 * 
 */ 
static char get_operation(char* str){

    if(strlen(str) <= 1){
        return '\0';
    }

    return str[1];
}


/**
 * 
 */ 
static void load_operation(uint32_t index){

    uint32_t addr=0;
    uint32_t dataBytes=0;
    char op;

    sscanf(file_read_Buffer+index, "%c %d, %d", &op, &addr, &dataBytes);

    uint32_t offset = addr & (~(0xff << b));
    uint32_t set = (addr >> b) & (~(0xff << s));
    uint32_t tag = addr >> (s+b);

#if OP_INFO
    printf("tag = 0x%X, set = 0x%X, offset = 0x%X\n", tag, set, offset);

    bool find = false;

    for(int i=0; i<E; i++){

        if(virtual_cache[set][i].vaild && virtual_cache[set][i].tag == tag){

            find = true;
            break;
        }
    }

    if(find){

        printf("op = %c, addr = %u, bytes = %u hits!\n", op, addr, dataBytes);
    }else{

        printf("op = %c, addr = %u, bytes = %u miss!\n", op, addr, dataBytes);
    }

#endif

}


/**
 * 
 */ 
static void modify_operation(uint32_t index){

    uint32_t addr=0;
    uint32_t dataBytes=0;
    char op;

    sscanf(file_read_Buffer+index, "%c %d, %d", &op, &addr, &dataBytes);

    uint32_t offset = addr & (~(0xff << b));
    uint32_t set = (addr >> b) & (~(0xff << s));
    uint32_t tag = addr >> (s+b);

#if OP_INFO
    printf("tag = 0x%X, set = 0x%X, offset = 0x%X\n", tag, set, offset);

    bool find = false;

    for(int i=0; i<E; i++){

        if(virtual_cache[set][i].vaild && virtual_cache[set][i].tag == tag){

            find = true;
            break;
        }
    }

    if(find){

        printf("op = %c, addr = %u, bytes = %u hits!\n", op, addr, dataBytes);
    }else{

        printf("op = %c, addr = %u, bytes = %u miss!\n", op, addr, dataBytes);
    }

#endif

}


/**
 * 
 */ 
static void save_operation(uint32_t index){

    uint32_t addr=0;
    uint32_t dataBytes=0;
    char op;

    sscanf(file_read_Buffer+index, "%c %d, %d", &op, &addr, &dataBytes);

    uint32_t offset = addr & (~(0xff << b));
    uint32_t set = (addr >> b) & (~(0xff << s));
    uint32_t tag = addr >> (s+b);

#if OP_INFO
    printf("tag = 0x%X, set = 0x%X, offset = 0x%X\n", tag, set, offset);

    bool find = false;

    for(int i=0; i<E; i++){

        if(virtual_cache[set][i].vaild && virtual_cache[set][i].tag == tag){

            find = true;
            break;
        }
    }

    if(find){

        printf("op = %c, addr = %u, bytes = %u hits!\n", op, addr, dataBytes);
    }else{

        printf("op = %c, addr = %u, bytes = %u miss!\n", op, addr, dataBytes);
    }

#endif

}


/**
 * 
 */ 
static void cmd_parsing(){

    assert(file_read_Buffer);
    uint32_t index=0;

    while(file_read_Buffer[index]){

        switch(file_read_Buffer[index]){

            case 'L':
                load_operation(index);
            break;

            case 'M':
                modify_operation(index);
            break;

            case 'S':
                save_operation(index);
            break;
        }

        index++;
    }
    
}


/**
 * Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>
 * Options:
 *   -h         Print this help message.
 *   -v         Optional verbose flag.
 *   -s <num>   Number of set index bits.
 *   -E <num>   Number of lines per set.
 *   -b <num>   Number of block offset bits.
 *   -t <file>  Trace file.*
 *
 * Examples:
 *   linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace
 *   linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace
 */ 
static void print_help_message(){

    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");

    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");

    printf("Examples:\n");
    printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}


/**
 * 
 */ 
static void cache_init(uint8_t s, uint8_t E){

    uint32_t sets = 2 << s;

    virtual_cache = (cacheLine**)malloc(sizeof(cacheLine*) * sets);

    for(int i=0; i<sets; i++){

        virtual_cache[i] = (cacheLine*)malloc(sizeof(cacheLine)*E);
    }
}

int main(int argc, char* argv[]){


    for(int i=1; i<argc; i++){

        char op = get_operation(argv[i]);

        switch(op){

            case 'h':
                print_help_message();
            break;

            case 'v':
                printf("print info for trace files...\n");
            break;

            case 's':
                s = atoi(argv[++i]);
            break;

            case 'E':
                E = atoi(argv[++i]);
            break;

            case 'b':
                b = atoi(argv[++i]);
            break;

            case 't':
                fp = fopen(argv[++i], "r");
                assert(fp);
                fread(file_read_Buffer, sizeof(char), BUFFER_LENGTH, fp);
                fclose(fp);
            break;

            default:
            break;
        }
    }

    cache_init(s, E);
    cmd_parsing();
    // printSummary(0, 0, 0);

    return 0;
}
