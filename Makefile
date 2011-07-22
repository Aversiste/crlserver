#	crlserver Makefile

PROG=		crlserver
SRCS=		main.c games.c init.c rlmenu.c

CC?=		gcc
CFLAGS+=	-Wall -Wextra -ansi -pedantic -g
LDFLAGS+=	-lcurses -lutil -lmenu

NOMAN=1
.include <bsd.prog.mk>
