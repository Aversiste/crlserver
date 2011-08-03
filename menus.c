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
# include "compat/util.h"
#endif

#include <sys/queue.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <curses.h>
#include <err.h>
#include <form.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "crlserver.h"
#include "conf.h"
#include "db.h"
#include "init.h"
#include "log.h"
#include "menus.h"
#include "pathnames.h"
#include "session.h"

#define CRLS_MAXNAMELEN 20

void
print_file(const char *path) {
	FILE *fd = fopen(path, "r");
	int y = 0;
	char *buf;
	const char sep[3] = {'\\', '\\', 0};

	(void)erase();
	if (fd == NULL)
		clean_up(1, "Important file is missing");

	while ((buf = fparseln(fd, NULL, NULL,
	  sep, FPARSELN_UNESCALL)) != NULL) {
		mvprintw(y, 1, buf);
		free(buf);
		++y;
	}
	refresh();
}

static void
form_release(FORM *form) {
	FIELD **fields;
	int i, fmax;

	fields = form_fields(form);
	fmax = field_count(form);
	unpost_form(form);
	free_form(form);
	for (i = 0; i < fmax; ++i)
		free_field(fields[i]);
}

extern char **environ;

static void
games_menu(games_list *glp) {
	int ch = 0;
	int status;
	pid_t pid;
	char **ap, *argv[10];

	argv[0] = glp->path;
	for (ap = &argv[1]; ap < &argv[9] &&
			(*ap = strsep(&glp->params, " \t")) != NULL;) {
		if (strchr(*ap, '%') != NULL)
			*ap = session.name;
		if (**ap != '\0')
			ap++;
	}
	*ap = NULL;

	do {
		switch (ch) {
			case 'p':
			case 'P':
				clear();
				refresh();
				endwin();
				pid = fork();
				if (pid < 0)
					clean_up(1, "fork");
				else if (pid == 0) {
					/* child */
					execve(glp->path, argv, environ);
					logmsg("execve error\n");
					exit(2);
				}
				else /* parent */
					waitpid(pid, &status, 0);
				break;
			case 'e':
			case 'E':
				scrmsg(14, 1, "Edit");
				break;
			default:
				break;
		}
		print_file("menus/games.txt");
	} while ((ch = getch()) != 'q');
}

static void
user_menu(void) {
	size_t	gl_length;
	games_list_head gl_head = SLIST_HEAD_INITIALIZER(gl_head);
	games_list *glp;
	int i, ch = 0;

	load_folder(GAMES_DIR, &gl_head);
	gl_length = list_size((struct list_head*)&gl_head);

	do {
		i = 6;
		print_file("menus/banner.txt");
		SLIST_FOREACH(glp, &gl_head, ls) {
			mvprintw(i, 1, "%s) %s - %s (%s)", glp->key, glp->name,
					glp->lname, glp->version);
			++i;
		}
		mvaddstr(4, 1, "q) Quit");
		refresh();
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
	free(session.name);
	session.logged = 0;
}

__inline void
server_info(void) {
	print_file("menus/server_info.txt");
	getch();
}

static void
form_navigation(FORM **form) {
	bool quit = false;
	unsigned int ch = 0;

	curs_set(1); /* Print the cursor */
	move(4, 18); /* This is too arbitrary */
	do {
		ch = getch();
		switch(ch) {
		case KEY_DOWN:
		case '\t':
			form_driver(*form, REQ_NEXT_FIELD);
			form_driver(*form, REQ_END_LINE);
			break;
		case KEY_UP:
			form_driver(*form, REQ_PREV_FIELD);
			form_driver(*form, REQ_END_LINE);
			break;
		case KEY_RIGHT:
			form_driver(*form, REQ_NEXT_CHAR);
			break;
		case KEY_LEFT:
			form_driver(*form, REQ_PREV_CHAR);
			break;
		case KEY_DC:
			form_driver(*form, REQ_DEL_CHAR);
			break;
		case KEY_BACKSPACE:
		case 127:
			form_driver(*form, REQ_DEL_PREV);
			break;
		case KEY_HOME:
			form_driver(*form, REQ_BEG_FIELD);
			break;
		case KEY_END:
			form_driver(*form, REQ_END_FIELD);
			break;
		case KEY_ENTER:
		case '\n':
		case '\r':
			form_driver(*form, REQ_VALIDATION);
			quit = true;		
			break;
		default:
			form_driver(*form, ch);
			break;
		}
	} while(quit == false);
	curs_set(0); /* Remove the cursor */
}

static char *
field_sanitize(const FIELD *field) {
	char *c, *s;

	s = field_buffer(field, 0);
	if (s == NULL) {
		scrmsg(14, 1, "A field is missing\n");
		return NULL;
	}
	c = strchr(s, ' ');
	if (c != NULL)
		*c = '\0';
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
	FORM  *form;
	unsigned int i;
	char *user, *pass;

	for (i = 0; i < 2; ++i) {
		fields[i] = new_field(1, CRLS_MAXNAMELEN, 4 + i, 18, 0, 0);
		field_opts_off(fields[i], O_AUTOSKIP);
	}
	/* Protect the password */
	field_opts_off(fields[1], O_PUBLIC);

	form = new_form(fields);
	post_form(form);
	print_file("menus/login.txt");
	refresh();

	form_navigation(&form);
	user = field_sanitize(fields[0]);
	pass = field_sanitize(fields[1]);

	if (user == NULL || pass == NULL) {
		form_release(form);
		return -1;
	}

	if (db_check_user(user, crypt(pass, "$1")) != 0) {
		form_release(form);
		return -1;
	}

	/* This flag is set by db_check_user */
	if (session.logged == 0) {
		scrmsg(14, 1, "No match");
		form_release(form);
		return -1;
	}

	session.name = strdup(user);
	form_release(form);
	user_menu();
	return 0;
}

static void
register_menu(void) {
	FIELD *fields[5] = {0, 0, 0, 0, 0};
	FORM  *form;
	unsigned int i;
	char *user, *email, *pass, *pass2;
	size_t pass_size;

	for (i = 0; i < 4; ++i) {
		fields[i] = new_field(1, CRLS_MAXNAMELEN, 4 + i, 18, 0, 0);
		field_opts_off(fields[i], O_AUTOSKIP);
	}

	/* Protect the password fields */
	for (i = 2; fields[i] != NULL; ++i)
		field_opts_off(fields[i], O_PUBLIC);

	form = new_form(fields);
	post_form(form);
	print_file("menus/register.txt");
	refresh();

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
	if (isalnum(*user) == 0) {
		scrmsg(14, 1, "Only alphanumerics caracteres are allowed for the first letter.");
		goto clean;
	}
	for (i = 0; user[i] != '\0'; ++i) {
		if (isascii(user[i]) == 0) {
			scrmsg(14, 1, "Only ascii in the username");
			goto clean;
		}
		if (user[i] == ':') {
			scrmsg(14, 1, "':' caractere is forbid");
			goto clean;
		}
	}

	/* This function actually print is own error message */
	if (do_user_exist(user) == 0)
		db_insert(user, email, crypt(pass, "$1"));

clean:
	form_release(form);
}

void
menus(void) {
	unsigned char c = 0;

	do {
		switch (c) {
			case 'l':
			case 'L':
				if (login_menu() != 0) {
					mvaddstr(14, 1, "Error");
					sleep(1);
				}
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
		print_file("menus/general.txt");
	} while ((c = getch()) != 'q');
}
