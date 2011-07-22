#	crlserver Makefile

PROG=		crlserver
SRCS=		main.c games.c init.c rlmenu.c rlsqlite.c

CC?=		gcc
CFLAGS+=	-Wall -Wextra -ansi -pedantic -g -I/usr/local/include
LDFLAGS+=	-L/usr/local/lib -lcurses -lutil -lmenu -lsqlite3

NOMAN=1
.include <bsd.prog.mk>
