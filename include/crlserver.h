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

#include <stdarg.h>
#include <curses.h>
#include <menu.h>

#include "list.h"
#include "session.h"

#ifndef CRLSERVER_H_
#define CRLSERVER_H_

#define DROWS 24
#define DCOLS 80

#define CRLSERVER_DEFAULT_TERM "xterm"
#define CRLSERVER_MAX_ENV_LENGTH 10
#define CRLSERVER_MAX_PARAMS_LENGTH 10
#define CRLS_MAXNAMELEN 20

extern struct session session;
extern games_list_head glh;
extern editors_list_head elh;

/* aux.c */
__inline int isokay(int);
void rtrim(char *);
void ltrim(char **);
void trim(char **);
char *escape_space(char *);
void ignore_signals(void);
void heed_signals(void);

/* conf.c */
void config(void);
void list_release(struct list_head *);
void list_finalize(struct list_head *);

/* init.c */
void byebye(int);
int init_playground_files(const char *);
int init_playground_dir(const char *);
int init_session(const char *);
void init(void);
void start_window(void);
__inline void end_window(void);
void free_env(void);

/* menus.c */
void menus(void);
int has_config_file(games_list *);

#endif
