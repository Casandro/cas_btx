#pragma once
#include "terminal.h"
#include "character.h"
#include <stdio.h>


typedef struct form_field_s{
	struct form_field_s *prev;
	struct form_field_s *next;
	int row;
	int col;
	int len;
	char mode;
	char *name;
	character_t *data;
} form_field_t;


typedef struct {
	form_field_t *fields;
	terminal_t *term;
	FILE *in;
} form_t;


form_t *form_create(FILE *in, terminal_t *term);
void form_free(form_t *f);
void form_add_field(form_t *f, const int row, const int col, const int len, const char mode, const char *name, const char *val);
form_field_t *form_find_field(form_field_t *f, const char *name);
int form_handle_editing(form_t *f);
void form_read_field(form_t *f, const char *name, char *dest, size_t size);
