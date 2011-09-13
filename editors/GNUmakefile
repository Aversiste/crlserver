# ee and virus

CC = gcc
CFLAGS += -Wall -Wextra
CFLAGS += -Os -fexpensive-optimizations -fno-common -fverbose-asm -static
LDFLAGS += -lcurses -lbsd

all:	ee virus

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

ee:	ee.o
	$(CC) $(LDFLAGS) -o ee ee.o
	strip ee

virus: virus.o
	$(CC) $(LDFLAGS) -o virus virus.o
	strip virus

clean:
	rm -f ee.o virus.o core *.core

distclean: clean
	rm -f ee virus
