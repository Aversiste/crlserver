#	crlserver Makefile

PROG=		crlserver
SRCS =		main.c conf.c init.c menus.ci db-sqlite.c log.c aux.c extern.c
OSTYPE=		$(shell uname -s)

CC?=		gcc
CFLAGS +=	-Wall -Wextra -ansi -pedantic -I/usr/local/include
CFLAGS +=	-D__${OSTYPE}__ -I./compat
CFLAGS += 	-I./include
LDFLAGS+=	-L/usr/local/lib -lcurses -lform -lsqlite3 -lbsd -lcrypt

OBJS=	${SRCS:.c=.o}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

${PROG}:	${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

clean:
	rm -f ${OBJS}
