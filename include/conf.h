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

#ifndef CONF_H__
#define CONF_H__

#include <sys/queue.h>
#include "crlserver.h"

SLIST_HEAD(list_head, list);
typedef struct list_head games_list_head;
typedef struct list_head editors_list_head;
typedef struct list games_list;
typedef struct list editors_list;

void config(void);
void list_release(struct list_head*);

struct list {
	char *name;
	char *lname;
	char *version;
	char *desc;
	char key;
	char *path;
	char **params;
	char **env;
	SLIST_ENTRY(list) ls;
};

#endif
