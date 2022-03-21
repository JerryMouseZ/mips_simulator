#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void readbinary(const char *filename);

void load_data(const char *filename);

void init_checkpoints(const char *checkpoint_file);

void checkpoint_memory(int ins_count);

void checkpoint_register(int ins_count);

#endif
