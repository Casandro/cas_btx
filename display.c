#include "display.h"
#include "editor.h"
#include <string.h>
#include <strings.h>



editor_t *load_links(const char *dir, const char *pn)
{
	fprintf(stderr, "load_links\n");
	char lnkf[256];
	snprintf(lnkf, sizeof(lnkf), "%s/%s.lnk", dir, pn);
	editor_t *e=editor_create(NULL, NULL, lnkf);
	return e;
}


int find_lnk(const char *inp, editor_t *e, char *next)
{
	int n;
	for (n=0; n<EDITOR_SIZE; n++) {
		int l=strlen(inp);
		int fnd=1;
		int m;
		for (m=0; m<l; m++) {
			if (e->doc.cells[(n+m)%EDITOR_SIZE]!=inp[m]) {
				fnd=0;
				break;
			}
		}
		if ((fnd!=0) && (e->doc.cells[(n+l)%EDITOR_SIZE]!='>')) {
			int ps=0;
			int p=(n+l+1)%EDITOR_SIZE;
			while (1==1) {
				character_t c=e->doc.cells[p];
				p=(p+1)%EDITOR_SIZE;
				if (c<'0') break;
				if (c>'9') break;
				next[ps]=c;
				ps=ps+1;
			}
			next[ps]=0;
			return 1;
		}
	}
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
	}
	fprintf(t->f, "\x1a");
	char inp[256];
	memset(inp, 0, sizeof(inp));
	editor_t *e=load_links(dir, pn);
	int type=0;
	while (c=fgetc(in)) {
		int l=strlen(inp);
		if ( (c==0x13) ) { //*
			type=1; //first *
			inp[0]=0; //Delete input
		} else
		if ( (c==0x1c)) { //#
			res=1;
			strcpy(next, inp);
			inp[0]=0;
			break;
		} else {
			inp[l]=c;
			inp[l+1]=0;
		}
		fprintf(stderr, "type=%d input=%s char=%02x\n", type, inp, c);
		term_set_status(t, inp);
		if (find_lnk(inp, e, next)!=0) {
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
			if (strcmp(next,"910")==0) editor_edit(in, t, dir);
			strcpy(prev, pn);
			strcpy(pn, next);
		}
		if (res==-1) {
			strcpy(pn, prev);
		}
	}
	return 0;
}


