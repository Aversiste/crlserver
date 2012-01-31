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

#ifndef PATHNAMES_H__
#define PATHNAMES_H__

/*
 * This a read only directory that contains crlserver's main config file
 * and two sub-directory, 'menus' and 'misc'.
 * The first one contains a series of text files required to indiquate
 * actions to the users.
 * The second one contains the different configuration files required
 * by the games (eg: nethackrc). Don't put dot (".") before them.
 */
#define CRLSERVER_CONFIG_DIR "/etc/crlserver"

/*
 * The playground directory is read and write, because it contains the
 * database, the log file and 'userdata', the users' personals directories
 */
#define CRLSERVER_PLAYGROUND "path/to/crlserver/playground"
#define CRLSERVER_LOG_FILE "crlserver.log"
#define CRLSERVER_DATABASE "crlserver.db"

#endif
