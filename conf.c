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

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>

#include "crlserver.h"

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

static char *
parse_space(char *p) {
	while (isspace(*p))
		p++;
	return p;
}

static char *
parse_walk_next_line(char *p) {
	while (*p != '\n')
		++p;
	++p;
	return p;
}

static char *
parse_string(char *p, char *str) {
	char *word;
	char *endword;
	int len;

	len = strlen(str);

	word = p;
	if (isalpha(*p) || *p == '_') {
		p++;
		while (isalpha(*p) || isdigit(*p) || *p == '_')
			p++;
	}
	endword = p;

	if (endword - word != len)
		return NULL;
	if (strncmp(word, str, len) != 0)
		return NULL;
	return p;
}

static char *
parse_var_string(char *p, const char *fnm, int *linep) {
	char *stringstart;
	char *string;
	char buf[78];
	char *value;

	/* a string begin with a '"' */
	if (*p != '"')
		errx(1, "%s:%d: invalid string", fnm, *linep);
	++p;

	/* check the validity of the string */
	stringstart = p;
	while (*p != '"')
		++p;
	string = p;
	(void)strncpy(buf, stringstart, string - stringstart);
	buf[string - stringstart] = '\0';
	value = strdup(buf);
	if (value == NULL)
		errx(1, "strdup");

	/* and end with an other '"' */
	if (*p != '"')
		errx(1, "%s:%d: invalid string value \"%s\"", 
	    	    fnm, *linep, buf);
	return value;
}

static char **
parse_var_array(char **p, const char *fnm, int *linep) {
	char *value;
	char **array;
	int i;

	array = calloc(10, sizeof(*array));
	if (array == NULL)
		errx(1, "calloc");
	for (i = 0; i < 10; ++i) {
		value = parse_var_string(*p, fnm, linep);
		*p += strlen(value) + 2;
		array[i] = strdup(value);
		if (array[i] == NULL)
			errx(1, "calloc");

		*p = parse_space(*p);

		/* no coma mean that we are at the end of the declaration */
		if (**p != ',')
			break;
		++*p;
		*p = parse_space(*p);
		if (**p == '#')
			*p = parse_space(parse_walk_next_line(*p));
	}
	if (i == 10)
		errx(1, "too much elements in array");
	return array;
}

static char
parse_var_char(char **p, const char *fnm, int *linep) {
	char *cstart;
	char *c;

	/* a char begin with a '\'' */
	if (**p != '\'')
		errx(1, "%s:%d: invalid char format", fnm, *linep);
	++*p;

	cstart = *p;
	while (**p != '\'')
		++*p;
	c = *p;
	if (c - cstart != 1)
		errx(1, "%s:%d: invalid char format", fnm, *linep);

	/* and end with an other '"' */
	if (**p != '\'')
		errx(1, "%s:%d: invalid char format", fnm, *linep);
	++*p;
	return *cstart;
}

/*
static char *
parse_int(char *p, struct kwvar *kvp, const char *fnm, int *linep) {
	char *valuestart, *digitstart;
	char savec;
	int newval;

	valuestart = p;
	if (*p == '-') 
		p++;
	digitstart = p;
	while (isdigit(*p))
		p++;
	if ((*p == '\0' || isspace(*p) || *p == '#') && digitstart != p) {
		savec = *p;
		*p = '\0';
		newval = atoi(valuestart);
		*p = savec;
		warnx("%s:%d: %s: %d", 
			fnm, *linep, kvp->kw, newval);
		*(int *)kvp->var = newval;
		return p;
	} else {
		errx(1, "%s:%d: invalid integer value \"%s\"", 
		    fnm, *linep, valuestart);
	}
	return NULL;
}
*/

static void
list_insert_string(struct list *lp, char *name, void *value) {
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
	else
		errx(1, "wtf ?");
}

static void
list_insert_array(struct list *lp, char *name, char **value) {
	if (strncmp(name, "par", 3) == 0)
		lp->params = (char **)value;
	else if (strncmp(name, "env", 3) == 0)
		lp->env = (char **)value;
}

static char *
parse_value(char *buf, struct list *lp, int index, char *fnm, int *linep) {
	char *p;
	char cvalue;
	char *svalue;
	char **avalue;

	p = buf;
	switch (varname[index].type) {
	case Vstring:
		svalue = parse_var_string(p, fnm, linep);
		p += strlen(svalue) + 2;
		list_insert_string(lp, varname[index].kw, svalue);
		break;
	case Vchar:
		cvalue = parse_var_char(&p, fnm, linep);
		list_insert_string(lp, varname[index].kw, &cvalue);
		break;
	case Varray:
		avalue = parse_var_array(&p, fnm, linep);
		list_insert_array(lp, varname[index].kw, avalue);
		break;
	case Vint:
	case Vdouble:
	case Vbool:
	default:
		abort();
	}

	p = parse_space(p);
	if (*p == '#')
		while (*p != '\n' && *p != '\0')
			++p;
	return p;
}

