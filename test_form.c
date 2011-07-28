#include <form.h>

#include <form.h>
#define CRLS_MAXNAMELEN 15

int main()
{
	FIELD *field[5];
	FORM  *my_form;
	int ch, i;

	/* Initialize curses */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0); /* Remove the cursor */

	/* Initialize the fields */
	field[0] = new_field(1, CRLS_MAXNAMELEN, 4, 18, 0, 0);
	field[1] = new_field(1, CRLS_MAXNAMELEN, 5, 18, 0, 0);
	field[2] = new_field(1, CRLS_MAXNAMELEN, 6, 18, 0, 0);
	field[3] = new_field(1, CRLS_MAXNAMELEN, 7, 18, 0, 0);
	field[4] = NULL;

	/* Set field options */
	for (i = 0; field[i] != NULL; ++i)
		set_field_back(field[i], A_UNDERLINE);
	for (i = 2; field[i] != NULL; ++i)
		field_opts_off(field[i], O_PUBLIC);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	pos_form_cursor(my_form);

	mvprintw(4, 1, "Username:");
	mvprintw(5, 1, "e-mail:");
	mvprintw(6, 1, "password:");
	mvprintw(7, 1, "password (re):");
	refresh();

	/* Loop through to get user requests */
	while((ch = getch()) != KEY_F(1)) {
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
			form_driver(my_form, REQ_PREVIOUS_CHAR);
			break;
		default:
			form_driver(my_form, ch);
			break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 

	endwin();
	return 0;
}
