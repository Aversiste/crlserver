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
	struct sqlite3_stmt *pStmt;
	int ret;
	
	pDb = malloc(sizeof(pDb));
	if (pDb == NULL)
		err(1, "sqlite_init: allocation error");
	if (sqlite3_open(RL_SQLITE_DB, &pDb) != SQLITE_OK)
		err(1, "sqlite3_open");

	ret = sqlite3_prepare_v2(pDb, RL_SQLITE_CREATE, -1, &pStmt, NULL);
	if (pStmt == NULL || ret != SQLITE_OK)
		err(1, "sqlite3_prepare_v2");

	if (sqlite3_step(pStmt) == SQLITE_ERROR)
		err(1, "sqlite3_step");
	sqlite3_finalize(pStmt);
	sqlite3_close(pDb);
}

/*
char *qbuf;
qbuf = sqlite3_mprintf("insert into users (username, email, password) values ('%q', '%q', '%q')", me->username, me->email, me->password);
qbuf = sqlite3_mprintf("update users set username='%q', email='%q', password='%q' where id=%i", me->username, me->email, me->password, me->id);
sqlite3_busy_timeout(db, 10000);
ret = sqlite3_exec(db, qbuf, NULL, NULL, &errmsg);
sqlite3_free(qbuf);
if (ret != SQLITE_OK) {
	sqlite3_close(db);
	 error message 
}
sqlite3_close(db);
*/
