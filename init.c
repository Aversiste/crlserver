/*
 * Copyright (c) 2011 Tristan Le Guern <leguern@medu.se>
 *
 r
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

#include <stdlib.h>
#include <sys/queue.h>
#include <sysexits.h>
#include <err.h>
#include <curses.h>

#include "games.h"
#include "crlserver.h"
#include "init.h"

void
init(void) {
	SLIST_INIT(&gl_head);
	init_games(&gl_head);
	init_gl_length();

	initscr();
	if (has_colors() == TRUE)
		start_color();
	curs_set(0);

	start_window();

	if ((LINES < DROWS) || (COLS < DCOLS))
		clean_up("must be displayed on 24 x 80 screen (or larger)");
}

void
start_window(void) {
	(void)cbreak();
	(void)noecho();
	(void)nonl();
	(void)keypad(stdscr, TRUE);
}

__inline void
end_window() {
	(void)endwin();
}

void
clean_up(const char *estr) {
	(void)move(DROWS-1, 0);
	(void)refresh();
	end_window();
	(void)printf("\n%s\n", estr);
	exit(EX_SOFTWARE);
}

void
init_gl_length(void) {
	size_t s = 0;
	struct games_list *glp;

	SLIST_FOREACH(glp, &gl_head, gls)
		++s;

	gl_length = s;
	warnx("gl_length: %u", (unsigned int)gl_length);
}
