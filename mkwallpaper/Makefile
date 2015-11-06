CC?=cc
CFLAGS+=-Wall -pedantic -std=gnu99 $(shell pkg-config --cflags pangocairo)
LDFLAGS+=$(shell pkg-config --libs pangocairo)

all: mkwallpaper

mkwallpaper: mkwallpaper.o
	$(CC) -o $@ $^ $(LDFLAGS)

mkwallpaper.o: mkwallpaper.c
	$(CC) -o $@ $(CFLAGS) -c $^

clean:
	-rm -f *.o mkwallpaper
