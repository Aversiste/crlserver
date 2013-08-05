/*
 * Copyright (c) 2013 Tristan Le Guern <leguern AT medu DOT se>
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

#include <string.h>

#include "crlserver.h"
#include "filter.h"
#include "log.h"

int
register_password_validation(const char *password) {
	(void)password;
	return 0;
}

int
register_email_validation(const char *email) {
	/* Don't annoy people too much */
	if (strchr(email, '@') == NULL) {
		log_screen(14, 1, "Put a valid email please.");
		return -1;
	}
	return 0;
}

int
register_username_validation(const char *username) {
	unsigned int i;

	for (i = 0; username[i] != '\0'; ++i) {
		if (isokay(username[i]) == 0) {
			log_screen(14, 1,
			    "Only ascii alpha numerics in the username");
			return -1;
		}
	}

	if (filter_match(username) != 0) {
		log_screen(14, 1, "This username is reserved");
		return -1;
	}
	return 0;
}

int
register_create_home(const char *username) {
	if (init_playground_dir(username)) {
		log_screen(14, 1,
		    "Error while creating your playground dir");
		return -1;
	}

	if (init_playground_rcfiles(username) == -1) {
		log_screen(14, 1,
		    "Error while creating your playgrounds file");
		return -1;
	}
	return 0;
}

