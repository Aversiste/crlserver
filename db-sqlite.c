/*
 * Copyright (c) 2011 Tristan Le Guern <leguern AT medu DOT se>
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
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>

#include "crlserver.h"
#include "db.h"
#include "log.h"

sqlite3 *db_link;

int
db_open(const char *db_path) {
	int sql_res;
	sqlite3 *db;

	/* open the database */
	if (sqlite3_open(db_path, &db) != SQLITE_OK) {
		log_warnx("Can't open %s: %s", db_path, sqlite3_errmsg(db));
		return -1;
	}
	
	/* make database secure */
	if (chmod(db_path, S_IRUSR | S_IWUSR) != 0) {
		log_warnx("Could not make %s file secure: %s",
		db_path, sqlite3_errmsg(db));
	}
	
	sql_res = sqlite3_busy_timeout(db, 2000);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't set busy timout: %s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	db_link = db;
	return 0;
}

int
db_init(const char *db_path) {
	int sql_res;

	sql_res = db_open(db_path);
	if (sql_res < 0)
		return -1;

	/* lock the database */
	sql_res = sqlite3_exec(db_link, "BEGIN TRANSACTION", NULL, NULL, NULL);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't initialise db: %s", sqlite3_errmsg(db_link));
		sqlite3_close(db_link);
		db_link = NULL;
		return -1;
	}

	sql_res = sqlite3_exec(db_link,
	    "CREATE TABLE users ("
	      "name TEXT PRIMARY KEY,"
	      "email TEXT,"
	      "password TEXT);",
	    NULL, NULL, NULL);

	if (sql_res != SQLITE_OK) {
		log_warnx("Can't initialise db: %s", sqlite3_errmsg(db_link));
		sqlite3_close(db_link);
		db_link = NULL;
		return -1;
	}

	/* unlock the database */
	sql_res = sqlite3_exec(db_link, "END TRANSACTION", NULL, NULL, NULL);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't initialise db: %s", sqlite3_errmsg(db_link));
		sqlite3_close(db_link);
		db_link = NULL;
		return -1;
	}

	sqlite3_close(db_link);
	db_link = NULL;
	return 0;
}

int
db_user_add(const char *name, const char *email, const char *password) {
	int sql_res;
	int ret;
	sqlite3_stmt *stmt;
	char *sql = "INSERT INTO users "
	    "(name, email, password) "
	    " VALUES (?, ?, ?)";

	ret = 0;
	sql_res = sqlite3_prepare_v2(db_link, sql, -1, &stmt, NULL);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't prepare sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	/* bind params */
	sql_res = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
	sql_res |= sqlite3_bind_text(stmt, 2, email, -1, SQLITE_TRANSIENT);
	sql_res |= sqlite3_bind_text(stmt, 3, password, -1, SQLITE_TRANSIENT);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't bind sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	sql_res = sqlite3_step(stmt);
	if (sql_res == SQLITE_CONSTRAINT) {
		log_warnx("User '%s' already exists", name);
		ret = -1;
		goto clean;
	} else if (sql_res != SQLITE_DONE) {
		log_warnx("Can't step sql: %s", sqlite3_errmsg(db_link));
		goto clean;
	}

clean:
	sqlite3_finalize(stmt);
	return ret;
}

int
db_user_mod_email(const char *name, const char *email) {
	int sql_res;
	int ret;
	sqlite3_stmt *stmt;
	char *sql = "UPDATE users SET email=? WHERE name=?;";

	ret = 0;
	sql_res = sqlite3_prepare_v2(db_link, sql, -1, &stmt, NULL);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't prepare sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	/* bind params*/
	sql_res = sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
	sql_res |= sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't bind sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	sql_res = sqlite3_step(stmt);
	if (sql_res != SQLITE_DONE) {
		log_warnx("Failed to update %s's email", name);
		ret = -1;
		goto clean;
	}

clean:
	sqlite3_finalize(stmt);
	return ret;
}

int
db_user_mod_password(const char *name, const char *hash) {
	int sql_res;
	int ret;
	sqlite3_stmt *stmt;
	char *sql = "UPDATE users SET password=? WHERE name=?;";

	ret = 0;
	sql_res = sqlite3_prepare_v2(db_link, sql, -1, &stmt, NULL);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't prepare sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	/* bind params */
	sql_res = sqlite3_bind_text(stmt, 1, hash, -1, SQLITE_TRANSIENT);
	sql_res |= sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't bind sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	sql_res = sqlite3_step(stmt);
	if (sql_res != SQLITE_DONE) {
		log_warnx("Failed to update %s's password", name);
		ret = -1;
		goto clean;
	}

clean:
	sqlite3_finalize(stmt);
	return ret;
}

int
db_user_auth(const char *name, const char *hash) {
	int sql_res;
	int ret;
	sqlite3_stmt *stmt;
	char *db_hash;
	char *sql = "SELECT password FROM users WHERE name=?;";

	ret = 0;
	sql_res = sqlite3_prepare_v2(db_link, sql, -1, &stmt, NULL);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't prepare sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	/* bind name */
	sql_res = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
	if (sql_res != SQLITE_OK) {
		log_warnx("Can't bind sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	sql_res = sqlite3_step(stmt);
	if (sql_res == SQLITE_DONE) {
		log_warnx("User %s does not exist", name);
		ret = -1;
		goto clean;
	} else if (sql_res != SQLITE_ROW) {
		log_warnx("Can't step sql: %s", sqlite3_errmsg(db_link));
		ret = -1;
		goto clean;
	}

	db_hash = (char *)sqlite3_column_text(stmt, 0);
	if (strcmp(hash, db_hash) != 0) {
		log_warnx("User %s: authentication failed", name);
		ret = -1;
		goto clean;
	}

clean:
	sqlite3_finalize(stmt);
	return ret;
}

int
db_check(const char *db_path) {
	struct stat st;

	if (stat(db_path, &st) == -1) {
		log_warn("Can't stat %s", db_path);
		return -1;
	}

	if (st.st_size == 0)
		return -1;
	return 0;
}
/*
int
main(void) {
	const char *db_path = "./test.db";

	if (db_check(db_path) == -1)
		db_init(db_path);
	db_open(db_path);

	db_user_add("Cyan Melanie", "@", "toto");
	db_user_mod_email("Cyan Melanie", "cyan@melan.ie");
	db_user_mod_email("Dark Melanie", "cyan@melan.ie");

	db_user_mod_password("Royal Blue Melanie", "password");

	db_user_auth("Royal Blue Melanie", "password");
	return 0;
}
*/
