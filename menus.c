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
#include <curses.h>
#include <err.h>
#include <form.h>
#include <stdlib.h>
#include <unistd.h>

#include "crlserver.h"
#include "init.h"
#include "log.h"
#include "menus.h"

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
server_info(void) {
	unsigned char c;

	print_file("menus/server_info.txt");
	while ((c = getch()) != 'q') {
		continue;
	}
}

static void
login_user(void) {
	FIELD *fields[3] = {0, 0, 0};
	FORM  *my_form;
	unsigned int i, ch;
	bool quit = false;

	curs_set(1); /* Print the cursor */
	for (i = 0; i < 2; ++i) {
		fields[i] = new_field(1, CRLS_MAXNAMELEN, 4 + i, 18, 0, 0);
		field_opts_off(fields[i], O_AUTOSKIP);
	}
	/* Protect the password */
	field_opts_off(fields[1], O_PUBLIC);

	my_form = new_form(fields);
	post_form(my_form);
	print_file("menus/login.txt");
	refresh();

	for (i = 0, ch = 0; quit == false; (ch = getch())) {
		switch(ch) {
		case KEY_DOWN:
		case '\t':
			form_driver(my_form, REQ_NEXT_FIELD);
			form_driver(my_form, REQ_END_LINE);
			break;
		case KEY_UP:
			form_driver(my_form, REQ_PREV_FIELD);
			form_driver(my_form, REQ_END_LINE);
			break;
		case KEY_RIGHT:
			form_driver(my_form, REQ_NEXT_CHAR);
			break;
		case KEY_LEFT:
			form_driver(my_form, REQ_PREV_CHAR);
			break;
		case KEY_DC:
			form_driver(my_form, REQ_DEL_CHAR);
			break;
		case KEY_BACKSPACE:
			form_driver(my_form, REQ_DEL_PREV);
			break;
		case KEY_HOME:
			form_driver(my_form, REQ_BEG_FIELD);
			break;
		case KEY_END:
			form_driver(my_form, REQ_END_FIELD);
			break;
		case KEY_ENTER:
		case '\n':
		case '\r':
			form_driver(my_form, REQ_VALIDATION);
			quit = true;		
			break;
		default:
			form_driver(my_form, ch);
			break;
		}
	}
	
	mvprintw(15, 1, "[%s] %i", field_buffer(fields[0], 0), field_status(fields[0]));
	mvprintw(16, 1, "[%s] %i", field_buffer(fields[1], 0), field_status(fields[1]));
	refresh();

	unpost_form(my_form);
	free_form(my_form);
	free_field(fields[0]);
	free_field(fields[1]);
	curs_set(0); /* Remove the cursor */

}

static void
register_user(void) {
	FIELD *fields[5] = {0, 0, 0, 0, 0};
	FORM  *my_form;
	unsigned int i, ch;
	bool quit = false;

	curs_set(1); /* Print the cursor */
	for (i = 0; i < 4; ++i) {
		fields[i] = new_field(1, CRLS_MAXNAMELEN, 4 + i, 18, 0, 0);
		field_opts_off(fields[i], O_AUTOSKIP);
	}

	/* Protect the password fields */
	for (i = 2; fields[i] != NULL; ++i)
		field_opts_off(fields[i], O_PUBLIC);

	my_form = new_form(fields);
	post_form(my_form);
	print_file("menus/register.txt");
	refresh();

	for (i = 0, ch = 0; quit == false; (ch = getch())) {
		switch(ch) {
		case KEY_DOWN:
		case '\t':
			form_driver(my_form, REQ_NEXT_FIELD);
			form_driver(my_form, REQ_END_LINE);
			break;
		case KEY_UP:
			form_driver(my_form, REQ_PREV_FIELD);
			form_driver(my_form, REQ_END_LINE);
			break;
		case KEY_RIGHT:
			form_driver(my_form, REQ_NEXT_CHAR);
			break;
		case KEY_LEFT:
			form_driver(my_form, REQ_PREV_CHAR);
			break;
		case KEY_DC:
			form_driver(my_form, REQ_DEL_CHAR);
			break;
		case KEY_BACKSPACE:
			form_driver(my_form, REQ_DEL_PREV);
			break;
		case KEY_HOME:
			form_driver(my_form, REQ_BEG_FIELD);
			break;
		case KEY_END:
			form_driver(my_form, REQ_END_FIELD);
			break;
		case KEY_ENTER:
		case '\n':
		case '\r':
			form_driver(my_form, REQ_VALIDATION);
			quit = true;		
			break;
		default:
			form_driver(my_form, ch);
			break;
		}
	}
	
	mvprintw(15, 1, "[%s] %i", field_buffer(fields[0], 0), field_status(fields[0]));
	mvprintw(16, 1, "[%s] %i", field_buffer(fields[1], 0), field_status(fields[1]));
	mvprintw(17, 1, "[%s] %i", field_buffer(fields[2], 0), field_status(fields[2]));
	mvprintw(18, 1, "[%s] %i", field_buffer(fields[3], 0), field_status(fields[3]));
	refresh();

	unpost_form(my_form);
	free_form(my_form);
	for (i = 0; i < 4; ++i)
		free_field(fields[i]);
	curs_set(0); /* Remove the cursor */
}

void
menus(void) {
	unsigned char c = 0;

	do {
		switch (c) {
		case 'l':
			login_user();
			break;
		case 'r':
			register_user();
			break;
		case 's':
			server_info();
			(void)refresh();
			break;
		case 'q':
			fclean_up("Good Bye");
			break;
		default:
			break;
		}
		print_file("menus/general.txt");
	} while ((c = getch()) != 'q');
}
