#	crlserver Makefile

PROG=		crlserver
SRCS =		main.c conf.c init.c general_menu.c rlmenu.c sqlite.c log.c
SRCS +=		./compat/fparseln.c
OSTYPE=		$(shell uname -s)

CC?=		gcc
CFLAGS +=	-Wall -Wextra -ansi -pedantic -I/usr/local/include
CFLAGS += 	-g
CFLAGS +=	-D__${OSTYPE}__ -I./compat
LDFLAGS+=	-L/usr/local/lib -lcurses -lmenu -lsqlite3 -lbsd

OBJS=	${SRCS:.c=.o}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

${PROG}:	${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

clean:
	rm -f ${OBJS}
