#	crlserver Makefile

#	SQLITE_FLAVOR = Compile with support for sqlite. It's actualy required.

PROG=		crlserver
SRCS =		main.c conf.c init.c menus.c sqlite.c log.c
SRCS +=		./compat/fparseln.c
OSTYPE=		$(shell uname -s)

CC?=		gcc
CFLAGS +=	-Wall -Wextra -ansi -pedantic -I/usr/local/include
CFLAGS += 	-g
CFLAGS +=	-D__${OSTYPE}__ -I./compat
CFLAGS +=	-DSQLITE_FLAVOR
LDFLAGS+=	-L/usr/local/lib -lcurses -lform -lsqlite3 -lbsd

OBJS=	${SRCS:.c=.o}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

${PROG}:	${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

clean:
	rm -f ${OBJS}
