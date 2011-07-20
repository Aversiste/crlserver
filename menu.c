/*
 * Copyright (c) 2011 Tristan Le Guern <leguern@medu.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <sys/queue.h>
#include <curses.h>
#include <unistd.h>
#include "games.h"
#include "crlserver.h"

void
print_game_menu(int highlight) {
	struct games_list *glp;
	int y = 2;
	int i = 1;

	SLIST_FOREACH(glp, &gl_head, gls) {
		if (highlight == i) {
			attron(A_REVERSE);
			mvaddstr(y, 2, glp->name);
			attroff(A_REVERSE);
		}
		else
			mvaddstr(y, 2, glp->name);
		++y;
		++i;
	}
}

void
print_menu(int highlight) {
	char *choices[] = { 
		"login",
		"register",
	};
	int n_choices = sizeof(choices) / sizeof(char *);
	int x, y, i;	

	x = 2;
	y = 2;
	for(i = 0; i < n_choices; ++i)
	{	if(highlight == i + 1) /* High light the present choice */
		{	attron(A_REVERSE); 
			mvprintw(y, x, "%s", choices[i]);
			attroff(A_REVERSE);
		}
		else
			mvprintw(y, x, "%s", choices[i]);
		++y;
	}
	refresh();
}

void
general_menu(void) {
	int highlight = 1;
	int choice = 0;
	int c;

	do {
		print_menu(highlight);
		c = getch();
		switch (c) {
		case KEY_UP:
			if (highlight == 1)
				highlight = gl_length;
			else
				--highlight;
			break;
		case KEY_DOWN:
			if (highlight == gl_length)
				highlight = 1;
			else 
				++highlight;
			break;
		case KEY_ENTER:
		case '\r':
		case '\n':
			choice = highlight;
			break;
		default:
			refresh();
			break;
		}
		if (choice != 0)
			break;
	} while (1);

	(void)refresh();
	(void)sleep(1);
	release_games(&gl_head);
}
