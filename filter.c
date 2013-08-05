/*
 * Copyright (c) 2013 Tristan Le Guern <leguern AT medu.se>
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
#include <sys/wait.h>

#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

#include "pathnames.h"

extern char **environ;

int
filter_apply(char * const path, char * const user) {
	int status, err;
	pid_t pid;

	status = 0;
	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Fork error\n");
	} else if (pid == 0) {
		char * const argv[] = {path, user, NULL};
		execve(path, argv, environ);
		perror(NULL);
	} else {
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			err = WEXITSTATUS(status);
			return err;
		}
	}
	return -1;
}

int
filter_match(char * const user) {
	struct dirent* dp;
	DIR* dir = opendir(_PATH_FILTERS);
	char buf[1024];
	int err = 0;
 
	if (dir == NULL)
		return -1;

	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_name[0] == '.')
			continue;
		snprintf(buf, sizeof buf, "%s/%s", _PATH_FILTERS, dp->d_name);
		if ((err = filter_apply(buf, user)) != 0)
			break;
	}
	(void)closedir(dir);
	return err;
}

