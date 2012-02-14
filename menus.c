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
# define _XOPEN_SOURCE
# include <bsd/string.h>
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

#include "crlserver.h"
#include "db.h"
#include "pathnames.h"

int
has_config_file(games_list *glp) {
	char path[MAXPATHLEN];
	
	(void)memset(path, '\0', MAXPATHLEN);
	(void)strlcpy(path, session.home, sizeof path);
	(void)strlcat(path, "/.", sizeof path);
	(void)strlcat(path, glp->name, sizeof path);
	(void)strlcat(path, "rc", sizeof path);
	if (access(path, R_OK) == 0)
		return 0;
	return -1;
}

void
print_file(const char *path) {
	FILE *fd = fopen(path, "r");
	int y = 0;
	char buf[80];

	if (fd == NULL)
		clean_up(1, "Error with file %s\n", path);

	(void)erase();
	memset(buf, '\0', sizeof buf);
	while (fgets(buf, sizeof buf, fd) != NULL) {
		(void)mvprintw(y, 1, buf);
		memset(buf, '\0', sizeof buf);
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
	char rc_path[MAXPATHLEN];
	struct list *lp;
	int status = 0, ch = 0;
	pid_t pid = 0;

	(void)memset(rc_path, '\0', MAXPATHLEN);
	(void)strlcpy(rc_path, session.home, sizeof rc_path);
	(void)strlcat(rc_path, "/.", sizeof rc_path);
	(void)strlcat(rc_path, glp->name, sizeof rc_path);
	(void)strlcat(rc_path, "rc", sizeof rc_path);
	do {
		int i = 6;
		print_file(CRLSERVER_CONFIG_DIR"/menus/banner.txt");
		SLIST_FOREACH(lp, &elh, ls) {
			(void)mvprintw(i, 1, "%c) Edit with %s (%s %s)",
			lp->key, lp->name, lp->lname, lp->version);
			++i;
		}
		(void)mvaddstr(4, 1, "q) Quit");
		(void)refresh();
		ch = getch();
		if (ch == 'q')
			break;
		SLIST_FOREACH(lp, &elh, ls) {
			if (ch == lp->key) {
				(void)clear();
				(void)refresh();
				(void)endwin();

				if (lp->params[1] != NULL)
					free(lp->params[1]);
				/*
				 * XXX: We should check for the next free place
				 * rather than use 1.
				 */
				lp->params[1] = strdup(rc_path);
				if (lp->params[1] == NULL)
					clean_upx(1, "memory error");

				pid = fork();
				if (pid < 0)
					clean_up(1, "fork");
				else if (pid == 0) {
					execve(lp->path, lp->params, lp->env);
					logmsg("%s: %s %s\n", session.name, lp->path, lp->params[1]);
					clean_up(1, "execve error");
				}
				else
					waitpid(pid, &status, 0);
					(void)clear();
					(void)refresh();
				break;

				free(lp->params[1]);
				lp->params = NULL;
			}
		}
		lp = NULL;
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
				execve(glp->path, glp->params, glp->env);
				logmsg("%s: %s %s\n", session.name, glp->path, glp->params[1]);
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
		print_file(CRLSERVER_CONFIG_DIR"/menus/games.txt");
		if (configurable == 0)
			mvprintw(5, 1,"%s", "e) Edit the configuration file");
		mvprintw(7, 1,"%s", glp->desc);
		refresh();
	} while ((ch = getch()) != 'q');
}

static void
user_menu(void) {
	do {
		games_list *glp;
		int i = 6;
		int ch = 0;

		print_file(CRLSERVER_CONFIG_DIR"/menus/banner.txt");
		SLIST_FOREACH(glp, &glh, ls) {
			mvprintw(i, 1, "%c) %s - %s (%s)", glp->key, glp->name,
					glp->lname, glp->version);
			++i;
		}
		(void)mvaddstr(4, 1, "q) Quit");
		(void)refresh();
		ch = getch();
		if (ch == 'q')
			break;
		SLIST_FOREACH(glp, &glh, ls) {
			if (ch == glp->key) {
				games_menu(glp);
				break;
			}
		}
	} while (1);
	session.logged = 0;
	free(session.name);
	free(session.home);
	/*
	free_env();
	*/
}

__inline void
server_info(void) {
	print_file(CRLSERVER_CONFIG_DIR"/menus/server_info.txt");
	getch();
}

static int
form_navigation(FORM **form) {
	int quit = 0;
	unsigned int ch = 0;

	(void)curs_set(1); /* Print the cursor */
	(void)move(4, 18); /* XXX: This is too arbitrary */
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
		case 127: /* An other backspace */
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
			quit = 1;
			break;
		case 27: /* Escape */
			quit = -1;
			break;
		default:
			(void)form_driver(*form, ch);
			break;
		}
	} while(quit == 0);
	(void)curs_set(0); /* Remove the cursor */
	return quit;
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
	char *user, *pass; /* TODO: remove the password from memory */

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
	print_file(CRLSERVER_CONFIG_DIR"/menus/login.txt");
	(void)refresh();

	if (form_navigation(&form) == -1)
		goto clean;

	user = field_sanitize(fields[0]);
	pass = field_sanitize(fields[1]);

	if (user == NULL || pass == NULL)
		goto clean;

	if (db_user_auth(user, crypt(pass, "$1")) != 0)
		goto clean;
	else
		session.logged = 1;

	if (init_session(user) != 0) {
		scrmsg(14, 1, "Error whith your session");
		goto clean;
	}

	/* Successful login! log it */
	/* logmsg("%s successfully logged in\n", user);*/

	/* parse the configuration files */
	config();
	list_finalize(&glh);
	list_finalize(&elh);

	(void)form_release(form);
	user_menu();

	list_release(&glh);
	list_release(&elh);

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
	print_file(CRLSERVER_CONFIG_DIR"/menus/register.txt");
	(void)refresh();

	if (form_navigation(&form) == -1)
		goto clean;
	
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
	db_user_add(user, email, crypt(pass, "$1"));

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
			return;
		default:
			break;
		}
		print_file(CRLSERVER_CONFIG_DIR"/menus/general.txt");
	} while ((c = getch()) != 'q');
}

