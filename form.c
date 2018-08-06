#include "form.h"

#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "terminal.h"



int form_edit_field(form_field_t *f, FILE *in, terminal_t *t)
{
	fprintf(stderr, "form_edit_field %d\n", f->len);
	if (f==NULL) return 0;
	if (in==NULL) return 0;
	if (t==NULL) return 0;
	term_get_talking(t);
	int n;
	for (n=0; n<f->len; n++) {
		character_t c=f->data[n];
		if (c==0) c=32;
		term_set_character(t, f->row, f->col+n, c);
	}
	int p=0;
	while (0==0) {
		p=p%f->len;
		term_move_cursor(t, f->row, f->col+p);
		character_t c=char_read(in);
		if (c==0x08) p=p-1; //Left
		if (c==0x09) p=p+1; //Right
		if (c==0x0A) return 1; //Down
		if (c==0x0B) return -1; //Up
		if (c==0x1c) return 2; //Terminator => OK
		if (c==0x13) { //Initiator
			c=char_read(in);
			if (c==0x1c) return 3; //*# => back
		}
		int accept=0;
		if ((f->mode=='U') && (c>=0x20)) accept=1;
		if ( (f->mode=='N') && (c>='0') && (c<='9') ) accept=1;
		if ( (f->mode=='A') && (c>=0x20) && (c<=0x7f)) accept=1;
		if (accept!=0) {
			f->data[p%f->len]=c;
			term_set_character(t, f->row, f->col+p, c);
			p=p+1;
		}
	}
	return 0;
}

form_field_t *form_rewind_fields(form_field_t *f)
{
	if (f==NULL) return NULL;
	if (f->prev==NULL) return f;
	return form_rewind_fields(f->prev);
}

int form_handle_editing(form_t *f)
{
	fprintf(stderr, "form_handle_editing\n");
	if (f==NULL) return 0;
	f->fields=form_rewind_fields(f->fields);
	form_field_t *field=f->fields;
	while (0==0)
	{
		if (field==NULL) field=f->fields;
		int res=form_edit_field(field, f->in, f->term);
		if (res==-1) field=field->prev;
		if (res==1) field=field->next;
		if (res==2) {
			if (field->next==NULL) return 1;
			field=field->next;
		}
		if (res==3) return 0; //chancelled
	}
	return 0;
}

form_field_t *form_add_fields(form_field_t *prev, const int row, const int col, const int len, const char mode, const char *name, const char *val)
{
	if (name==NULL) return prev;
	if (len<=0) return prev;
	form_field_t *f=malloc(sizeof(form_field_t));
	if (f==NULL) return prev;
	memset(f, 0, sizeof(form_field_t));
	int nl=strlen(name)+1;
	f->name=malloc(nl);
	if (f->name!=NULL) {
		memset(f->name, 0, nl);
		strncpy(f->name, name, nl-1);
	}
	f->data=malloc(len*sizeof(character_t));
	if (f->data!=NULL) {
		int n;
		for (n=0; n<len; n++) f->data[n]=32;
		if (val==NULL) {
			for (n=0; n<strlen(val); n++) {
				f->data[n]=val[n];
			}
		}
	}
	f->mode=mode;
	f->row=row;
	f->col=col;
	f->len=len;
	f->prev=prev;
	if (prev!=NULL) prev->next=f;
	return f;
}


form_field_t *form_find_field(form_field_t *f, const char *name)
{
	if (f==NULL) return f;
	if (strcmp(f->name, name)==0) return f;
	return form_find_field(f->next, name);
}

void form_del_fields(form_field_t *f)
{
	if (f==NULL) return;
	form_field_t *n=f->next;
	if (f->name!=NULL) free(f->name);
	if (f->data!=NULL) free(f->data);
	free(f);
	return form_del_fields(n);
}

void form_free(form_t *f)
{
	form_del_fields(f->fields);
	free(f);
}

form_t *form_create(FILE *in, terminal_t *term)
{
	form_t *f=malloc(sizeof(form_t));
	if (f==NULL) return NULL;
	memset(f, 0, sizeof(form_t));
	f->term=term;
	f->in=in;
	return f;
}


void form_add_field(form_t *f, const int row, const int col, const int len, const char mode, const char *name, const char *val)
{
	if (f==NULL) return;
	if (row<0) return;
	if (col<0) return;
	f->fields=form_add_fields(f->fields, row, col, len, mode, name, val);
}	

void form_read_field(form_t *f, const char *name, char *dest, size_t size)
{
	form_field_t *fld=form_find_field(f->fields, name);
	if (fld==NULL) return;
	memset(dest, 0, size);
	int len=0;
	int n;
	for (n=0; n<fld->len; n++) {
		len=fld->len-n;
		character_t c=fld->data[len-1];
		if ( (c!=32) && (c!=0) ) break;
	}
	if (len>size-1) len=size-1;
	for (n=0; n<len; n++) dest[n]=fld->data[n];
	dest[len]=0;
}