static int
parse_varname(char *p) {
	char *savep;
	int i;

	for (i = 0; varname[i].kw; i++) {
		savep = parse_string(p, varname[i].kw);
		if (savep != NULL)
			break;
	}
	return i;
}

static char *
parse_line(char *buf, struct list *lp, char *fnm, int *line) {
	char *p;
	int index = 0;

	p = parse_space(buf);

	/* skip blank lines and comment lines */
	if (*p == '\n' || *p == '#')
		return parse_walk_next_line(p);

	/* search a variable name */
	index = parse_varname(p);
	if (varname[index].kw == NULL)
		errx(1, "%s:%d: bad variable name: %s", fnm, *line, p);
	p += strlen(varname[index].kw);
	p = parse_space(p);

	/* search for the assignement operator */
	if (*p != '=')
		errx(1, "%s:%d: expected '='", fnm, *line);
	p++;
	p = parse_space(p);

	/* parse the value */
	p = parse_value(p, lp, index, fnm, line);
	p = parse_space(p);

	/* end of line comment check is the responsability of parse_value */
	return p;
}

static int
parse_blockname(char *p) {
	char *savep;
	int i;

	for (i = 0; blockname[i]; i++) {
		savep = parse_string(p, (char *)blockname[i]);
		if (savep != NULL)
			break;
	}

	/*warnx("block %s", blockname[i]);*/
	return i;
}

static char *
parse_block(char *buf, char *fnm, int *line) {
	int index;
	char *p = buf;
	struct list *lp;

	/* skip leading spaces */
	p = parse_space(p);

	/* allow blank lines and comment lines */
	while (*p == '#' || *p == '\n')
		p = parse_walk_next_line(p);

	/* are we at the end of file ? */
	if (*p == '\0')
		return p;

	/* check for the 'new' keyword */
	p = parse_string(p, "new");
	if (p == NULL)
		errx(1, "%s:%d: missing new operator", fnm, *line);

	/* skip spaces */
	p = parse_space(p);

	/* match the block name */
	index = parse_blockname(p);
	if (blockname[index] == NULL)
		errx(1, "%s:%d: bad block name", fnm, *line);
	p += strlen(blockname[index]);
	lp = calloc(1, sizeof *lp);

	/* skip spaces */
	p = parse_space(p);

	/* check for the assignement operator */
	if (*p != '=')
		errx(1, "%s:%d: missing assignement operator", fnm, *line);
	++p;
	
	/* skip spaces */
	p = parse_space(p);

	/* check for the '{' operator */
	if (*p != '{') {
		p = parse_walk_next_line(p);
		p = parse_space(p);
		if (*p != '{')
			errx(1, "%s:%i: missing opening braces", fnm, *line);
	}
	++p;

	/* check for the '}' operator */
	/* XXX: unsafe */
	while (*p != '}')
		p = parse_line(p, lp, fnm, line);

	++p;
	/*warnx("eat one block");*/

	if (strncmp(blockname[index], "editor", 6) == 0)
		SLIST_INSERT_HEAD(&elh, (editors_list *)lp, ls);
	else
		SLIST_INSERT_HEAD(&glh, (games_list *)lp, ls);

	return p;
}

/*
 * mmap the given config file and start the parsing
 */
static void
load_config(int fd, int len, char *fnm) {
	int line;
	void *buf;
	char *p;

	buf = mmap(0, len, PROT_READ, MAP_FILE, fd, 0);
	if (buf == MAP_FAILED)
		err(1, "mmap");

	line = 0;
	p = (char *)buf;
	while (*p != '\0') {
		p = parse_block(p, fnm, &line);
	}
	/*warnx("end of file");*/
	if (munmap(buf, len) == -1)
		err(1, "munmap");
}

/*
 * load various config file, allowing later ones to 
 * overwrite earlier values
 */
void
config(void) {
	char *home;
	char nm[MAXNAMLEN + 1];
	static char *fnms[] = { 
		"/etc/crlserver.conf",
		"%s/.crlserver.conf",
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
				err(1, "stat");

			load_config(fd, st.st_size, nm);
			(void)close(fd);
		} 
		else if (errno != ENOENT)
			warnx("%s", nm);
	}
}

/*
int
main(void) {
	struct list *lp;
	int i;

	config();
	SLIST_FOREACH(lp, &glh, ls) {
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
*/

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
		for (i = 0; lp->params[i] != NULL; ++i) 
			free(lp->params[i]);
		free(lp);
	}
}

