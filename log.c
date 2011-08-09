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
#include <curses.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "crlserver.h"
#include "init.h"
#include "pathnames.h"

extern char *__progname;

static void
graceful_exit(int eval, const char *fmt, va_list ap, int flag) {
	int	sverrno;

	sverrno = errno;

	(void)move(DROWS-1, 0);
	(void)refresh();
	end_window();

	(void)fprintf(stderr, "%s: ", __progname);
	if (fmt != NULL) {
		(void)vfprintf(stderr, fmt, ap);
		if (flag == 1)
			(void)fprintf(stderr, ": ");
	}
	if (flag == 1)
		(void)fprintf(stderr, "%s\n", strerror(sverrno));
	exit(eval);
}

void
clean_up(int eval, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	graceful_exit(eval, fmt, ap, 1);
	va_end(ap);
}

void
clean_upx(int eval, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	graceful_exit(eval, fmt, ap, 0);
	va_end(ap);
}

/* fast clean_up, in case of memory error */
void
fclean_up(const char *estr) {
	(void)move(DROWS-1, 0);
	(void)refresh();
	end_window();
	(void)printf("\n%s\n", estr);
	exit(EX_SOFTWARE);
}

void
logmsg(const char *fmt, ...) {
	va_list ap;
	FILE *fd;

	fd = fopen(CRLSERVER_LOG_FILE, "a+");
	if (fd == NULL)
		clean_up(1, "fopen");
	
	va_start(ap, fmt);
	vfprintf(fd, fmt, ap);
	va_end(ap);
	(void)fclose(fd);
}

void
scrmsg(int y, int x, const char *msg) {
	mvaddstr(y, x, msg);
	refresh();
	sleep(2);
}
