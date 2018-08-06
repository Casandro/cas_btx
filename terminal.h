#pragma once

#include <stdio.h>
#include "character.h"

typedef struct {
	FILE *f;
	int width;
	int height;
	character_t *cells;
	int cursor;
	bits_t bits;
} terminal_t;



terminal_t *term_create(FILE *f, const int height, const int width);
void term_free(terminal_t *t);
void term_move_cursor(terminal_t *t, const int row, const int col);
void term_set_character(terminal_t *t, const int row, const int col, const character_t c);
void term_clear_screen(terminal_t *t);
void term_clear_eol(terminal_t *t);
void term_set_string(terminal_t *t, const int row, const int col, const char *s);
void term_get_talking(terminal_t *t);
void term_set_status(terminal_t *t, const char *s);
