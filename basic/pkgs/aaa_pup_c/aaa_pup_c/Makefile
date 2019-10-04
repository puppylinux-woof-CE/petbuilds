CC=gcc
INSTALL_PATH=/usr/sbin

CAIRO_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config cairo --cflags --libs)
GTK2_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config gtk+-2.0 --cflags --libs)
GTK3_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config gtk+-3.0 --cflags --libs)

all: pngoverlay-cairo \
	pngoverlay-gtk2 \
	pngoverlay \
	picscale

pngoverlay-cairo:
	$(CC) -o pngoverlay-cairo $(CFLAFS) $(CAIRO_FLAGS) gtk/pngoverlay-cairo.c

pngoverlay-gtk2:
	$(CC) -o pngoverlay-gtk2 $(CFLAFS) $(GTK2_FLAGS) gtk/pngoverlay-gtk2.c

pngoverlay:
	ln -snf pngoverlay-cairo pngoverlay

picscale:
	$(CC) -o picscale $(CFLAFS) $(GTK2_FLAGS) gtk/picscale.c

install:
	install -s -m 0755 pngoverlay-cairo $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pngoverlay-gtk2 $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pngoverlay $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 picscale $(DESTDIR)$(INSTALL_PATH)

uninstall:
	rm $(INSTALL_PATH)/pngoverlay-cairo
	rm $(INSTALL_PATH)/pngoverlay-gtk2
	rm $(INSTALL_PATH)/pngoverlay
	rm $(INSTALL_PATH)/picscale

clean:
	-find -name '*.o' -delete
	-rm -f pngoverlay pngoverlay-gtk2 pngoverlay-cairo picscale
