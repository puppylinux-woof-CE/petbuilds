CC=gcc
INSTALL_PATH=/usr/sbin
INSTALL_PATH2=/usr/bin
INSTALL_XDG_START=/root/.config/autostart

CAIRO_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config cairo --cflags --libs)
PANGOCAIRO_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config pangocairo --cflags --libs)
GTK2_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config gtk+-2.0 --cflags --libs)
GTK3_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config gtk+-3.0 --cflags --libs)
X11_FLAGS=-Wall -pedantic -std=gnu99 $(shell pkg-config x11 --libs)

all: freememapplet \
	getcurpos \
	gtksplash \
	mkwallpaper \
	mpscan \
	pm13tray \
	pmusic_trayapp \
	pngoverlay-cairo \
	pngoverlay-gtk2 \
	picscale \
	setvol \
	switch2

freememapplet:
	xgettext --keyword="_" gtk/freememapplet/freememapplet.c -o freememapplet.pot
	$(CC) -o freememapplet $(CFLAFS) $(GTK2_FLAGS) gtk/freememapplet/freememapplet.c

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
	ln -snf pngoverlay-cairo pngoverlay
	$(CC) -o pngoverlay-gtk2 $(CFLAFS) $(GTK2_FLAGS) gtk/pngoverlay-gtk2.c

picscale:
	$(CC) -o picscale $(CFLAFS) $(GTK2_FLAGS) gtk/picscale.c

setvol:
	$(CC) -o setvol $(CFLAFS) $(GTK2_FLAGS) cli/setvol.c

switch2:
	$(CC) -o switch2 $(CFLAFS) $(GTK2_FLAGS) gtk/switch2.c

install:
	#--
	install -d $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 freememapplet $(DESTDIR)$(INSTALL_PATH)
	install -m 0755 gtk/freememapplet/freememappletshell $(DESTDIR)$(INSTALL_PATH)
	ln -snf freememappletshell $(DESTDIR)$(INSTALL_PATH)/freememapplet_tray
	install -d $(DESTDIR)$(INSTALL_XDG_START)
	install -m 0644 gtk/freememapplet/freememapplet_tray.desktop $(DESTDIR)$(INSTALL_XDG_START)
	#--
	install -s -m 0755 getcurpos $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 gtksplash $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 mkwallpaper $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 mpscan $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 pm13tray $(DESTDIR)$(INSTALL_PATH)
	#--
	install -d $(DESTDIR)/usr/local/pmusic/plugins
	install -s -m 0755 pmusic_trayapp $(DESTDIR)/usr/local/pmusic/plugins
	install -m 644 gtk/pmusic_trayapp.rc $(DESTDIR)/usr/local/pmusic/plugins
	#--
	install -s -m 0755 pngoverlay-cairo $(DESTDIR)$(INSTALL_PATH)
	ln -snf pngoverlay-cairo $(DESTDIR)$(INSTALL_PATH)/pngoverlay
	#--
	install -s -m 0755 pngoverlay-gtk2 $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 picscale $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 setvol $(DESTDIR)$(INSTALL_PATH)
	install -s -m 0755 switch2 $(DESTDIR)$(INSTALL_PATH)

uninstall:
	#--
	rm $(INSTALL_PATH)/freememapplet
	rm $(INSTALL_PATH)/freememapplet_tray
	rm $(INSTALL_PATH)/freememappletshell
	rm $(INSTALL_XDG_START)/freememapplet_tray.desktop
	#--
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
	rm $(INSTALL_PATH)/setvol
	rm $(INSTALL_PATH)/switch2

clean:
	-find -name '*.o' -delete
	-rm -f freememapplet* \
	getcurpos \
	gtksplash \
	mkwallpaper \
	pm13tray \
	mpscan \
	pmusic_trayapp \
	pngoverlay \
	pngoverlay-gtk2 \
	pngoverlay-cairo \
	picscale \
	setvol \
	switch2
