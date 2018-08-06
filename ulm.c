#include "settings.h"
#include <stdio.h>


#include "character.h"
#include "terminal.h"
#include "editor.h"
#include "display.h"


int main(int argc, char **argv)
{
	if (argc!=2) {
		printf("Usage: %s <directory>\n", argv[0]);
		return 0;
	}


	terminal_t *t=term_create(stdout, 0, 0);
	term_clear_screen(t);
	
	browser(stdin, t, argv[1]);
	
	term_free(t);


	return 0; //read_or_edit_screen(stdin, stdout, &screen, BIT8);
}
