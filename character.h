#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef int32_t character_t;

typedef enum tbits {BIT7, BIT8} bits_t;

void print_char(FILE *f, const character_t c, const bits_t b);
const bool is_blank_character(const character_t c);
const character_t read_char(FILE *f, const char start);
const character_t char_read(FILE *f);
