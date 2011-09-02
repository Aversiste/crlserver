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

#ifdef __OpenBSD__
# include <util.h>
#elif __Linux__
# define _XOPEN_SOURCE
# define _BSD_SOURCE
# include <bsd/bsd.h>
# include "compat/util.h"
#endif

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <curses.h>
#include <err.h>
#include <form.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aux.h"
#include "crlserver.h"
#include "conf.h"
#include "db.h"
#include "init.h"
#include "log.h"
#include "menus.h"
#include "pathnames.h"
#include "session.h"

void
print_file(const char *path) {
	FILE *fd = fopen(path, "r");
	int y = 0;
	char *buf = NULL;
	const char sep[3] = {'\\', '\\', 0};

	if (fd == NULL)
		clean_up(1, "Error with file %s\n", path);

	(void)erase();
	while ((buf = fparseln(fd, NULL, NULL,
	  sep, FPARSELN_UNESCALL)) != NULL) {
		(void)mvprintw(y, 1, buf);
		free(buf);
		++y;
	}
	if (session.logged == 1)
		mvprintw(LINES - 1, 1, "Logged in as: %s", session.name);
	refresh();
	(void)fclose(fd);
}

static void
form_release(FORM *form) {
	FIELD **fields = form_fields(form);
	int fmax = field_count(form);
	int i = 0;

	if (fmax == ERR)
		return;
	(void)unpost_form(form);
	(void)free_form(form);
	for (; i < fmax; ++i)
		(void)free_field(fields[i]);
}

static void
editors_menu(games_list *glp) {
	char path[MAXPATHLEN];
	char* params[] = {"", ""};
	struct list *lp;
	int status = 0, ch = 0;
	pid_t pid = 0;

	memset(path, '\0', MAXPATHLEN);
	(void)strlcpy(path, session.home, sizeof path);
	(void)strlcat(path, "/.", sizeof path);
	(void)strlcat(path, glp->name, sizeof path);
	(void)strlcat(path, "rc", sizeof path);
	params[1] = path;
	do {
		int i = 6;
		print_file(CRLSERVER_MENUS_DIR"/banner.txt");
		SLIST_FOREACH(lp, session.list[1], ls) {
			mvprintw(i, 1, "%s) Edit with %s (%s %s)", lp->key, lp->name,
					lp->lname, lp->version);
			++i;
		}
		(void)mvaddstr(4, 1, "q) Quit");
		(void)refresh();
		ch = getch();
		if (ch == 'q')
			break;
		SLIST_FOREACH(lp, session.list[1], ls) {
			if (ch == lp->key[0]) {
				(void)clear();
				(void)refresh();
				(void)endwin();
				pid = fork();
				if (pid < 0)
					clean_up(1, "fork");
				else if (pid == 0) {
					params[0] = lp->path;
					fprintf(stderr, "%s %s\n", params[0], params[1]);
					execve(lp->path, params, session.env);
					clean_up(1, "execve error");
				}
				else
					waitpid(pid, &status, 0);
				break;
			}
		}
	} while (1);	
}

static void
games_menu(games_list *glp) {
	int status = 0, ch = 0;
	pid_t pid = 0;
	int configurable = has_config_file(glp);

	do {
		switch (ch) {
		case 'p':
		case 'P':
			(void)clear();
			(void)refresh();
			(void)endwin();
			pid = fork();
			if (pid < 0)
				clean_up(1, "fork");
			else if (pid == 0) {
				execve(glp->path, glp->params, session.env);
				clean_up(1, "execve error");
			}
			else
				waitpid(pid, &status, 0);
			break;
		case 'e':
		case 'E':
			if (configurable == 0)
				editors_menu(glp);
			break;
		default:
			break;
		}
		print_file(CRLSERVER_MENUS_DIR"/games.txt");
		if (configurable == 0)
			mvprintw(5, 1,"%s", "e) Edit the configuration file");
		mvprintw(7, 1,"%s", glp->desc);
		refresh();
	} while ((ch = getch()) != 'q');
}

static void
user_menu(void) {
	games_list_head gl_head = SLIST_HEAD_INITIALIZER(gl_head);
	editors_list_head el_head = SLIST_HEAD_INITIALIZER(el_head);
	games_list *glp;
	int i, ch = 0;

	load_folder(CRLSERVER_GAMES_DIR, &gl_head);
	load_folder(CRLSERVER_EDITORS_DIR, &el_head);
	session.list[0] = &gl_head;
	session.list[1] = &el_head;

	do {
		i = 6;
		print_file(CRLSERVER_MENUS_DIR"/banner.txt");
		SLIST_FOREACH(glp, &gl_head, ls) {
			mvprintw(i, 1, "%s) %s - %s (%s)", glp->key, glp->name,
					glp->lname, glp->version);
			++i;
		}
		(void)mvaddstr(4, 1, "q) Quit");
		(void)refresh();
		ch = getch();
		if (ch == 'q')
			break;
		SLIST_FOREACH(glp, &gl_head, ls) {
			if (ch == glp->key[0]) {
				games_menu(glp);
				break;
			}
		}
	} while (1);
	list_release(&gl_head);
	list_release(&el_head);
	free(session.name);
	free(session.home);
	free_env();
	session.logged = 0;
}

__inline void
server_info(void) {
	print_file(CRLSERVER_MENUS_DIR"/server_info.txt");
	getch();
}

