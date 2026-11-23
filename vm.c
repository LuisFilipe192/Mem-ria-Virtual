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

#define BACKING_STORE "BACKING_STORE.bin"
#define OUTPUT_FILE "correct.txt"

#define LINE_MAX_LEN 256

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

static void strip_newline(char *s){
    size_t len = strlen(s);

    while(len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')){
        s[--len] = '\0';
    }
}

static int is_blank(const char *s){
    for(; *s; s++){
        if(!isspace((unsigned char)*s)){
            return 0;
        }
    }
    return 1;
}

static int parse_address(const char *s, int *out){
    if(*s == '\0'){
        return 0;
    }

    for(const char *p = s; *p; p++){
        if(!isdigit((unsigned char)*p)){
            return 0;
        }
    }

    if(strlen(s) > 5){
        return 0;
    }

    long v = strtol(s, NULL, 10);

    if(v > 65535){
        return 0;
    }

    *out = (int)v;
    return 1;
}

static int *addresses = NULL;
static int n_addresses = 0;

static void load_addresses(const char *path){
    FILE *f = fopen(path, "r");
    if(f == NULL){
        fprintf(stderr, "Error: could not open address file '%s'\n", path);
        exit(EXIT_FAILURE);
    }

    char line[LINE_MAX_LEN];
    int lineno = 0;
    int value;

    while(fgets(line, sizeof(line), f) != NULL){
        lineno++;
        strip_newline(line);

        if(is_blank(line)){
            continue;
        }
        if(!parse_address(line, &value)){
            fprintf(stderr, "Error: address file line %d is invalid "
                "(expected one integer between 0 and 65535)\n", lineno);
            fclose(f);
            exit(EXIT_FAILURE);
        }

        n_addresses++;
    }

    if(n_addresses == 0){
        fclose(f);
        die("address file has no addresses");
    }

    addresses = malloc(n_addresses * sizeof(int));
    if(addresses == NULL){
        fclose(f);
        die("out of memory");
    }

    rewind(f);
    int i = 0;

    while(fgets(line, sizeof(line), f) != NULL){
        strip_newline(line);

        if(is_blank(line)){
            continue;
        }

        parse_address(line, &addresses[i]);
        i++;
    }
    fclose(f);
}

static FILE *backing = NULL;

static void open_backing_store(void){
    backing = fopen(BACKING_STORE, "rb");
    if(backing == NULL){
        die("could not open " BACKING_STORE);
    }

    fseek(backing, 0, SEEK_END);
    long size = ftell(backing);

    if(size != NUM_PAGES * PAGE_SIZE){
        fclose(backing);
        die(BACKING_STORE " must have 65536 bytes");
    }
    rewind(backing);
}

static signed char memory[NUM_PAGES][FRAME_SIZE];
static int page_table[NUM_PAGES];
static int next_frame = 0;
static int page_faults = 0;
static int tlb_hits = 0;

static void init_page_table(void){
    for(int i = 0; i < NUM_PAGES; i++){
        page_table[i] = -1;
    }
}

static void load_page(int page, int frame){
    fseek(backing, (long)page * PAGE_SIZE, SEEK_SET);

    if(fread(memory[frame], 1, PAGE_SIZE, backing) != PAGE_SIZE){
        die("could not read page from " BACKING_STORE);
    }
}

static int get_frame(int page){
    if(page_table[page] != -1){
        return page_table[page];
    }

    page_faults++;
    int frame = next_frame;
    next_frame++;

    load_page(page, frame);
    page_table[page] = frame;
    return frame;
}

static void run(void){
    FILE *out = fopen(OUTPUT_FILE, "w");
    if(out == NULL){
        die("could not create " OUTPUT_FILE);
    }

    for(int i=0;i<n_addresses;i++){
        int virtual_address = addresses[i];
        int page = (virtual_address >> 8) & 0xFF;
        int offset = virtual_address & 0xFF;
        int tlb_index = -1;

        int frame = get_frame(page);
        int physical_address = frame * FRAME_SIZE + offset;
        int value = memory[frame][offset];

        fprintf(out, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
            virtual_address, tlb_index, physical_address, value);
    }

    fprintf(out, "Number of Translated Addresses = %d\n", n_addresses);
    fprintf(out, "Page Faults = %d\n", page_faults);
    fprintf(out, "Page Fault Rate = %.3f\n", (double)page_faults / n_addresses);
    fprintf(out, "TLB Hits = %d\n", tlb_hits);
    fprintf(out, "TLB Hit Rate = %.3f\n", (double)tlb_hits / n_addresses);
    fclose(out);
}

int main(int argc, char *argv[]){
    parse_args(argc, argv);
    load_addresses(argv[1]);
    open_backing_store();
    init_page_table();
    run();
    fclose(backing);
    free(addresses);
    return 0;
}