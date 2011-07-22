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
#include <sys/stat.h>
#include <sys/queue.h>
#include <sysexits.h>
#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>
#include <unistd.h>

#include "pathnames.h"
#include "games.h"

#define MAXNAMLEN       255
#define FPARSELN(x)	fparseln((x), NULL, NULL, NULL, FPARSELN_UNESCALL)

static int
file_size(const char *path) {
	struct stat s;
	(void)stat(path, &s);
	return (int)s.st_size;
}

static struct games_list*
parse(FILE *fd) {
	char *b;
	struct games_list *gl = calloc(1, sizeof *gl);

	if (gl == NULL)
		return NULL;

	while ((b = FPARSELN(fd)) != NULL) {
		char *key = strtok(b, "=");
		char *value = strtok(NULL, "=");

		if (strncmp("name", key, 4) == 0 && value != NULL)
			gl->name = strdup(value);
		else if (strncmp("longname", key, 8) == 0 && value != NULL)
			gl->lname = strdup(value);
		else if (strncmp("version", key, 7) == 0 && value != NULL)
			gl->version = strdup(value);
		else if (strncmp("description", key, 10) == 0 && value != NULL)
			gl->desc= strdup(value);
		else if (strncmp("path", key, 4) == 0 && value != NULL)
			gl->path = strdup(value);
		else if (strncmp("params", key, 6) == 0 && value != NULL)
			gl->params = strdup(value);
		else if (strncmp("env", key, 3) == 0 && value != NULL)
			gl->env = strdup(value);
		else
			warnx("bad line: %s", b);
		free(b);
	}
	return gl;
}

int
init_games(struct games_list_head *gl_head) {
	DIR* dir = opendir(GAMES_DIR);
	struct dirent* dp;

	if (dir == NULL)
		err(EX_IOERR, "Can't open %s\n", GAMES_DIR);

	while ((dp = readdir(dir)) != NULL) {
		char buf[ (MAXNAMLEN + 1) * 2 ];
		FILE* fd;
		struct games_list *glp;

		if (dp->d_type != DT_REG && dp->d_type != DT_LNK)
			continue;

		(void)snprintf(buf, strlen(GAMES_DIR) + dp->d_namlen + 2, 
				"%s/%s\n", GAMES_DIR, dp->d_name);
		if (file_size(buf) == 0)	
			continue;

		fd = fopen(buf, "r");
		if (fd == NULL) {
			perror("get_games: ");
			continue;
		}

		warnx("%s:", buf);
		glp = parse(fd);
		if (glp != NULL)
			SLIST_INSERT_HEAD(gl_head, glp, gls);

		(void)fclose(fd);
	}

	(void)closedir(dir);
	return 0;
}

void
release_games(struct games_list_head *gl_head) {
	struct games_list *glp;
	while (!SLIST_EMPTY(gl_head)) {
		glp = SLIST_FIRST(gl_head);
		SLIST_REMOVE_HEAD(gl_head, gls);

		free(glp->name);
		free(glp->lname);
		free(glp->version);
		free(glp->desc);
		free(glp->path);
		free(glp->params);
		free(glp->env);
		free(glp);
	}
}
