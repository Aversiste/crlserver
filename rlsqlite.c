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
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <sqlite3.h>
#include "rlsqlite.h"

void
sqlite_init(void) {
	sqlite3 *pDb;
	char *qbuf;
	int ret;
	
	pDb = malloc(sizeof(pDb));
	if (pDb == NULL)
		err(1, "sqlite_init: allocation error");
	if (sqlite3_open(RL_SQLITE_DB, &pDb) != SQLITE_OK)
		err(1, "sqlite3_open");

	qbuf = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS users \
		(id INTEGER PRIMARY KEY AUTOINCREMENT, \
		 name TEXT, \
		 email TEXT, \
		 password TEXT);");

	sqlite3_busy_timeout(pDb, 10000);
	ret = sqlite3_exec(pDb, qbuf, NULL, NULL, NULL);

	sqlite3_free(qbuf);
	if (ret != SQLITE_OK) {
		sqlite3_close(pDb);
		errx(1, "sqlite");
	}
	sqlite3_close(pDb);
}

void
sqlite_insert(const char *name, const char *email, const char *password) {
	sqlite3 *pDb;
	char *qbuf;
	int ret;
	
	pDb = malloc(sizeof(pDb));
	if (pDb == NULL)
		err(1, "sqlite_init: allocation error");
	if (sqlite3_open(RL_SQLITE_DB, &pDb) != SQLITE_OK)
		err(1, "sqlite3_open");

	qbuf = sqlite3_mprintf("INSERT INTO users \
		(name, email, password) VALUES \
		 (%q, %q, %q);", name, email, password);

	sqlite3_busy_timeout(pDb, 10000);
	ret = sqlite3_exec(pDb, qbuf, NULL, NULL, NULL);

	sqlite3_free(qbuf);
	if (ret != SQLITE_OK) {
		sqlite3_close(pDb);
		errx(1, "sqlite");
	}
	sqlite3_close(pDb);
}
/*
qbuf = sqlite3_mprintf("update users set username='%q', email='%q', password='%q' where id=%i", me->username, me->email, me->password, me->id);
*/
