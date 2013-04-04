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

#include <sys/param.h>

#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "crlserver.h"

int
isokay(int c) {
	if (isalnum(c) || c == ' ' || c == '-' || c == '_')
		return 1;
	return 0;
}

void
rtrim(char *s) {
	char *c;

	c = strchr(s, ' ');
	if (c == NULL)
		return;
	for (; *c != '\0' && *(c+1) != '\0'; ++c)
		if (*c == ' ' && *(c + 1) == ' ')
			*c = '\0';
}

void
ltrim(char **s) {
	int i = 0;

	for (; (*s)[i] == ' ' && (*s)[i] != '\0'; ++i);
	*s = (*s)+i;
}

void
trim(char **s) {
	ltrim(s);
	rtrim(*s);
}

void
byebye(int unused) {
	(void)unused;
	ignore_signals();
	/* TODO: Ask user if he really wants to quit */
	endwin();
	exit(1);
}

void
heed_signals(void) {
	signal(SIGINT, byebye);
	signal(SIGQUIT, byebye);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}

void
ignore_signals(void) {
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}

char *
escape_space(char *s) {
	char buf[CRLS_MAXNAMELEN];
	char *ret;
	int i, j;

	memset(buf, '\0', sizeof buf);
	for (i = 0, j = 0; s[i] != '\0'; ++i, ++j) {
		if (s[i] == ' ') {
			buf[j] = '\\';
			++j;
			buf[j] = s[i];
		}
		else
			buf[j] = s[i];
	}
	ret = strdup(buf);
	return ret;
}

int
has_config_file(struct list *lp) {
	char path[MAXPATHLEN];
	
	(void)memset(path, '\0', MAXPATHLEN);
	(void)snprintf(path, sizeof path, "%s/.%src", session.home, lp->l_name);
	if (access(path, R_OK) == 0)
		return 0;
	return -1;
}


