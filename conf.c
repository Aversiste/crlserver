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

/*
 * The idea for this parser came from hunt, but I rewrite it a little bit.
 * This is the original copyright :
 * David Leonard <d@openbsd.org>, 1999. Public domain.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/queue.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "crlserver.h"
#include "session.h"
#include "list.h"
#include "log.h"

int debug;

struct kwvar {
	char *kw;
	enum vartype { Vint, Vchar, Vstring, Varray,
	    Vbool, Vdouble } type;
};

static const char *blockname[] = {"game", "editor", NULL};
static const struct kwvar varname[] = {
	{"name", Vstring},
	{"longname", Vstring},
	{"version", Vstring},
	{"description", Vstring},
	{"key", Vchar},
	{"path", Vstring},
	{"params", Varray},
	{"env", Varray},
	{NULL, Vstring}
};

void
list_release(struct list_head *lh) {
	struct list *lp;

	while (!SLIST_EMPTY(lh)) {
		unsigned int i;
		lp = SLIST_FIRST(lh);
		SLIST_REMOVE_HEAD(lh, ls);

		free(lp->name);
		free(lp->lname);
		free(lp->version);
		free(lp->desc);
		free(lp->path);
		
		/* Don't double free() the path */
		for (i = 1; lp->params[i] != NULL; ++i)
			free(lp->params[i]);
		free(lp->params);

		for (i = 0; lp->env[i] != NULL; ++i)
			free(lp->env[i]);
		free(lp->env);

		free(lp);
		lp = NULL;
	}
}

void
list_finalize(struct list_head *lh) {
	int i;
	struct list *lp;
	char home[MAXPATHLEN + 5];

	(void)snprintf(home, sizeof home, "HOME=%s", session.home);
	SLIST_FOREACH(lp, lh, ls) {
		/* Does params exist ? */
		if (lp->params == NULL) {
			lp->params = calloc(2, sizeof *(lp->params));
			if (lp->params == NULL)
				log_err(1, "%s:%s:", __FILE__, __func__);
		}

		/* Does env exist ? */
		if (lp->env == NULL) {
			lp->env = calloc(3, sizeof *(lp->env));
			if (lp->env == NULL)
				log_err(1, "%s:%s:", __FILE__, __func__);
		}

		/* Set the first params to the path */
		if (lp->path == NULL)
			log_errx(1, "conf: the path must be set");
		lp->params[0] = lp->path;

		/* Set default environment variables */
		lp->env[0] = strdup(home);
		if (lp->env[0] == NULL)
			log_err(1, "%s:%s:", __FILE__, __func__);
		for (i = 1; lp->env[i] != NULL; ++i);
		lp->env[i] = strdup("TERM="CRLSERVER_DEFAULT_TERM);
		if (lp->env[i] == NULL)
			log_err(1, "%s:%s:", __FILE__, __func__);
		lp->env[i + 1] = NULL;
	}
}

static void
parse_inner_replacement(char **value) {
	char *c;
	char buf[MAXNAMLEN];

	/* replace %u by the username */
	memset(buf, 0, sizeof buf);
	c = strchr(*value, '%');
	if (c != NULL && c[1] == 'u') {
		c[1] = 's';
		(void)snprintf(buf, sizeof buf, *value, session.name);
		free(*value);
		*value = strdup(buf);
		if (*value == NULL)
			log_err(1, "%s:%s:", __FILE__, __func__);
	}

}

static int
parse_char(char **p, char c) {
	if (**p == c) {
		(*p)++;
		return 0;
	}
	return -1;
}

static void
parse_space(char **p) {
	while (isspace(**p))
		(*p)++;
}

static int
parse_walk_next_line(char **p) {
	while (**p != '\0' && **p != '\n')
		(*p)++;
	if (**p == '\0')
		return -1;
	(*p)++;
	return 0;
}

static int
parse_string(char **p, const char *str) {
	char *word;
	char *endword;
	size_t len;

	len = strlen(str);
	word = *p;
	if (isalpha(**p) || **p == '_') {
		(*p)++;
		while (isalpha(**p) || isdigit(**p) || **p == '_')
			(*p)++;
	}
	endword = *p;

	if ((size_t)(endword - word) != len || strncmp(word, str, len) != 0) {
		*p = word;
		return -1;
	}

	return 0;
}

