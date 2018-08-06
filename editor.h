#pragma once

#include <stdio.h>
#include "character.h"
#include "terminal.h"

#define EDITOR_WIDTH (40)
#define EDITOR_HEIGHT (22)
#define EDITOR_TOP (1)

#define EDITOR_SIZE (EDITOR_WIDTH*EDITOR_HEIGHT)

typedef struct {
	character_t cells[EDITOR_SIZE];
} editor_doc_t;


typedef struct {
	char *filename;
	editor_doc_t doc;
	terminal_t *term;
	FILE *input;
	int cursor;
} editor_t;

void editor_free(editor_t *e);
int editor_edit_file(FILE *in, terminal_t *term, char *fn);
editor_t *editor_create(FILE *in, terminal_t *t, char *fn);

int editor_edit(FILE *in, terminal_t *term, const char *dir);

