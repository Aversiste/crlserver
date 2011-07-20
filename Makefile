#	crlserver Makefile

PROG=	crlserver
SRCS=	main.c games.c init.c menu.c

CC=	gcc
CFLAGS=	-Wall -Wextra -ansi -pedantic
LDFLAGS=-lcurses -lutil

.include <bsd.prog.mk>
