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

#define LDAP_DEPRECATED 1

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ldap.h>
#include <unistd.h>

#include "crlserver.h"
#include "db.h"
#include "log.h"

static LDAP *gl_ld;
static char *gl_uri;
static char *gl_dn;

int
db_open(const char *ldap_uri) {
	(void)ldap_uri;
	int err;

	err = ldap_initialize(&gl_ld, gl_uri);
	if (err != LDAP_SUCCESS) {
		log_warnx("Can't open %s: %s", gl_uri, ldap_err2string(err));
		return -1;
	}
	return 0;
}

int
db_close(const char *ldap_uri) {
	(void)ldap_uri;

	ldap_unbind_s(gl_ld);
	free(gl_uri);
	free(gl_dn);
	gl_uri = NULL;
	gl_dn = NULL;
	gl_ld = NULL;
	return 0;
}

int
db_init(const char *ldap_uri) {
	(void)ldap_uri;
	return 0;
}

int
db_user_add(const char *name, const char *email, const char *password) {
	(void)name;
	(void)email;
	(void)password;
	return 0;
}

int
db_user_mod_email(const char *name, const char *email) {
	(void)name;
	(void)email;
	return 0;
}

int
db_user_mod_password(const char *name, const char *hash) {
	(void)name;
	(void)hash;
	return 0;
}

int
db_user_auth(const char *name, const char *password) {
	int err;
	char dn[1024];

	(void)memset(dn, 0, sizeof dn);
	(void)snprintf(dn, sizeof dn, "uid=%s,%s\n", name, gl_dn);

	if (gl_ld == NULL) {
		log_screen(0, 0, "gl_ld == NULL");
	}

	err = ldap_simple_bind_s(gl_ld, dn, password);
	if (err != LDAP_SUCCESS) {
		log_warnx("User %s: authentication failed", ldap_err2string(err));
		return -1;
	}
	return 0;
}

int
db_check(const char *ldap_uri) {
	char *p;

	/* For the LDAP backend db_check only split the given URI */
	p = strrchr(ldap_uri, '/');
	if (p == NULL)
		return -1;

	gl_dn = strdup((p + 1));
	gl_uri = strndup(ldap_uri, strlen(ldap_uri) - strlen(gl_dn));

	log_screen(0, 0, gl_uri);
	return 0;
}

