#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "instruction.h"
#include "parser.h"
#include <unistd.h>

uint32_t registers[32] = {0};
uint32_t pc = 0x400000;

uint8_t *memory = NULL;

void init_registers()
{
    // what is $gp to init
    registers[28] = 0x508000;
    // init sp to 0xa00000
    registers[29] = 0xa00000;
    registers[30] = 0xa00000;
}

// ./simulator test.asm test.txt checkpoints.txt test.in test.out
int main(int argc, char **argv)
{
    if (argc != 6) {
        fprintf(stderr, "Usage:\t ./simulator test.asm test.txt test_checkpoints.txt test.in test.out\n");
        exit(-1);
    }
    // allocate 6M for memory
    memory = mmap(0, 0x600000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    init_registers();
    readbinary(argv[2]);
    load_data(argv[1]);
    init_checkpoints(argv[3]);
    int infd = open(argv[4], O_RDONLY);
    if (infd < 0) {
        fprintf(stderr, "error on opening %s\n", argv[4]);
        exit(-1);
    }
    int outfd = open(argv[5], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (outfd < 0) {
        fprintf(stderr, "error on opening %s\n", argv[5]);
        exit(-1);
    }
    dup2(infd, 0); // redirect stdin
    dup2(outfd, 1); // redirect stdout
    execute();
    close(infd);
    close(outfd);
}
