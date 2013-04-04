/*
 * Copyright (c) 2011 Tristan Le Guern <leguern AT medu.se>
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

#include "crlserver.h"
#include "pathnames.h"
#include "list.h"

struct session session;
struct options options;

/*
 * TODO:
 *    -u: batch creation of users
 *    -l user -p password: autologin of a user
 */

int
main(void) {
	struct list_head *headp;
	struct list *lp, *lt;

	headp = conf_load_file(CRLSERVER_CONFIG_DIR"/crlserver.conf");
	if (headp == NULL)
		return (1);
	init();
	menu_general(headp);
	endwin();
	SLIST_FOREACH_SAFE(lp, headp, l_next, lt) {
		free(lp->l_name);
		free(lp->l_longname);
		free(lp->l_description);
		free(lp->l_version);
		free(lp->l_path);
		for (unsigned int i = 0; lp->l_params[i] != NULL; ++i)
			free(lp->l_params[i]);
		for (unsigned int i = 0; lp->l_env[i] != NULL; ++i)
			free(lp->l_env[i]);
	}
	return (EX_OK);
}

