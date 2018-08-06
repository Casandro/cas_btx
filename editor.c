#include "editor.h"
#include "terminal.h"
#include "form.h"
#include <string.h>
#include <stdlib.h>




int editor_load_or_edit(FILE *in, editor_t *e);

void editor_doc_clear(editor_doc_t *d)
{
	if (d==NULL) return;
	int n;
	for (n=0; n<EDITOR_SIZE; n++) d->cells[n]=32;
}

void editor_doc_save_line(editor_doc_t *d, const int line,  FILE *out)
{
	int n;
	int endspaces=0;
	for (n=EDITOR_WIDTH-1; n>=0; n--) {
		if (d->cells[line*EDITOR_WIDTH+n]==32) endspaces=endspaces+1;
		else break;
	}
	if (endspaces!=EDITOR_WIDTH) {
		fprintf(out, "\x1f%c\x41", 0x41+EDITOR_TOP+line);
		for (n=0; n<EDITOR_WIDTH-endspaces; n++) 
			print_char(out, d->cells[line*EDITOR_WIDTH+n], BIT8);
	}
}

void editor_doc_save(editor_doc_t *d, const char *fn)
{
	FILE *out=fopen(fn, "w");
	fprintf(out,"\x1f\x42\x41\x0c");
	int row;
	for (row=0; row<EDITOR_HEIGHT; row++) editor_doc_save_line(d, row, out);
	fclose(out);
}



editor_t *editor_create(FILE *in, terminal_t *t, char *fn)
{
	editor_t *e=malloc(sizeof(editor_t));
	if (e==NULL) return NULL;
	memset(e, 0, sizeof(editor_t));
	e->term=NULL;
	size_t len=strlen(fn)+1;
	e->filename=malloc(len);
	if (e->filename==NULL) {
		free(e);
		return NULL;
	}
	strncpy(e->filename, fn, len);
	e->filename[len-1]=0; //In case fn changes while this executes
	e->cursor=0;
	editor_doc_clear(&(e->doc));
	FILE *f=fopen(e->filename, "r");
	if (f!=NULL) {
		int res=editor_load_or_edit(f, e);
		fclose(f);
		if (res<0) { //Incompatible format
			editor_free(e);
			return NULL;
		}
	} else fprintf(stderr, "editor_create could not open %s\n", e->filename);
	e->term=t;
	return e;
}


void editor_free(editor_t *e)
{
	if (e==NULL) return;
	if (e->filename!=NULL) free(e->filename);
	free(e);
}

void editor_set_cursor(editor_t *e, const int cursor)
{
	e->cursor=cursor;
	if (e->term!=NULL) {
		int row=(e->cursor)/EDITOR_WIDTH+EDITOR_TOP;
		int col=(e->cursor)%EDITOR_WIDTH;
		term_move_cursor(e->term, row, col);
	}
}

void editor_move_cursor(editor_t *e, const int diff)
{
	if (e==NULL) return;
	if (diff==0) return;
	if (e->cursor<0) return;
	if (e->cursor>=EDITOR_SIZE) return;
	editor_set_cursor(e, (e->cursor+EDITOR_SIZE+diff)%EDITOR_SIZE);
}

void editor_clear_screen(editor_t *e)
{
	editor_doc_clear(&(e->doc));
	if (e->term!=NULL) term_clear_screen(e->term);
}

void editor_set_character(editor_t *e, character_t c)
{
	if (e->cursor<0) return;
	if (e->cursor>=EDITOR_SIZE) return;
	e->doc.cells[e->cursor]=c;
	if (e->term!=NULL) {
		int row=(e->cursor)/EDITOR_WIDTH+EDITOR_TOP;
		int col=(e->cursor)%EDITOR_WIDTH;
		term_set_character(e->term, row, col, c);
	}
	editor_move_cursor(e, 1);
}

void editor_clear_eol(editor_t *e)
{
	int start=e->cursor;
	if (start<0) return;
	if (start>=EDITOR_SIZE) return;
	int end=(start/EDITOR_WIDTH+1)*EDITOR_WIDTH;
	int n;
	for (n=start; n<end; n++) e->doc.cells[n]=32;
	if (e->term!=NULL) {
		editor_set_cursor(e, (e->cursor));
		term_clear_eol(e->term);
	}
}

