#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cachelab.h"


typedef struct{

    uint32_t vaild;
    uint32_t tag;
    uint64_t timeStamp;
}cacheLine;


static cacheLine** virtual_cache = NULL;
static FILE* fp;
static uint8_t file_read_Buffer[1024];

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

    virtual_cache = (cacheLine**)malloc(sizeof(cacheLine*)*s);

    for(int i=0; i<s; i++){

        virtual_cache[i] = (cacheLine*)malloc(sizeof(cacheLine)*E);
    }
}

int main(int argc, char* argv[]){

    uint8_t s=0, E=0, b=0;

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
                fp = fopen(argv[++i], "w+");
                size_t ret = fread(file_read_Buffer, 1, 1, fp);
                fclose(fp);
                printf("read file result: %lu\n", ret);
            break;

            default:
            break;
        }
    }

    printf("s = %u, E = %u, b = %u\n", s, E, b);
    printf("trace file read: %ld\n", strlen((char*)file_read_Buffer));

    cache_init(s, E);
    // printSummary(0, 0, 0);

    return 0;
}
