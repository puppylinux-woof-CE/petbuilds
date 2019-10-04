CC=gcc
INSTALL_PATH=/usr/sbin

CAIRO_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config cairo --cflags --libs)
PANGOCAIRO_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config pangocairo --cflags --libs)
GTK2_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config gtk+-2.0 --cflags --libs)
GTK3_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config gtk+-3.0 --cflags --libs)
X11_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config x11 --libs)

all: getcurpos \
	gtksplash \
	mkwallpaper \
	mpscan \
	pm13tray \
	pmusic_trayapp \
	pngoverlay-cairo \
	pngoverlay-gtk2 \
	pngoverlay \
	picscale

getcurpos:
	$(CC) -o getcurpos $(CFLAFS) $(X11_FLAGS) x/getcurpos.c

gtksplash:
	$(CC) -o gtksplash $(CFLAFS) $(GTK2_FLAGS) gtk/gtksplash.c

mkwallpaper:
	$(CC) -o mkwallpaper $(CFLAFS) $(PANGOCAIRO_FLAGS) gtk/mkwallpaper.c

mpscan:
	$(CC) -o mpscan $(CFLAFS) cli/mpscan.c

pm13tray:
	$(CC) -o pm13tray $(CFLAFS) $(GTK2_FLAGS) gtk/pngoverlay-gtk2.c

pmusic_trayapp:
	$(CC) -o pmusic_trayapp $(CFLAFS) $(GTK2_FLAGS) gtk/pmusic_trayapp.c

pngoverlay-cairo:
	$(CC) -o pngoverlay-cairo $(CFLAFS) $(CAIRO_FLAGS) gtk/pngoverlay-cairo.c

pngoverlay-gtk2:
	$(CC) -o pngoverlay-gtk2 $(CFLAFS) $(GTK2_FLAGS) gtk/pngoverlay-gtk2.c

pngoverlay:
	ln -snf pngoverlay-cairo pngoverlay

picscale:
	$(CC) -o picscale $(CFLAFS) $(GTK2_FLAGS) gtk/picscale.c

install:
	install -s -m 0755 getcurpos $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 gtksplash $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 mkwallpaper $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 mpscan $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pm13tray $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pmusic_trayapp $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pngoverlay-cairo $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pngoverlay-gtk2 $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pngoverlay $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 picscale $(DESTDIR)$(INSTALL_PATH)

uninstall:
	rm $(INSTALL_PATH)/getcurpos
	rm $(INSTALL_PATH)/gtksplash
	rm $(INSTALL_PATH)/mkwallpaper
	rm $(INSTALL_PATH)/pm13tray
	rm $(INSTALL_PATH)/mpscan
	rm $(INSTALL_PATH)/pmusic_trayapp
	rm $(INSTALL_PATH)/pngoverlay-cairo
	rm $(INSTALL_PATH)/pngoverlay-gtk2
	rm $(INSTALL_PATH)/pngoverlay
	rm $(INSTALL_PATH)/picscale

clean:
	-find -name '*.o' -delete
	-rm -f getcurpos \
	gtksplash \
	mkwallpaper \
	pm13tray \
	mpscan \
	pmusic_trayapp \
	pngoverlay \
	pngoverlay-gtk2 \
	pngoverlay-cairo \
	picscale
