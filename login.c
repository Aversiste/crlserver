/*
 * Copyright (c) 2013 Tristan Le Guern <leguern AT medu DOT se>
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

#include <sys/param.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "crlserver.h"
#include "db.h"
#include "log.h"
#include "pathnames.h"

int
login_create_session(char * const username, char * const password) {
	char path[MAXPATHLEN];
	const char *db_path;
	
	if (options.o_db != NULL)
		db_path = options.o_db;
	else
		db_path = CRLSERVER_PLAYGROUND"/"CRLSERVER_DATABASE;

	if (db_check(db_path) == -1)
		db_init(db_path);
	db_open(db_path);

	if (db_user_auth(username, password) != 0)
		return -1;

	session.logged = 1;
	session.name = strdup(username);

	(void)memset(path, 0, sizeof path);
	(void)snprintf(path, sizeof path, "%s/%c/%s",
		 CRLSERVER_PLAYGROUND"/userdata",
		 session.name[0], session.name);

	session.home = strdup(path);

	if (access(session.home, F_OK) != 0)
#if defined HAVE_LDAP
		if (register_create_home(username) == -1)
			goto clean;
#else
		goto clean;
#endif
	return 0;

clean:
	free(session.home);
	free(session.name);
	session.logged = 0;
	return -1;
}

void
login_destroy_session(void) {
	db_close("db");
	free(session.home);
	free(session.name);
	session.home = NULL;
	session.name= NULL;
	session.logged = 0;
}

/* Replace %user% with the actual username in the various games options */
int
login_build_list(struct list_head *headp) {
	struct list *lp;
	char buf[MAXPATHLEN + 5];

	SLIST_FOREACH(lp, headp, l_next) {
		unsigned int i;
		for (i = 0; lp->l_params[i] != NULL; ++i) {
			if (strcmp(lp->l_params[i], "%user%") != 0)
				continue;
			free(lp->l_params[i]);
			lp->l_params[i] = strdup(session.name);
		}
		for (i = 0; lp->l_env[i] != NULL; ++i) {
			if (strncmp(lp->l_env[i], "HOME=", 5) != 0)
				continue;
			free(lp->l_env[i]);
			(void)snprintf(buf, sizeof buf,
			    "HOME=%s", session.home);
			lp->l_env[i] = strdup(buf);
			break;
		}
	}
}

/* Replace the actual username with %user% in the various games options */
int
login_unbuild_list(struct list_head *headp) {
	struct list *lp;
	char buf[MAXPATHLEN + 5];

	SLIST_FOREACH(lp, headp, l_next) {
		unsigned int i;
		for (i = 0; lp->l_params[i] != NULL; ++i) {
			if (strcmp(lp->l_params[i], session.name) != 0)
				continue;
			free(lp->l_params[i]);
			lp->l_params[i] = strdup("%user%");
		}
	}
}

