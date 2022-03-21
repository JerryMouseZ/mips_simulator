#include "parser.h"
#include <bits/pthreadtypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

extern uint8_t *memory;
extern uint32_t program_break;
extern uint32_t registers[];
extern uint32_t pc;
extern uint32_t rhi;
extern uint32_t rlo;

void readbinary(const char *filename)
{
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        fprintf(stderr, "machine code file can't be open\n");
        exit(-1);
    }
    
    char buffer[35];
    int len = 0;

    while (fgets(buffer, 35, input) != NULL) {
        uint32_t *data = (uint32_t *)(memory + len);
        *data = strtol(buffer, NULL, 2);
        len += 4;
    }

    fclose(input);
}

void load_data(const char *filename) 
{
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        fprintf(stderr, "error on opening %s\n", filename);
        exit(-1);
    }

    // read data
    char buffer[128];
    int pos = 0x100000;
    bool begin = true;
    while(fgets(buffer, 128, input) != NULL) {
        if (strstr(buffer, ".data") != NULL) {
            begin = true;
            continue;
        }
        if (strstr(buffer, ".text") != NULL) {
            begin = false;
            continue;
        }

        if (begin) {
            char *type = strstr(buffer, ":");
            char *value = NULL;
            if (type != NULL) {
                if (strstr(type, ".ascii") != NULL) {
                    value = strstr(type, "\"") + 1;
                    int len = 0;
                    while (*value != '\"') {
                        if (*value != '\\') {
                            memory[pos + len] = *value;
                        } else {
                            value++;
                            if (*value == 'n') {
                                memory[pos + len] = '\n';
                            } else if (*value == 't') {
                                memory[pos + len] = '\t';
                            }
                        }
                        len++;
                        value++;
                    }
                    /* len = len - 1; */
                    /* memcpy(memory + pos, value, len); */
                    int occupy_len;
                    if (strstr(type, ".asciiz") != NULL) {
                        occupy_len = (len + 4) / 4 * 4;
                    } else {
                        occupy_len = (len + 3) / 4 * 4; // end as \0, and fill up to 4 byte
                    }
                    memset(memory + pos + len, 0, occupy_len - len);
                    pos += occupy_len;
                }
                else if (strstr(type, ".word") != NULL) {
                    value = strstr(type, ".word") + 6;
                    char *token = strtok(value, ",");
                    while (token != NULL) {
                        *(uint32_t *)(memory + pos) = atoi(token);
                        pos += 4;
                        token = strtok(NULL, ",");
                    }
                } else if (strstr(type, ".half") != NULL) {
                    value = strstr(type, ".half") + 6;
                    char *token = strtok(value, ",");
                    while (token != NULL) {
                        *(uint16_t *)(memory + pos) = atoi(token);
                        pos += 2;
                        token = strtok(NULL, ",");
                    }
                    if (pos % 4) {
                        memset(memory + pos, 0, 4 - pos % 4);
                    }
                } else if (strstr(type, ".byte") != NULL) {
                    value = strstr(type, ".byte") + 6;
                    char *token = strtok(value, ",");
                    while (token != NULL) {
                        *(uint8_t *)(memory + pos) = atoi(token);
                        pos += 1;
                        token = strtok(NULL, ",");
                    }
                    if (pos % 4) {
                        memset(memory + pos, 0, 4 - pos % 4);
                    }
                } else {
                    fprintf(stderr, "error type reading %s\n", type);
                    exit(-1);
                }
            }
        }
    }

    program_break += 0x400000 + pos;
    fclose(input);
}

int checkpoints[100] = {0};
int points_counter = 0;

void init_checkpoints(const char *checkpoint_file)
{
    FILE *fp = fopen(checkpoint_file, "r");
    int tmp;
    while (fscanf(fp, "%d", &tmp) != EOF) {
        checkpoints[points_counter++] = tmp;
    }
    fclose(fp);
}

void checkpoint_memory(int ins_count)
{
    bool flag = false;
    for (int i = 0; i < points_counter; ++i) {
        if (checkpoints[i] == ins_count) {
            flag = true;
            break;
        }
    }
    if (!flag) {
        return;
    }
    char filename[64];
    sprintf(filename, "memory_%d.bin", ins_count);
    FILE *fp = fopen(filename, "wb");
    fwrite(memory, 1, 0x600000, fp);
    fclose(fp);
}

void checkpoint_register(int ins_count)
{
    bool flag = false;
    for (int i = 0; i < points_counter; ++i) {
        if (checkpoints[i] == ins_count) {
            flag = true;
            break;
        }
    }
    if (!flag) {
        return;
    }
    char filename[64];
    sprintf(filename, "register_%d.bin", ins_count);
    FILE *fp = fopen(filename, "wb");
    for (int i = 0; i < 32; ++i) {
        fwrite(registers + i, 4, 1, fp);
    }
    fwrite(&pc, 4, 1, fp);
    fwrite(&rhi, 4, 1, fp);
    fwrite(&rlo, 4, 1, fp);

    fclose(fp);
}
