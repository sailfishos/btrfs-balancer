SHELL		= /bin/sh

PREFIX		?= /usr
EXEC_PREFIX	?= $(PREFIX)
BINDIR		?= $(EXEC_PREFIX)/bin
SBINDIR		?= $(EXEC_PREFIX)/sbin
LIBDIR		?= $(EXEC_PREFIX)/lib
DATADIR		?= $(PREFIX)/share
INCLUDEDIR	?= $(PREFIX)/include
UNITDIR		?= /lib/systemd/system

PROGRAM		:= btrfs-balancer

# pkg-config dependencies here
PKGC_LIBS	:= glib-2.0 dbus-glib-1 keepalive-glib

# pkg-config flags
PKGC_CFLAGS	:= $(shell pkg-config --cflags $(PKGC_LIBS))
PKGC_LDFLAGS	:= $(shell pkg-config --libs $(PKGC_LIBS))

OPTFLAGS	?= -D_FILE_OFFSET_BITS=64
CFLAGS		?= -g -Wall
CFLAGS		+= $(OPTFLAGS)
CFLAGS		+= $(PKGC_CFLAGS)
LDFLAGS		+= $(PKGC_LDFLAGS)
CXXFLAGS	+= -std=c++0x $(CFLAGS)

all: $(PROGRAM)

distclean: clean

clean:
	$(RM) *.o $(PROGRAM) *~

install: all
	install -d $(DESTDIR)/$(SBINDIR)
	install $(PROGRAM) $(DESTDIR)/$(SBINDIR)
	install -d $(DESTDIR)/$(UNITDIR)
	install -t $(DESTDIR)/$(UNITDIR) service/btrfs-balance.service service/btrfs-balancer.timer
	install -d $(DESTDIR)/$(UNITDIR)/multi-user.target.wants
	ln -s ../btrfs-balancer.timer $(DESTDIR)/$(UNITDIR)/multi-user.target.wants/btrfs-balancer.timer


.PHONY: all clean distclean install
