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

#include <sys/param.h>

#include <stdlib.h>
#include <string.h>
#include <confuse.h>

#include "crlserver.h"
#include "pathnames.h"
#include "list.h"

//static cfg_opt_t editor_opts[] = {
//	CFG_STR("longname", "", CFGF_NONE),
//	CFG_STR("version", "", CFGF_NONE),
//	CFG_STR("description", "", CFGF_NONE),
//	CFG_STR("path", "", CFGF_NONE),
//	CFG_STR("key", "", CFGF_NONE),
//	CFG_STR_LIST("env", "{}", CFGF_NONE),
//	CFG_STR_LIST("params", "{}", CFGF_NONE),
//	CFG_END()
//};

//int cfg_include_lol(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv) {
//	/* YADA YADA YADA */
//}

static cfg_opt_t game_opts[] = {
	CFG_STR("longname", "", CFGF_NONE),
	CFG_STR("version", "", CFGF_NONE),
	CFG_STR("description", "", CFGF_NONE),
	CFG_STR("path", "", CFGF_NONE),
	CFG_STR("key", "", CFGF_NONE),
	CFG_STR_LIST("env", "{}", CFGF_NONE),
	CFG_STR_LIST("params", "{}", CFGF_NONE),
	CFG_END()
};

static cfg_opt_t opts[] = {
	CFG_FUNC("include", cfg_include),
	CFG_STR("db", "/var/db/crlserver.db", CFGF_NONE),
	CFG_STR("log", "/var/log/crlserver.log", CFGF_NONE),
	CFG_SEC("editor", game_opts, CFGF_TITLE | CFGF_MULTI),
	CFG_SEC("game", game_opts, CFGF_TITLE | CFGF_MULTI),
	CFG_END()
};

static void
conf_parse_list(cfg_t *cfg, char *sec, char **array) {
	size_t i, max;

	/* Hack for the 'params' option: the path has to be the first */
	max = cfg_size(cfg, sec);
	array[0] = strdup(cfg_getnstr(cfg, sec, max - 1));

	if (max == 1)
		return;

	for (i = 0; i < max - 1; i++) {
		array[i + 1] = strdup(cfg_getnstr(cfg, sec, i));
	}
	array[max] = NULL;
}

int
conf_parse_section(cfg_t *cfg, struct list *l, const char *sec, const u_int index) {
	cfg_t *cfg_sec;

	cfg_sec = cfg_getnsec(cfg, sec, index);
	if (cfg_sec == NULL)
		return -1;
	switch (sec[0]) {
	case 'g':
		l->l_type = LT_GAME;
		break;
	case 'e':
		l->l_type = LT_EDITOR;
		break;
	default:
		return -1;
	}
	l->l_name = strdup(cfg_title(cfg_sec));
	l->l_longname = strdup(cfg_getstr(cfg_sec, "longname"));
	l->l_version = strdup(cfg_getstr(cfg_sec, "version"));
	l->l_description = strdup(cfg_getstr(cfg_sec, "description"));
	l->l_key = cfg_getstr(cfg_sec, "key")[0];
	l->l_path = strdup(cfg_getstr(cfg_sec, "path"));

	/* Fill default env variable fist */
	cfg_addlist(cfg_sec, "env", 1, "TERM=xterm");
	cfg_addlist(cfg_sec, "env", 1, "HOME=...");
	cfg_addlist(cfg_sec, "params", 1, l->l_path);

	l->l_params = calloc(cfg_size(cfg_sec, "params") + 1, sizeof(char *));
	conf_parse_list(cfg_sec, "params", l->l_params);
	l->l_env = calloc(cfg_size(cfg_sec, "env") + 1, sizeof(char *));
	conf_parse_list(cfg_sec, "env", l->l_env);

	return 0;
}

struct list_head *
conf_load_file(const char *file) {
	int i = 0;
	int j;
	char *secp[] = {"game", "editor", NULL};
	cfg_t *cfg;
	struct list *l;
	struct list_head *headp;

	/* TODO: xmalloc stuff */
	headp = malloc(sizeof headp);
	cfg = cfg_init(opts, CFGF_NONE);
	if (cfg_parse(cfg, file) == CFG_PARSE_ERROR) {
		/* TODO: Error message */
		return NULL;
	}

	options.o_db = strdup(cfg_getstr(cfg, "db"));
	options.o_log = strdup(cfg_getstr(cfg, "log"));

	for (i = 0; secp[i] != NULL; ++i) {
		for (j = 0; cfg_size(cfg, secp[i]); ++j) {
			l = malloc(sizeof(struct list));
			int err = conf_parse_section(cfg, l, secp[i], j);
			if (err == -1) {
				free(l);
				break;
			}
			SLIST_INSERT_HEAD(headp, l, l_next);
		}
	}
	cfg_free(cfg);

	//printf("db: %s\n", options.o_db);
	//printf("log: %s\n", options.o_log);
	//SLIST_FOREACH(l, headp, l_next) {
	//	int i;
	//	printf("%s\n", l->l_name);
	//	//for (i=0;l->l_env[i] != NULL;++i)
	//	//	printf("\t%s\n", l->l_env[i]);
	//	for (i=0;l->l_params[i] != NULL;++i)
	//		printf("\t%i -> %s\n", i, l->l_params[i]);
	//}
	//exit(0);
	return headp;
}

