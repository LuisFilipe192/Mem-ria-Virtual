#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define PAGE_SIZE 256
#define NUM_PAGES 256
#define FRAME_SIZE 256
#define NUM_FRAMES 128
#define TLB_SIZE 16

#define ALG_FIFO 0
#define ALG_LRU 1

static int page_alg = ALG_FIFO;
static int tlb_alg = ALG_FIFO;

static void die(const char *msg){
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

static int parse_alg(const char *s){
    if(strcmp(s, "fifo") == 0){
        return ALG_FIFO;
    }
    if(strcmp(s, "lru") == 0){
        return ALG_LRU;
    }
    return -1;
}

static void parse_args(int argc, char *argv[]){
    if(argc != 4){
        die("usage: ./vm <address file> <page replacement: fifo|lru> <tlb replacement: fifo|lru>");
    }

    page_alg = parse_alg(argv[2]);
    if(page_alg == -1){
        die("page replacement algorithm must be fifo or lru");
    }

    tlb_alg = parse_alg(argv[3]);
    if(tlb_alg == -1){
        die("tlb replacement algorithm must be fifo or lru");
    }
}

#define BACKING_STORE "BACKING_STORE.bin"
#define OUTPUT_FILE "correct.txt"

int main(int argc, char *argv[]){
    parse_args(argc, argv);
    return 0;
}