#include "settings.h"
#include "character.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define BASEMASK (0xff)

#define ACCENTPOS (8)
#define ACCENTMASK (0x0f)

#define CODEPAGEPOS (12)
#define CODEPAGEMASK (0x03)


#define SUPS_START (0x80)
#define SUPS_STOP (0x9f)

#define SS2 (0x19)
#define SS3 (0x1D)


const bool is_supplementary_set(const character_t c)
{
	const uint8_t c2=c&BASEMASK;
	if (c2<=SUPS_START) return false;
	if (c2>=SUPS_STOP)  return false;
	return true;
}

const bool is_blank_character(const character_t c)
{
	if (c==0) return true;
	if (c==0x20) return true;
	return false;
}


void print_char(FILE *f, const character_t c, const bits_t b)
{
	if (c==0) return print_char(f, 32, b); //0 is also space
	uint8_t base=c&BASEMASK;
	if ( is_supplementary_set(c) ) {
		if (b==BIT7) {
			base=(base&0x1f)+0x40; 
			fprintf(f, "\x1b\x22\x40%c", base);
		} else	fprintf(f,"%c",base);
		return;
	}
	uint8_t accent=(c>>ACCENTPOS)&ACCENTMASK;
	if (accent!=0) {
		fprintf(f, "\x19%c", accent+0x40); //SS2 Single Shift G2 + accent character
	}
	uint8_t page=(c>>CODEPAGEPOS) & CODEPAGEMASK;
	uint8_t bbase=(c&0x7f);
	if (page==0) fprintf(f, "%c", bbase);
	if (page==1) fprintf(f, "%c", bbase);
	if (page==2) fprintf(f, "\x19%c", bbase); //SS2 + character
	if (page==3) fprintf(f, "\x1D%c", bbase); //SS3 + character 
	return;
}	

const character_t read_char(FILE *f, const char start)
{
	character_t ch=0;

}

const character_t char_read(FILE *f)
{
	if (feof(f)) return -1;
	character_t ch=0;
	int start=fgetc(f);	
	if (start==SS2) {
		char c=fgetc(f);
		if ( (c>=0x40) && (c<=0x4f) ) { //accent
			int accent=c-0x40;
			c=fgetc(f);
			ch=(c&0x7f) | accent<<ACCENTPOS;
			return ch;
		} else {
			return (2<<CODEPAGEPOS) | c;
		}
	}
	if (start==SS3) {
		char c=fgetc(f);
		return (2<<CODEPAGEPOS) | c;
	}
	if ( (start>=0x20) && (start<=0x7f) ) return start;
	if ( (start>=SUPS_START) && (start<=SUPS_STOP) ) return start;
	return start;
}

