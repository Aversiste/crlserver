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
#include <err.h>
#include "rlmenu.h"
#include "games.h"
#include "crlserver.h"
#include "init.h"

/*
 * TODO: - mouse support
 *       - put menu in a window
 *       - multi columns menu
 */

static void
init_items_general(ITEM ***items) {
	unsigned int i = 0;
	const char *names[] = {"Login", "Register", "Quit"};

	for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
		(*items)[i] = new_item(names[i], names[i]);
}

static void
menu_opt_general(MENU **menu) {
	set_menu_spacing(*menu, TABSIZE, 0, 0);
	menu_opts_off(*menu, O_SHOWDESC);
}

static void
init_items_game(ITEM ***items) {
	struct games_list *glp;
	unsigned int i = 0;

	SLIST_FOREACH(glp, &gl_head, gls) {
		(*items)[i] = new_item(glp->name, glp->version);
		++i;
	}
}

static void
menu_opt_game(MENU **menu) {
	set_menu_spacing(*menu, TABSIZE, 0, 0);
}

void
menu_tpl(size_t length, void (*init_items)(ITEM ***), void (*menu_opt)(MENU **)) {
	MENU *menu;	
	ITEM **items;
	unsigned int i;
	unsigned int c;

	items = calloc(length, sizeof(*items));
	if (items == (ITEM**)NULL)
		clean_up("items error");
	init_items(&items);
	items[length] = (ITEM*)NULL;

	menu = new_menu(items);
	if (menu == (MENU*)NULL)
		clean_up("menu error");
	menu_opt(&menu);
	if (post_menu(menu) != E_OK)
		clean_up("menu error");

	(void)refresh();

	while ((c = getch()) != KEY_F(1))
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
		default:
			break;
		}

	(void)unpost_menu(menu);
	(void)free_menu(menu);
	for (i = 0; i < length; ++i)
		(void)free_item(items[i]);
}

void
menu(void) {
	menu_tpl(3, &init_items_general, &menu_opt_general);
}
