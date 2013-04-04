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
#include "list.h"
#include "log.h"

void
print_file(const char *path) {
	FILE *fd = fopen(path, "r");
	int y = 0;
	char buf[80];

	if (fd == NULL)
		log_err(1, "Error with file %s\n", path);

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

//static void
//editors_menu(struct list *lp) {
//	char rc_path[MAXPATHLEN];
//	int status = 0, ch = 0;
//	pid_t pid = 0;
//
//	(void)memset(rc_path, '\0', MAXPATHLEN);
//	(void)strlcpy(rc_path, session.home, sizeof rc_path);
//	(void)strlcat(rc_path, "/.", sizeof rc_path);
//	(void)strlcat(rc_path, glp->name, sizeof rc_path);
//	(void)strlcat(rc_path, "rc", sizeof rc_path);
//	do {
//		int i = 6;
//		print_file(CRLSERVER_CONFIG_DIR"/menus/banner.txt");
//		SLIST_FOREACH(lp, &elh, ls) {
//			(void)mvprintw(i, 1, "%c) Edit with %s (%s %s)",
//			lp->key, lp->name, lp->lname, lp->version);
//			++i;
//		}
//		(void)mvaddstr(4, 1, "q) Quit");
//		(void)refresh();
//		ch = getch();
//		if (ch == 'q')
//			break;
//		SLIST_FOREACH(lp, &elh, ls) {
//			if (ch == lp->key) {
//				(void)clear();
//				(void)refresh();
//				(void)endwin();
//
//				if (lp->params[1] != NULL)
//					free(lp->params[1]);
//				/*
//				 * XXX: We should check for the next free place
//				 * rather than use 1.
//				 */
//				lp->params[1] = strdup(rc_path);
//				if (lp->params[1] == NULL)
//					log_err(1, "editors_menu");
//
//				pid = fork();
//				if (pid < 0)
//					log_err(1, "fork");
//				else if (pid == 0) {
//					execve(lp->path, lp->params, lp->env);
//					log_debug("%s: %s %s\n", session.name, lp->path, lp->params[1]);
//					log_err(1, "execve");
//				}
//				else
//					waitpid(pid, &status, 0);
//					(void)clear();
//					(void)refresh();
//				break;
//
//				free(lp->params[1]);
//				lp->params = NULL;
//			}
//		}
//		lp = NULL;
//	} while (1);	
//}

static void
games_menu(struct list *lp) {
	int status = 0, ch = 0;
	pid_t pid = 0;
	int configurable = has_config_file(lp);

	do {
		switch (ch) {
		case 'p':
		case 'P':
			(void)clear();
			(void)refresh();
			(void)endwin();

			pid = fork();
			if (pid < 0)
				log_err(1, "fork");
			else if (pid == 0) {
				execve(lp->l_path, lp->l_params, lp->l_env);
				fprintf(stderr, "%s: %s\n", session.name,
				    lp->l_path);
				log_err(1, "execve");
			}
			else
				waitpid(pid, &status, 0);
			break;
		case 'e':
		case 'E':
			if (configurable == 0)
				configurable = 1;
				//editors_menu(glp);
			break;
		case 'q':
		case 'Q':
			return;
		default:
			break;
		}
		print_file(CRLSERVER_CONFIG_DIR"/menus/games.txt");
		if (configurable == 0)
			mvprintw(5, 1,"%s", "e) Edit the configuration file");
		mvprintw(7, 1,"%s", lp->l_description);
		refresh();
	} while ((ch = getch()) != 'q');
}

//static void
//settings_menu() {
//	int ch = 0;
//
//	do {
//		print_file(CRLSERVER_CONFIG_DIR"/menus/settings.txt");
//		ch = getch();
//		switch (ch) {
//		case 'e':
//		case 'E':
//			break;
//		case 'p':
//		case 'P':
//			break;
//		case 'q':
//		case 'Q':
//			return;
//		default:
//			break;
//		}
//	} while (1);
//}

static void
user_menu(struct list_head *headp) {
	int ch = 0;

	do {
		int i = 7;
		struct list *lp;

		print_file(CRLSERVER_CONFIG_DIR"/menus/banner.txt");
		(void)mvaddstr(4, 1, "q) Quit");
		//(void)mvaddstr(5, 1, "s) Settings");
		SLIST_FOREACH(lp, headp, l_next) {
			if (lp->l_type == LT_GAME) {
				mvprintw(i, 1, "%c) %s - %s (%s)", lp->l_key,
				    lp->l_name, lp->l_longname, lp->l_version);
				++i;
			}
		}
		(void)refresh();

		ch = getch();
		switch (ch) {
		case 's':
		case 'S':
			//settings_menu();
			break;
		default:
			break;
		}
		SLIST_FOREACH(lp, headp, l_next) {
			if (ch == lp->l_key && lp->l_type == LT_GAME) {
				games_menu(lp);
				break;
			}
		}
	} while (ch != 'q');
	session.logged = 0;
	free(session.name);
	free(session.home);
}

static void
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
		log_screen(14, 1, "A field is missing\n");
		return NULL;
	}
	trim(&s);
	if (strlen(s) < 1) {
		log_screen(14, 1, "A field is missing\n");
		return NULL;
	}
	return s;
}

