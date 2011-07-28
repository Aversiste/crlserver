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

#ifdef __OpenBSD__
# include <util.h>
#elif __Linux__
# include "compat/util.h"
#endif

#include <sys/queue.h>
#include <curses.h>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>

#include "crlserver.h"
#include "general_menu.h"
#include "init.h"
#include "menus.h"

void
print_file(const char *path) {
	FILE *fd = fopen(path, "r");
	int y = 0;
	char *buf;
	const char sep[3] = {'\\', '\\', 0};

	(void)erase();
	if (fd == NULL)
		clean_up("Important file is missing");

	while ((buf = fparseln(fd, NULL, NULL,
	  sep, FPARSELN_UNESCALL)) != NULL) {
		mvprintw(y, 1, buf);
		free(buf);
		++y;
	}
	refresh();
}

void
menus(void) {
	unsigned char c;

	print_file("menus/general.txt");
	while ((c = getch()) != 'q') {
		switch (c) {
		case 'l':
			mvprintw(LINES - 1, 0, "s");
			break;
		case 'r':
			mvprintw(LINES - 1, 0, "s");
			break;
		case 's':
			print_file("menus/server_info.txt");
			break;
		case 'q':
			clean_up("Good Bye");
			break;
		default:
			break;
		}
	}
}