static int
parse_var_string(char **p, char **value) {
	char *stringstart;
	char *string;
	char buf[78];

	/* a string begin with a '"' */
	if (parse_char(p, '"') == -1)
		log_errx(1, "invalid string");

	/* check the validity of the string */
	stringstart = *p;
	while (**p != '\0' && **p != '"')
		(*p)++;
	string = *p;
	(void)strncpy(buf, stringstart, string - stringstart);
	buf[string - stringstart] = '\0';

	/* and end with an other '"' */
	if (parse_char(p, '"') == -1)
		log_errx(1, "conf: invalid string value");

	*value = strdup(buf);
	if (*value == NULL)
		log_err(1, "%s:%s:", __FILE__, __func__);

	return 0;
}

static int
parse_var_array(char **p, char ***array) {
	int e;
	unsigned int i;

	*array = calloc(1, sizeof *array);
	if (*array == NULL)
		log_err(1, "%s:%s:", __FILE__, __func__);

	/*
         * i start at 1, because:
	 * - params[0] is latter set to the path
	 * - env[0] is HOME
	 */
	for (i = 1; ; ++i) {
		char *value;

		if ((*array)[i] == NULL) {
			size_t nsize;
			char **npp;

			nsize = i + 1;
			npp = (char **)realloc(*array, nsize*sizeof(char*));
			if (npp == NULL)
				log_err(1, "%s:%s:", __FILE__, __func__);
			*array = npp;
			(*array)[nsize] = NULL;
		}
		parse_var_string(p, &value);
		parse_inner_replacement(&value);
		parse_space(p);
		/* Check for %u or %h */
		(*array)[i] = value;

		/* no coma mean that we are at the end of the declaration */
		e = parse_char(p, ',');
		if (e == -1)
			break;

		parse_space(p);
		if (parse_char(p, '#') >= 0) {
			parse_walk_next_line(p);
			parse_space(p);
		}
	}
	return 0;
}

static int
parse_var_char(char **p, char *value) {
	char *cstart;
	char *c;

	/* a char begin with a '\'' */
	if (parse_char(p, '\'') == -1)
		log_errx(1, "invalid char format");

	cstart = *p;
	while (**p != '\0' && **p != '\'')
		(*p)++;
	c = *p;
	if (c - cstart != 1)
		log_errx(1, "invalid char format");

	/* and end with an other '\'' */
	if (parse_char(p, '\'') == -1)
		log_errx(1, "invalid char format");

	*value = *cstart;
	return 0;
}

static void
list_insert(struct list *lp, char *name, void *value) {
	if (strncmp(name, "nam", 3) == 0)
		lp->name = (char *)value;
	else if (strncmp(name, "lon", 3) == 0)
		lp->lname = (char *)value;
	else if (strncmp(name, "ver", 3) == 0)
		lp->version = (char *)value;
	else if (strncmp(name, "des", 3) == 0)
		lp->desc= (char *)value;
	else if (strncmp(name, "key", 3) == 0)
		lp->key = *(char *)value;
	else if (strncmp(name, "pat", 3) == 0)
		lp->path = (char *)value;
	else if (strncmp(name, "par", 3) == 0)
		lp->params = (char **)value;
	else if (strncmp(name, "env", 3) == 0)
		lp->env = (char **)value;
}

static int
parse_value(char **p, int vindex, struct list *lp) {
	char *svalue;
	char cvalue;
	char **avalue;

	switch (varname[vindex].type) {
	case Vstring:
		parse_var_string(p, &svalue);
		list_insert(lp, varname[vindex].kw, svalue);
		break;
	case Vchar:
		parse_var_char(p, &cvalue);
		list_insert(lp, varname[vindex].kw, &cvalue);
		break;
	case Varray:
		parse_var_array(p, &avalue);
		list_insert(lp, varname[vindex].kw, avalue);
		break;
	case Vint:
	case Vdouble:
	case Vbool:
	default:
		log_errx(1, "wtf");
	}

	parse_space(p);
	if (**p == '#')
		while (**p != '\n' && **p != '\0')
			(*p)++;
	return 0;
}

static int
parse_varname(char **p) {
	char *savep;
	int i;
	int e;

	for (i = 0; varname[i].kw; i++) {
		savep = *p;
		e = parse_string(p, varname[i].kw);
		if (e >= 0)
			return i;
		else
			*p = savep;
	}
	return -1;
}

