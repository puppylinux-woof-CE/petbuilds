CC = gcc
FLAGS = -Wall `pkg-config --libs --cflags gtk+-2.0`
PROG = x-key-status
SCR = x_key_status_start
PREFIX=$(DESTDIR)/usr

all : $(PROG)

$(PROG) : $(PROG).c
	$(CC) -o $@ $^ $(FLAGS)
	xgettext --keyword="_" $(PROG).c -o po/$(PROG).pot 
	
clean :
	rm -f $(PROG) po/*
	
install :
	install -d -m 0755 $(PREFIX)/bin
	install -s -m 0755 $(PROG) $(PREFIX)/bin
	install -m 0755 $(SCR) $(PREFIX)/bin
	
uninstall :
	rm -f $(PREFIX)/bin/$(PROG)
