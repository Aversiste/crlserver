#	crlserver Makefile

#	SQLITE_FLAVOR = Compile with support for sqlite. It's actualy required.

PROG =		crlserver
SRCS =		main.c conf.c init.c menus.c sqlite.c log.c aux.c
OSTYPE !=	uname -s

CC ?=		gcc
CFLAGS +=	-Wall -Wextra -ansi -pedantic -I/usr/local/include
CFLAGS += 	-g
CFLAGS +=	-D__${OSTYPE}__ -DSQLITE_FLAVOR
LDFLAGS +=	-L/usr/local/lib -lcurses -lutil -lsqlite3 -lform

NOMAN=1

.include <bsd.prog.mk>
