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

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>

#include "crlserver.h"
#include "db.h"
#include "init.h"
#include "log.h"

static int
user_exist_callback() {
	scrmsg(14, 1, "This account already exists.");
	return -1;
}

static int
check_user_callback(void *pass, int argc, char **argv, char **colname) {
	char *bpass = NULL;
	colname = NULL;

	if (argc != 2)
		return -1;

	bpass = argv[1];
	if (bpass == NULL ||
	    strncmp(pass, bpass, strlen(pass)) != 0 ||
	    strlen(bpass) != strlen(pass)) {
		scrmsg(14, 1, "No match");
		return -1;
	}

	session.logged = 1;
	return 0;
}

static int
sqlite_cmd(char *query, int (*callback)(void *, int, char **, char **), void *fp) {
	sqlite3 *db;
	char *errmsg;
	int ret;
	
	/* XXX: It can be usefull to see why we can't open,
		rather than exiting */
	if (sqlite3_open(CRLSERVER_SQLITE_DB, &db) != SQLITE_OK) {
		sqlite3_free(query);
		clean_up(1, "sqlite3_init");
	}

	(void)sqlite3_busy_timeout(db, 10000);
	(void)sqlite3_extended_result_codes(db, 1);
	ret = sqlite3_exec(db, query, callback, fp, &errmsg);

	sqlite3_free(query);
	sqlite3_close(db);
	if (ret != SQLITE_OK) {
		logmsg("Error %i: %s\n", ret, errmsg);
		sqlite3_free(errmsg);
		return ret;
	}
	return 0;
}

void
sqlite_init(void) {
	char *query = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS users \
		(id INTEGER PRIMARY KEY, \
		 name TEXT, \
		 email TEXT, \
		 password TEXT);");

	if (query == NULL)
		clean_up(1, "sqlite_init");

	sqlite_cmd(query, NULL, NULL);
}

void
sqlite_insert(const char *name, const char *email, const char *password) {
	char *query = sqlite3_mprintf("INSERT INTO users \
		(name, email, password) VALUES \
		 ('%q', '%q', '%q');", name, email, password);

	logmsg("sqlite_insert %s %s %s\n", name, email, password);
	if (query == NULL)
		clean_up(1, "sqlite_insert");

	sqlite_cmd(query, NULL, NULL);
}

void
sqlite_update(unsigned int id, const char *name, const char *email, const char *password) {
	char *query = sqlite3_mprintf("UPDATE users \
		SET name='%q', email='%q', password='%q' \
		WHERE id=%i;", name, email, password, id);

	logmsg("sqlite_update %s %s %s for id %i\n", name, email, password, id);
	if (query == NULL)
		clean_up(1, "sqlite_update");

	sqlite_cmd(query, NULL, NULL);
}

int
do_user_exist(const char *name) {
	char *query = sqlite3_mprintf("SELECT * FROM users \
		WHERE name LIKE \"%s\" LIMIT 1;", name);

	logmsg("do_user_exists %s\n", name);
	if (query == NULL)
		clean_up(1, "do_user_exist");

	return sqlite_cmd(query, &user_exist_callback, NULL);
}

int
sqlite_check_user(const char *name, char *pass) {
	char *query = sqlite3_mprintf("SELECT name, password FROM users \
		WHERE name LIKE \"%s\" LIMIT 1;", name);
	int ret = 0;

	logmsg("sqlite_check_user %s\n", name);
	if (query == NULL)
		clean_up(1, "sqlite_check_user");

	ret = sqlite_cmd(query, &check_user_callback, pass);
	return ret;
}
