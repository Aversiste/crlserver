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

#ifdef __Linux__
# include <bsd/string.h>
#endif

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <curses.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "crlserver.h"
#include "db.h"
#include "pathnames.h"
#include "log.h"

int
init_playground_conffile(const char *path) {
	char buf[1024];
	int fd;

	(void)path;
	snprintf(buf, sizeof buf, "%s/%s", session.home, "crlserver.conf");
	fd = open(buf, O_RDONLY);
	/* YADA YADA YADA */
	return -1;
}

int
init_playground_rcfiles(const char *path) {
	struct dirent* dp;
	const char *misc = CRLSERVER_CONFIG_DIR"/misc";
	DIR* dir = opendir(misc);

	if (dir == NULL)
		return -1;

	while ((dp = readdir(dir)) != NULL) {
		char in_path[ MAXPATHLEN ];
		char out_path[ MAXPATHLEN ];
		char buf[ 1024 ];
		FILE *ifd, *ofd;

		if (dp->d_name[0] == '.')
			continue;

		(void)snprintf(in_path, strlen(misc) + strlen(dp->d_name) + 2,
				"%s/%s\n", misc, dp->d_name);
		(void)snprintf(out_path, strlen(path) + strlen(dp->d_name) + 3,
				"%s/.%s\n", path, dp->d_name);

		ifd = fopen(in_path, "r");
		ofd = fopen(out_path, "a+");
		if (ifd == NULL || ofd == NULL) {
			(void)fclose(ifd);
			(void)fclose(ofd);
			continue;
		}
		
		while (feof(ifd) == 0) {
			size_t s;
			(void)memset(buf, '\0', 1024);
			s = fread(buf, 1, 1024, ifd);
			(void)fwrite(buf, 1, s, ofd);
		}
		if (fclose(ifd) != 0)
			log_notice("%s: fclose(%s)", __func__, in_path);
		if (fclose(ofd) != 0)
			log_notice("%s: fclose(%s)", __func__, out_path);
	}
	if (closedir(dir) == -1)
		log_notice("%s: closedir(%s)", __func__, misc);
	return 0;
}

int
init_session(const char *name) {
	char path[MAXPATHLEN];

	session.name = strdup(name);
	if (session.name == NULL)
		log_err(1, "%s:", __func__);

	(void)memset(path, 0, sizeof path);
	(void)snprintf(path, sizeof path, "%s/%c/%s",
		 CRLSERVER_PLAYGROUND"/userdata",
		 session.name[0], session.name);

	session.home = strdup(path);
	if (session.home == NULL)
		log_err(1, "%s:", __func__);
	if (access(session.home, F_OK) != 0) {
		free(session.home);
		free(session.name);
		return -1;
	}

	return 0;
}

char *
init_playground_dir(const char *player_name) {
	char *p;
	char playground[MAXPATHLEN] = CRLSERVER_PLAYGROUND"/userdata";

	if (player_name == NULL)
		return NULL;

	(void)strlcat(playground, "/", sizeof playground);
	(void)strncat(playground, player_name, 1);
	(void)mkdir(playground, 0744);

	(void)strlcat(playground, "/", sizeof playground);
	(void)strlcat(playground, player_name, sizeof playground);
	(void)mkdir(playground, 0744);

	if (access(playground, F_OK) == -1)
		return NULL;

	p = playground;
	return p;
}

void
init(void) {
	heed_signals();

	/* First init the DB, if it fails we don't need to go further */
	const char *db_path;
	
	if (options.o_db != NULL)
		db_path = options.o_db;
	else
		db_path = CRLSERVER_PLAYGROUND"/"CRLSERVER_DATABASE;

	if (db_check(db_path) == -1)
		db_init(db_path);
	db_open(db_path);

	/* Curses initialization */
	(void)initscr();
	if (has_colors() == TRUE)
		(void)start_color();
	(void)curs_set(0);
	(void)cbreak();
	(void)noecho();
	(void)nonl();
	(void)keypad(stdscr, TRUE);

	/*
	 * A lot of games ask 24x80, so check it now,
	 * but don't care of later window resizing.
	 */
	if ((LINES < DROWS) || (COLS < DCOLS)) {
		(void)move(DROWS - 1, 0);
		(void)refresh();
		endwin();
		fprintf(stderr, "must be displayed on 24 x 80 screen "
		  "(or larger)");
		exit(1);
	}

	/* Initialize default directories */
	if (access(CRLSERVER_PLAYGROUND, F_OK) != 0) {
		if (mkdir(CRLSERVER_PLAYGROUND, S_IRWXU) == -1) {
			mvprintw(1, 1, "You need to mkdir %s\n",
			    CRLSERVER_PLAYGROUND);
			getch();
			byebye(1);
		}
	}
	if (access(CRLSERVER_PLAYGROUND"/userdata", F_OK) != 0) {
		if (mkdir(CRLSERVER_PLAYGROUND"/userdata", S_IRWXU) == -1) {
			mvprintw(2, 1, "You need to mkdir %s\n",
			    CRLSERVER_PLAYGROUND"/userdata");
			getch();
			byebye(1);
		}
	}

	session.logged = 0;
	session.name = NULL;
	session.home = NULL;
}

//void
//free_env(void) {
//	unsigned int i = 1; /* The env array has one hard coded field */
//
//	for (; i < CRLSERVER_MAX_ENV_LENGTH && session.env[i] != NULL; ++i) {
//		free(session.env[i]);
//	}
//}

