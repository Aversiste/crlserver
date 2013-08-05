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
		goto clean;
#if defined HAVE_LDAP
	else if (register_create_home(username) == -1)
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

