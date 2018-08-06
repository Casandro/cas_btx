#include "display.h"
#include "editor.h"
#include <string.h>
#include <strings.h>



editor_t *load_links(const char *dir, const char *pn)
{
	char lnkf[256];
	snprintf(lnkf, sizeof(lnkf), "%s/%s.lnk", dir, pn);
	fprintf(stderr, "load_links %s\n",lnkf);
	editor_t *e=editor_create(NULL, NULL, lnkf);
	return e;
}



int find_lnk(const char *inp, editor_t *e, char *next)
{
	next[0]=0;
	return 0;
}


int load_page(FILE *in, terminal_t *t, const char *dir, const char *pn, char *next)
{
	fprintf(stderr, "load_page\n");
	int res=0;
	char pgf[256];
	snprintf(pgf, sizeof(pgf), "%s/%s.btx", dir, pn);
	char c;
	FILE *f=fopen(pgf, "r");
	if (f!=NULL) {
		while (!feof(f)) {
			c=fgetc(f);
			fprintf(t->f, "%c", c);
		}	
		fclose(f);
	} else return -2;
//	term_set_string(t, 0, 1, "Anbieterkennung");
	term_set_string(t, 0, 25, pn);
	fprintf(t->f, "\x1a");
	char inp[256];
	memset(inp, 0, sizeof(inp));
	editor_t *e=load_links(dir, pn);
	int type=0;
	while (c=fgetc(in)) {
		int l=strlen(inp);
		if ( (c==0x13) ) { //*
			if (type==0) {
				type=1; //first *
			} else type=0;
			inp[0]=0; //Delete input
		} else
		if ( (c==0x1c)) { //#
			if (inp[0]!=0) {
				res=1;
				strcpy(next, inp);
			} else res=-1;
			inp[0]=0;
			break;
		} else {
			inp[l]=c;
			inp[l+1]=0;
		}
		fprintf(stderr, "type=%d input=%s char=%02x\n", type, inp, c);
		char status[256];
		status[0]=0;
		if (type==1) strcat(status, "*");
		strcat(status, inp);
		term_set_status(t, status);
		if ( (type==0) && (find_lnk(inp, e, next)!=0)) {
			res=1;
			break;
		}
		
	}
	editor_free(e);	
	return res;
}

int browser(FILE *in, terminal_t *t, const char *dir)
{
	char pn[256];
	memset(pn, 0, sizeof(pn));
	char next[256];
	memset(next, 0, sizeof(next));
	char prev[256];
	memset(prev, 0, sizeof(prev));
	int res=0;
	strcpy(pn, "0");
	while (0==0) {
		res=load_page(in, t, dir, pn, next);
		fprintf(stderr,"pn=%s next=*%s#\n", pn, next);
		if (res==1) {
			if (strcmp(next,"9")==0) return 0; //ende
			if (strcmp(next,"910")==0) {
				editor_edit(in, t, dir);
				strcpy(next,"0");
			}
			strcpy(prev, pn);
			strcpy(pn, next);
		}
		if (res==-1) {
			strcpy(pn, prev);
		}
		if (res==-2) { //Page not found
			strcpy(pn, "0");
		}
	}
	return 0;
}


