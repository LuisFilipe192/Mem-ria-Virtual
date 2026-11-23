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

#define BACKING_STORE "BACKING_STORE.bin"
#define OUTPUT_FILE "correct.txt"

int main(int argc, char *argv[]){
    (void)argc;
    (void)argv;
    return 0;
}