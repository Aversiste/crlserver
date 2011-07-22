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
#include <stdlib.h>
#include <menu.h>
#include "games.h"
#include "crlserver.h"

/*
 * TODO: - mouse support
 *       - put menu in a window
 *       - multi columns menu
 */

void
game_menu(void) {
	MENU *menu;
	ITEM **items;
	unsigned int i;
	unsigned int c;
	struct games_list *glp;

	items = calloc(gl_length + 1, sizeof(*items));
	SLIST_FOREACH(glp, &gl_head, gls) {
		items[i] = new_item(glp->name, glp->version);
		++i;
	}
	items[gl_length] = (ITEM*)NULL;

	mvprintw(1, 1, "%s %s", "game", "version");
	menu = new_menu(items);
	set_menu_spacing(menu, TABSIZE, 0, 0);
	post_menu(menu);
	refresh();
	
	while ((c = getch()) != KEY_F(1)) {
		switch (c) {
			case KEY_DOWN:
			menu_driver(menu, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
			menu_driver(menu, REQ_UP_ITEM);
			break;
			case KEY_HOME:
			menu_driver(menu, REQ_FIRST_ITEM);
			break;
			case KEY_END:
			menu_driver(menu, REQ_LAST_ITEM);
			break;
		}
	}

	(void)unpost_menu(menu);
	(void)free_menu(menu);
	/*for (i = 0, i < gl_length, i += 1) {
		(void)free_item(items[i]);
	}*/
}
