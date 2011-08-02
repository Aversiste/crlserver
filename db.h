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

#ifndef CRLSERVER_SQLITE_H_
#define CRLSERVER_SQLITE_H_

#ifdef SQLITE_FLAVOR

# define db_init   sqlite_init
# define db_insert sqlite_insert
# define db_update sqlite_update
# define db_check_user sqlite_check_user

# define RL_SQLITE_DB "/tmp/crlserver.db"
void sqlite_init(void);
void sqlite_insert(const char *, const char *, const char *);
void sqlite_update(unsigned int, const char *, const char *, const char *);
int do_user_exist(const char *);
int sqlite_check_user(const char *, char *);

#endif /* SQLITE_FLAVOR */

#endif
