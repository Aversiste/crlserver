/*	$OpenBSD: log.c,v 1.8 2007/08/22 21:04:30 ckuethe Exp $ */

/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <curses.h>

#include "crlserver.h"
#include "pathnames.h"
#include "log.h"

static unsigned int debug;

static void
graceful_exit(int eval) {
	(void)move(DROWS-1, 0);
	(void)refresh();
	endwin();
	exit(eval);
}

static void
vlog(int pri, const char *fmt, va_list ap) {
	char	*nfmt;
	FILE	*f;
	const char *log_path;

	if (options.o_log != NULL)
		log_path = options.o_log;
	else
		log_path = CRLSERVER_PLAYGROUND"/"CRLSERVER_LOG_FILE;
	f = fopen(log_path, "a+");
	if (f == NULL)
		return;

	/* best effort in out of mem situations */
	if (asprintf(&nfmt, "%i: %s\n", pri, fmt) == -1) {
		vfprintf(f, fmt, ap);
		fprintf(f, "\n");
	} else {
		vfprintf(f, nfmt, ap);
		free(nfmt);
	}
	fflush(f);
}

static void
flog(int pri, const char *fmt, ...) {
	va_list	ap;

	va_start(ap, fmt);
	vlog(pri, fmt, ap);
	va_end(ap);
}

void
log_init(void) {
	extern char *__progname;

	if (debug == 0)
		openlog(__progname, LOG_PID | LOG_NDELAY, LOG_USER);

	tzset();
}

void
log_err(int eval, const char *emsg, ...) {
	if (emsg == NULL)
		flog(LOG_ERR, "error: %s", strerror(errno));
	else
		if (errno)
			flog(LOG_ERR, "error: %s: %s",
			    emsg, strerror(errno));
		else
			flog(LOG_ERR, "error: %s", emsg);

	graceful_exit(eval);
}

void
log_errx(int eval, const char *emsg) {
	errno = 0;
	log_err(eval, emsg);
}

void
log_warn(const char *emsg, ...) {
	char	*nfmt;
	va_list	 ap;

	/* best effort to even work in out of memory situations */
	if (emsg == NULL)
		flog(LOG_WARNING, "%s", strerror(errno));
	else {
		va_start(ap, emsg);

		if (asprintf(&nfmt, "%s: %s", emsg, strerror(errno)) == -1) {
			/* we tried it... */
			vlog(LOG_WARNING, emsg, ap);
			flog(LOG_ERR, "%s", strerror(errno));
		} else {
			vlog(LOG_WARNING, nfmt, ap);
			free(nfmt);
		}
		va_end(ap);
	}
}

void
log_warnx(const char *emsg, ...) {
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_WARNING, emsg, ap);
	va_end(ap);
}

void
log_notice(const char *emsg, ...) {
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_NOTICE, emsg, ap);
	va_end(ap);
}
void
log_info(const char *emsg, ...) {
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_INFO, emsg, ap);
	va_end(ap);
}

void
log_debug(const char *emsg, ...) {
	va_list	 ap;

	if (debug == 1) {
		va_start(ap, emsg);
		vlog(LOG_DEBUG, emsg, ap);
		va_end(ap);
	}
}

void
log_screen(int y, int x, const char *msg) {
	struct timeval t;

	t.tv_sec = 1;
	t.tv_usec = 5;
	(void)mvaddstr(y, x, msg);
	(void)refresh();
	(void)select(0, NULL, NULL, NULL, &t);
}