static void
form_navigation(FORM **form) {
	bool quit = false;
	unsigned int ch = 0;

	(void)curs_set(1); /* Print the cursor */
	(void)move(4, 18); /* This is too arbitrary */
	do {
		ch = getch();
		switch(ch) {
		case KEY_DOWN:
		case '\t':
			(void)form_driver(*form, REQ_NEXT_FIELD);
			(void)form_driver(*form, REQ_END_LINE);
			break;
		case KEY_UP:
			(void)form_driver(*form, REQ_PREV_FIELD);
			(void)form_driver(*form, REQ_END_LINE);
			break;
		case KEY_RIGHT:
			(void)form_driver(*form, REQ_NEXT_CHAR);
			break;
		case KEY_LEFT:
			(void)form_driver(*form, REQ_PREV_CHAR);
			break;
		case KEY_DC:
			(void)form_driver(*form, REQ_DEL_CHAR);
			break;
		case KEY_BACKSPACE:
		case 127:
			(void)form_driver(*form, REQ_DEL_PREV);
			break;
		case KEY_HOME:
			(void)form_driver(*form, REQ_BEG_FIELD);
			break;
		case KEY_END:
			(void)form_driver(*form, REQ_END_FIELD);
			break;
		case KEY_ENTER:
		case '\n':
		case '\r':
			(void)form_driver(*form, REQ_VALIDATION);
			quit = true;		
			break;
		default:
			(void)form_driver(*form, ch);
			break;
		}
	} while(quit == false);
	(void)curs_set(0); /* Remove the cursor */
}

static char *
field_sanitize(const FIELD *field) {
	char *s = field_buffer(field, 0);

	if (s == NULL) {
		scrmsg(14, 1, "A field is missing\n");
		return NULL;
	}
	trim(&s);
	if (strlen(s) < 1) {
		scrmsg(14, 1, "A field is missing\n");
		return NULL;
	}
	return s;
}

/* Return 0 in case of a succesfull login or -1*/
static int
login_menu(void) {
	FIELD *fields[3] = {0, 0, 0};
	FORM  *form = NULL;
	unsigned int i = 0;
	char *user, *pass;

	for (; i < 2; ++i) {
		fields[i] = new_field(1, CRLS_MAXNAMELEN, 4 + i, 18, 0, 0);
		(void)field_opts_off(fields[i], O_AUTOSKIP);
	}
	/* Protect the password */
	(void)field_opts_off(fields[1], O_PUBLIC);

	form = new_form(fields);
	if (form == NULL)
		clean_up(1, "login_menu");
	(void)post_form(form);
	print_file(CRLSERVER_MENUS_DIR"/login.txt");
	(void)refresh();

	form_navigation(&form);
	user = field_sanitize(fields[0]);
	pass = field_sanitize(fields[1]);

	if (user == NULL || pass == NULL)
		goto clean;

	if (db_check_user(user, crypt(pass, "$1")) != 0)
		goto clean;

	/* This flag is set by db_check_user */
	if (session.logged == 0) {
		scrmsg(14, 1, "No match");
		goto clean;
	}

	if (init_session(user) != 0) {
		scrmsg(14, 1, "Error whith your session");
		goto clean;
	}

	(void)form_release(form);
	user_menu();
	return 0;
clean:
	(void)form_release(form);
	return -1;
}

static void
register_menu(void) {
	FIELD *fields[5] = {0, 0, 0, 0, 0};
	FORM  *form = NULL;
	unsigned int i = 0;
	char *user, *email, *pass, *pass2;
	size_t pass_size;

	for (; i < 4; ++i) {
		fields[i] = new_field(1, CRLS_MAXNAMELEN, 4 + i, 18, 0, 0);
		(void)field_opts_off(fields[i], O_AUTOSKIP);
	}

	/* Protect the password fields */
	for (i = 2; fields[i] != NULL; ++i)
		(void)field_opts_off(fields[i], O_PUBLIC);

	form = new_form(fields);
	if (form == NULL)
		clean_up(1, "register_menu");
	(void)post_form(form);
	print_file(CRLSERVER_MENUS_DIR"/register.txt");
	(void)refresh();

	form_navigation(&form);
	
	/* Get the input with trimed whitespaces */
	user = field_sanitize(fields[0]);
	email = field_sanitize(fields[1]);
	pass = field_sanitize(fields[2]);
	pass2 = field_sanitize(fields[3]);

	if (user == NULL || email == NULL || pass == NULL || pass2 == NULL)
		goto clean;
	pass_size = strlen(pass);
	if (strchr(email, '@') == NULL) {
		scrmsg(14, 1, "Put a valid email please.");
		goto clean;
	}
	if (strncmp(pass, pass2, pass_size) != 0) {
		scrmsg(14, 1, "Passwords don't match!");
		goto clean;
	}
	for (i = 0; user[i] != '\0'; ++i) {
		if (isokay(user[i]) == 0) {
			scrmsg(14, 1, "Only ascii alpha numerics in the username");
			goto clean;
		}
	}

	if (init_playground_dir(user) == -1) {
		scrmsg(14, 1, "Error while creating your playground");
		goto clean;
	}

	/* This function actually print is own error message */
	if (do_user_exist(user) == 0)
		db_insert(user, email, crypt(pass, "$1"));

clean:
	(void)form_release(form);
}

void
menus(void) {
	unsigned char c = 0;

	do {
		switch (c) {
		case 'l':
		case 'L':
			(void)login_menu();
		break;
		case 'r':
		case 'R':
			register_menu();
			break;
		case 's':
		case 'S':
			server_info();
			break;
		case 'q':
		case 'Q':
			fclean_up("Good Bye");
			break;
		default:
			break;
		}
		print_file(CRLSERVER_MENUS_DIR"/general.txt");
	} while ((c = getch()) != 'q');
}
