CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99 -g -I/usr/include `pkg-config --cflags glib-2.0 gtk+-2.0`
LDFLAGS = -Wall -s -Os `pkg-config --libs glib-2.0 gtk+-2.0`

all : gtksplash

gtksplash : gtksplash.o
	$(CC) -o $@ $^ $(LDFLAGS)
	
gtksplash.o : gtksplash.c
	$(CC) -o $@ $(CFLAGS) -c $^

clean :
	-rm -f *.o gtksplash
