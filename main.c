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

#include <stdlib.h>
#include <sysexits.h>

#include "conf.h"
#include "init.h"
#include "pathnames.h"
#include "menu.h"

size_t	gl_length;
size_t	el_length;

int
main(void) {
	games_list_head gl_head = SLIST_HEAD_INITIALIZER(gl_head);
	editors_list_head el_head = SLIST_HEAD_INITIALIZER(el_head);

	load_folder(GAMES_DIR, &gl_head);
	load_folder(EDITORS_DIR, &el_head);
	gl_length = list_size((struct list_head*)&gl_head);
	el_length = list_size((struct list_head*)&el_head);

	init();
	menus();
	end_window();
	return (EX_OK);
}
