#include "terminal.h"
#include <string.h>
#include <stdlib.h>

terminal_t *term_create(FILE *f, const int height, const int width)
{
	if ((height==0) && (width==0)) return term_create(f, 24, 40);
	if (height<=0) return NULL;
	if (width<=0) return NULL;
	if (height>64) return NULL;
	if (width>64) return NULL;
	size_t size=sizeof(character_t)*height*width;
	terminal_t *t=malloc(sizeof(terminal_t));
	if (t==NULL) return NULL;
	memset(t, 0, sizeof(terminal_t));
	t->cells=malloc(size);
	if (t->cells==NULL) {
		free(t);
		return NULL;
	}
	memset(t->cells, 0, size);
	t->width=width;
	t->height=height;
	t->cursor=0;
	t->f=f;
	t->bits=BIT8;
	if (f!=NULL) fprintf(f,"\x1b\x22\x40");
	return t;
}

void term_free(terminal_t *t)
{
	if (t==NULL) return;
	if (t->cells!=NULL) free(t->cells);
	free(t);
}

int term_get_pos(terminal_t *t, const int row, const int col)
{	
	if (t==NULL) return -1;
	if (row<0) return -1;
	if (col<0) return -1;
	if (row>=t->height) return -1;
	if (col>=t->width) return -1;
	return row*t->width+col;
}


void term_move_cursor(terminal_t *t, const int row, const int col)
{
	if (t==NULL) return;
	int p=term_get_pos(t, row, col);
	if (p<0) return;
	
	int w=t->width;
	int h=t->height;
	int s=w*h;

	int diff=p-t->cursor;

	if ( diff==0 ) return; //Nothing to do
	if ( p==0) fprintf(t->f, "\x1e"); else //active Position home
	if ( diff==-1 ) fprintf(t->f, "\x08"); else //Active position left
	if ( diff== 1 ) fprintf(t->f, "\x09"); else //Active position right
	if ( diff== w ) fprintf(t->f, "\x0a"); else //Active position down
	if ( diff==-w ) fprintf(t->f, "\x0b"); else //Active position up
	{
		fprintf(t->f, "\x1f%c%c", 0x41+row, 0x41+col);
	}
	t->cursor=p;	
}

void term_inc_cursor(terminal_t *t)
{
	if (t==NULL) return;
	int s=t->width*t->height;
	t->cursor=(t->cursor+1)%s;
}


character_t term_get_char(terminal_t *t, const int row, const int col)
{

	int p=term_get_pos(t, row, col);
	if (p<0) return -1;
	if (t->cells==NULL) return -1;
	return t->cells[p];
}


void term_print_char(terminal_t *t, character_t c)
{
	if (t==NULL) return;
	print_char(t->f, c, t->bits);
	t->cells[t->cursor]=c;
	term_inc_cursor(t);
}

void term_set_character(terminal_t *t, const int row, const int col, const character_t c)
{
	int p=term_get_pos(t, row, col);
	if (p<0) return;
	if (t==NULL) return; 
	term_move_cursor(t, row, col);
	term_print_char(t, c);
}

void term_clear_screen(terminal_t *t)
{
	if (t==NULL) return;
	int s=t->width*t->height;
	int n;
	for (n=0; n<s; n++) t->cells[n]=32;
	fprintf(t->f, "\x0c"); //Clear screen
}

void term_clear_eol(terminal_t *t)
{
	if (t==NULL) return;
	int p=t->cursor;
	int e=(p/t->width+1)*t->width;
	int n;
	for (n=p; n<e; n++) t->cells[n]=32;
	fprintf(t->f, "\x18");//CAN Clear till end of line

}

void term_set_string_(terminal_t *t, int *row, int *col, const int uc)
{
	character_t c=uc;
	term_set_character(t, *row, *col, c);
	*col=*col+1;
	if (*col>=t->width) {
		*col=0;
		*row=*row+1;
		if (*row>=t->height) {
			*row=0;
		}
	}
}

void term_set_string(terminal_t *t, const int row, const int col, const char *s)
{
	int ro=row;
	int co=col;
	int len=strlen(s);
	int uc=0; //Unicode character
	int un=0; //Number of UTF-8 octets to follow
	int n;
	for (n=0; n<len; n++) {
		int c=s[n];
		if (c<0x80) {
			term_set_string_(t, &ro, &co, c);
			uc=0;
		} else 
		if ((c&0xC0)==0x80) {
			uc=(uc<<6) | (c&0x3f);
			un=un-1;
			if (un==0) term_set_string_(t, &ro, &co, uc);
		}  else if ( (c&0xe0) == 0xC0 ) {
			uc=c&0x1f;
			un=1;
		} else if ( (c&0xf0) == 0xE0) {
			uc=c&0x0f;
			un=2;
		} else if ( (c&0xf8) == 0xf0) {
			un=c&0x07;
			un=3;
		}
	}
}

void term_set_status(terminal_t *t, const char *s)
{
	term_set_string(t, 23, 0, s);
	term_clear_eol(t);
}

void term_get_talking(terminal_t *t)
{
	fprintf(t->f, "\x11\x1a");
}


