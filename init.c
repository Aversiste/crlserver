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
init_playground_rcfiles(const char *username) {
	struct dirent* dp;
	const char *misc = CRLSERVER_CONFIG_DIR"/misc";
	DIR* dir = opendir(misc);

	if (dir == NULL)
		return -1;

	while ((dp = readdir(dir)) != NULL) {
		char in_path[MAXPATHLEN];
		char out_path[MAXPATHLEN];
		char buf[1024];
		FILE *ifd, *ofd;

		if (dp->d_name[0] == '.')
			continue;

		/* in_path: game configuration file */
		(void)memset(in_path, 0, sizeof in_path);
		(void)snprintf(in_path, sizeof in_path,
		    "%s/%s\n", misc, dp->d_name);

		/* out_path: user fake home directory */
		(void)memset(out_path, 0, sizeof out_path);
		(void)snprintf(out_path, sizeof out_path,
		    "%s/userdata/%c/%s/.%s\n",
		    CRLSERVER_PLAYGROUND, username[0], username,
		    dp->d_name);
		log_info("out_path: [%s]", out_path);

		if ((ifd = fopen(in_path, "r")) == NULL)
			continue;
		if ((ofd = fopen(out_path, "a+")) == NULL) {
			log_info("fopen: [%s] [%s]", out_path, strerror(errno));
			(void)fclose(ifd);
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
init_playground_dir(const char *player_name) {
	char playground[MAXPATHLEN] = CRLSERVER_PLAYGROUND"/userdata";

	if (player_name == NULL)
		return -1;

	(void)strlcat(playground, "/", sizeof playground);
	(void)strncat(playground, player_name, 1);
	(void)mkdir(playground, 0744);

	(void)strlcat(playground, "/", sizeof playground);
	(void)strlcat(playground, player_name, sizeof playground);
	(void)mkdir(playground, 0744);

	if (access(playground, F_OK) == -1)
		return -1;

	return 0;
}

void
init(void) {
	heed_signals();

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
		(void)endwin();
		(void)curs_set(1);
		(void)fprintf(stderr, "must be displayed on 24 x 80 screen "
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

