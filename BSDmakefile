#	crlserver Makefile

PROG =		crlserver
SRCS =		main.c games.c init.c rlmenu.c rlsqlite.c
OSTYPE !=	uname -s


CC ?=		gcc
CFLAGS +=	-Wall -Wextra -ansi -pedantic -I/usr/local/include
CFLAGS += 	-g
CFLAGS +=	-D__${OSTYPE}__
LDFLAGS +=	-L/usr/local/lib -lcurses -lutil -lmenu -lsqlite3

NOMAN=1
.include <bsd.prog.mk>