static void
parse_line(char **p, struct list *lp) {
	int e;
	int vindex;

	parse_space(p);

	/* skip blank lines and comment lines */
	if (**p == '\n' || **p == '#') {
		parse_walk_next_line(p);
		return;
	}

	/* search a variable name */
	e = parse_varname(p);
	if (e == -1)
		log_errx(1, "bad variable name");
	/* Save the index on the variable table */
	vindex = e;

	parse_space(p);

	/* search for the assignement operator */
	e = parse_char(p, '=');
	if (e == -1)
		log_errx(1, "expected '='");
	parse_space(p);

	/* parse the value */
	parse_value(p, vindex, lp);
	parse_space(p);

	/* end of line comment check is the responsability of parse_value */
	return ;
}

static int
parse_blockname(char **p) {
	int i;
	int e;

	for (i = 0; blockname[i]; i++) {
		e = parse_string(p, (char *)blockname[i]);
		if (e >= 0)
			return i;
	}
	return -1;
}

static char *
parse_block(char *buf) {
	int e;
	int bindex;
	char *p = buf;
	struct list *lp;

	/*
	 * initialize the list
	 */
	lp = calloc(1, sizeof *lp);

	/* skip leading spaces */
	parse_space(&p);

	/* allow blank lines and comment lines */
	while (*p == '#' || *p == '\n')
		parse_walk_next_line(&p);

	/* are we at the end of file ? */
	if (*p == '\0')
		return p;

	/* skip spaces */
	parse_space(&p);

	/* match the block name */
	e = parse_blockname(&p);
	if (e == -1)
		log_errx(1, "conf: bad block name");
	bindex = e;

	/* skip spaces */
	parse_space(&p);

	/* check for the '{' operator */
	e = parse_char(&p, '{');
	if (e == -1) {
		parse_walk_next_line(&p);
		parse_space(&p);
		e = parse_char(&p, '{');
		if (e == -1)
			log_err(1, "conf: missing openning braces");
	}

	/* check for the '}' operator */
	do {
		parse_line(&p, lp);
	} while (parse_char(&p, '}') == -1);

	if (strncmp(blockname[bindex], "editor", 6) == 0)
		SLIST_INSERT_HEAD(&elh, (editors_list *)lp, ls);
	else
		SLIST_INSERT_HEAD(&glh, (games_list *)lp, ls);

	return p;
}

/*
 * mmap the given config file and start the parsing
 */
static void
load_config(int fd, unsigned long int len) {
	void *buf;
	char *p;

	buf = mmap(0, len, PROT_READ, MAP_FILE, fd, 0);
	if (buf == MAP_FAILED)
		log_err(1, "mmap");

	p = (char *)buf;
	while (*p != '\0') {
		p = parse_block(p);
	}

	if (munmap(buf, len) == -1)
		log_err(1, "munmap");
}

/*
 * load various config file, allowing later ones to 
 * overwrite earlier values
 */
void
config(void) {
	char *home;
	char nm[MAXNAMLEN + 1];
	char *fnms[] = { 
		".crlserver.conf",
		NULL
	};
	int fn;
	int fd;
	struct stat st;

	/* All the %s's get converted to $HOME */
	if ((home = getenv("HOME")) == NULL)
		home = "";

	for (fn = 0; fnms[fn]; fn++) {
		(void)snprintf(nm, sizeof nm, fnms[fn], home);
		if ((fd = open(nm, O_RDONLY)) != -1) {
			if (stat(nm, &st) == -1)
				log_err(1, "stat");

			load_config(fd, st.st_size);
			(void)close(fd);
		} 
		else if (errno != ENOENT)
			log_notice("config: %s", nm);
	}
}

#ifdef CONF_DEBUG
int
main(int argc, char *argv[]) {
	int ch;
	struct list *lp;

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
		case 'd':
			++debug;
		}
	}
	argc -= optind;
	argv += optind;

	config();
	list_finalize(&glh);
	list_finalize(&elh);
	SLIST_FOREACH(lp, &glh, ls) {
		int i;
		printf("\n%s (%s %s)\n", lp->name, lp->lname, lp->version);
		printf(" -> %s\n", lp->desc);
		printf(" -> %s\n", lp->path);
		printf(" -> %c\n", lp->key);
		if (lp->params != NULL) {
			printf(" -> params\n");
			for (i = 0; lp->params[i]; ++i)
				printf("   -> [%s]\n", lp->params[i]);
		}
		if (lp->env != NULL) {
			printf(" -> env\n");
			for (i = 0; lp->env[i]; ++i)
				printf("   -> [%s]\n", lp->env[i]);
		}
	}
	return 0;
}
#endif