int editor_getpos(const int row, const int col)
{
	if (row<EDITOR_TOP) return -1;
	if (row>=EDITOR_TOP+EDITOR_HEIGHT) return -1;
	if (col<0) return -1;
	if (col>=EDITOR_WIDTH) return -1;
	return (row-EDITOR_TOP)*EDITOR_WIDTH+col;
}

void editor_redraw_line(editor_t *e, const int row)
{
	if (e->term==NULL) return;
	int col=0;
	term_move_cursor(e->term, row+EDITOR_TOP, 0);
	term_clear_eol(e->term);
	for (col=0; col<EDITOR_WIDTH; col++) {
		character_t dc=e->doc.cells[row*EDITOR_WIDTH+col];
		if (dc!=32) term_set_character(e->term, row+EDITOR_TOP, col, dc);
	}
}


int editor_handle_esc(FILE *in, editor_t *e)
{
	char c=fgetc(in);
	if (c==0x22) {
		c=fgetc(in);
		if (c==0x40) {
	/*		c=fgetc(in);
			character_t ch=0x80+(c&0x1f);
//			editor_move_cursor(e, -1); //Overwrite last character to get rid of space
			editor_set_character(e,ch);
			return 1;*/
		}
		if (c==0x41) return -1;
	}
	return 0;
}


void editor_redraw_screen(editor_t *e)
{
	term_clear_screen(e->term);
	int row=0;
	int col=0;
	for (row=0; row<EDITOR_HEIGHT; row++) 
	for (col=0; col<EDITOR_WIDTH; col++) {
		character_t dc=e->doc.cells[row*EDITOR_WIDTH+col];
		if (dc!=32) term_set_character(e->term, row+EDITOR_TOP, col, dc);
	}
	term_set_string(e->term, 0, 0, "Editor:");
}

int editor_handle_menu(FILE *in, editor_t *e)
{
	//                        1234567890123456789012345678901234567890
	term_set_status(e->term, "Exit_#, Save:0, Color:1, Attributes:2");
	char c=fgetc(in);
	if (c==0x1c) return -1;
	if (c==0x13) {
		editor_redraw_screen(e);
		return 0;
	}
	if (c=='0') {
		editor_doc_save(&(e->doc), e->filename);
		term_set_status(e->term, "saved");
		return 0;
	}
	if (c=='1') {
		//                        1234567890123456789012345678901234567890
		term_set_status(e->term, "Alpha:1, Mosaik:2, Background: 3");
		c=fgetc(in);
		if ((c=='1') || (c=='2')) {
			term_set_status(e->term, "Color: 1-7");
			char co=fgetc(in);
			if (co<'1') return 0;
			if (co>'7') return 0;
	
			if (c=='1') editor_set_character(e, 0x80+co-'0');
			if (c=='2') editor_set_character(e, 0x90+co-'0');
		}
		if (c=='3') {
			term_set_status(e->term, "New BG: 1, Black BG: 2");
			char co=fgetc(in);
			if (co=='1') editor_set_character(e, 0x9D);
			if (co=='2') editor_set_character(e, 0x9C);
	
		}
		editor_redraw_line(e, e->cursor/EDITOR_WIDTH);
		return 0;
	}
	if (c=='2') {	
		//                        1234567890123456789012345678901234567890
		term_set_status(e->term, "Attrib: 1:NZS, 2:DBH, 3:FSH, 4:STD");
		c=fgetc(in);
		if (c=='1') editor_set_character(e, 0x8C);
		if (c=='2') editor_set_character(e, 0x8D);
		if (c=='3') editor_set_character(e, 0x88);
		if (c=='4') editor_set_character(e, 0x89);
	}
	return 0;
}

void editor_update_status(FILE *in, editor_t *e)
{
	if (e->term==NULL) return;
	int row=(e->cursor)/EDITOR_WIDTH+EDITOR_TOP;
	int col=(e->cursor)%EDITOR_WIDTH;
	char status[EDITOR_WIDTH+1];
	memset(status, 0, sizeof(status));
	snprintf(status, EDITOR_WIDTH, "%02d:%02d=%05x", row, col, e->doc.cells[e->cursor]);
	term_set_status(e->term, status);
	term_move_cursor(e->term, row, col);
}


