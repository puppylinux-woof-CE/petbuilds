CC?=gcc
CFLAGS+=-Wall -pedantic -std=gnu99 $(shell pkg-config --cflags cairo)
LDFLAGS+=-lcairo
PROG=pngoverlay-cairo
INSTALL_PATH=/usr/sbin

all: $(PROG)

$(PROG): $(PROG).o
	$(CC) -o $@ $^ $(LDFLAGS)

$(PROG).o: $(PROG).c
	$(CC) -o $@ $(CFLAGS) -c $^

install:
	install -s -m 0755 $(PROG) $(DESTDIR)$(INSTALL_PATH)
	
uninstall:
	rm $(INSTALL_PATH)/$(PROG)

clean:
	-rm -f *.o $(PROG)