/* Return 0 in case of a succesfull login or -1*/
static int
login_menu(struct list_head *headp) {
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
		log_err(1, "curses: login_menu");
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

	session.logged = 1;
	(void)memset(pass, 0, CRLS_MAXNAMELEN);
	pass = NULL;

	if (init_session(user) != 0) {
		log_screen(14, 1, "Error whith your session");
		goto clean;
	}

	/*
	 * Successful login!
	 *    - Replace %user% by the real username
	 *    - Replace HOME environnment variable
	 */
	{
		struct list *lp;
		char buf[MAXPATHLEN + 5];

		SLIST_FOREACH(lp, headp, l_next) {
			unsigned int i;
			for (i = 0; lp->l_params[i] != NULL; ++i) {
				if (strcmp(lp->l_params[i], "%user%") != 0)
					continue;
				free(lp->l_params[i]);
				/* 
				 * XXX: Trouble if reconnection 
				 * with an other account
				 */
				lp->l_params[i] = strdup(session.name);
			}
			for (i = 0; lp->l_env[i] != NULL; ++i) {
				if (strncmp(lp->l_env[i], "HOME=", 5) != 0)
					continue;
				free(lp->l_env[i]);
				(void)snprintf(buf, sizeof buf,
				    "HOME=%s", session.home);
				lp->l_env[i] = strdup(buf);
				break;
			}
		}
	}

	(void)form_release(form);
	user_menu(headp);
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
		log_err(1, "curses: register_menu");
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
		log_screen(14, 1, "Put a valid email please.");
		goto clean;
	}
	if (strncmp(pass, pass2, pass_size) != 0) {
		log_screen(14, 1, "Passwords don't match!");
		goto clean;
	}
	for (i = 0; user[i] != '\0'; ++i) {
		if (isokay(user[i]) == 0) {
			log_screen(14, 1,
			    "Only ascii alpha numerics in the username");
			goto clean;
		}
	}

	{
		int err = init_playground_dir(user);
		if (err == -1) {
			log_screen(14, 1,
			    "Error while creating your playground dir");
			goto clean;
		}
		if (init_playground_rcfiles(user) == -1) {
			log_screen(14, 1,
			    "Error while creating your playgrounds file");
			goto clean;
		}
	}

	/* This function actually print is own error message */
	db_user_add(user, email, crypt(pass, "$1"));

clean:
	(void)form_release(form);
}

void
menu_general(struct list_head *headp) {
	unsigned char c = 0;

	do {
		print_file(CRLSERVER_CONFIG_DIR"/menus/general.txt");
		c = getch();
		switch (c) {
		case 'l':
		case 'L':
			(void)login_menu(headp);
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
	} while (c != 'q');
}