int editor_load_or_edit(FILE *in, editor_t *e)
{
	if (e->term!=NULL) {
		editor_redraw_screen(e);
	}
	if (e->term!=NULL) term_get_talking(e->term);
	fprintf(stderr, "editor_load_or_edit\n");
	int charactercount=0;
	character_t lc=0; //Last inserted character
	while (!feof(in)) {
		int n;
		editor_update_status(in, e);
		character_t c=char_read(in);
		if (c<0) {
			fprintf(stderr, "editor_load_or_edit %d characters loaded\n", charactercount);
			return 0;
		}
		charactercount=charactercount+1;
		switch (c) {
			case 0x08: //Active Position Backwards
				editor_move_cursor(e, -1);
				break;
			case 0x09: //Active Position Forward
				editor_move_cursor(e, 1);
				break;
			case 0x0a: //Active Position Down
				editor_move_cursor(e, EDITOR_WIDTH);
				break;
			case 0x0b: //Active Position Up
				editor_move_cursor(e, -EDITOR_WIDTH);
				break;
			case 0x0c: //Clear Screen
				editor_clear_screen(e);
				break;
			case 0x0d: //Active Position Return
				editor_set_cursor(e, (e->cursor/EDITOR_WIDTH)*EDITOR_WIDTH);
				break;
			case 0x0e: //Shift In
			case 0x0f: //Shift Out
			case 0x11: //Cursor On
				break;
			case 0x12:; //Repeat
				int rpt=fgetc(in)%0x1f;
				for (n=0; n<rpt; n++) editor_set_character(e, lc);
				break;
			case 0x13:; //INItiator *-Taste
				int res=0;
				if (e->term!=NULL) res=editor_handle_menu(in, e);
				if (res<0) return 0;
				break;
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
				break;
			case 0x18: //CAN erase end of line
				editor_clear_eol(e);
				break;
			case 0x19: //SS2 Should be handled by char_read
			case 0x1A: 
				break;
			case 0x1B:; //ESC
				res=editor_handle_esc(in, e);
				if (res<0) return -1; //Incompatible
				break;
			case 0x1c: 
			case 0x1d: //SS3 Should be handled by char_read
				break;
			case 0x1e: //Active Position Home
				editor_set_cursor(e, 0);
				break;
			case 0x1f:; //Active Position Addressing
				int ar=fgetc(in);
				if (ar<0x41) return -1; //Not an APA, but an Unit Separator, not supported
				int ac=fgetc(in);
				if (ac<0x41) return 0; //Some other problem, eof or something
				int row=ar-0x41; //Row	
				int col=ac-0x41; //Column
				int pos=editor_getpos(row, col);
				e->cursor=pos; //If -1 then writing will be inhibited
				break;
			default: //Normal character
				editor_set_character(e, c);
				lc=c;
				break;
		}
	}

}


int editor_edit_file(FILE *in, terminal_t *term, char *fn)
{
	editor_t *e=editor_create(in, term, fn);
	if (e==NULL) return -1;
	if (in!=NULL) {
		editor_load_or_edit(in, e);
	}
	editor_free(e);
	return 0;
}

int editor_edit_dir_ext(FILE *in, terminal_t *term, const char *dir, const char *page, const char *ext)
{
	char tmp[1024];
	snprintf(tmp, sizeof(tmp), "%s/%s.%s", dir, page, ext);
	return editor_edit_file(in, term, tmp);
}


int editor_edit(FILE *in, terminal_t *term, const char *dir)
{
	char pageno[256];
	char option[4];
	form_t *f=form_create(in, term);
	form_add_field(f, 5,18, 16, 'N', "number", "");
	form_add_field(f, 6,18,  1, 'N', "option", "");
	while (0==0) {
		term_clear_screen(term);

		term_set_character(term, 1, 0, 0x8D);
		term_set_string(term, 1, 1, "Editor");
		term_set_string(term, 5, 2, "page number:");
	 	term_set_string(term, 6, 2, "option:");
		term_set_string(term, 7, 3, "1=edit page");
		term_set_string(term, 8, 3, "2=edit links");	
		int res=form_handle_editing(f);
		form_read_field(f, "number", pageno, sizeof(pageno));
		form_read_field(f, "option", option, sizeof(option));

		fprintf(stderr, "Number: *%s# %s\n", pageno, option);
		if (res==1) {
			if (option[0]=='1') editor_edit_dir_ext(in, term, dir, pageno, "btx");	
			if (option[0]=='2') editor_edit_dir_ext(in, term, dir, pageno, "lnk");
		} else break;
	}	
	form_free(f);
}

