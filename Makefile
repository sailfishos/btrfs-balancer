SHELL		= /bin/sh

PREFIX		?= /usr
EXEC_PREFIX	?= $(PREFIX)
BINDIR		?= $(EXEC_PREFIX)/bin
SBINDIR		?= $(EXEC_PREFIX)/sbin
LIBDIR		?= $(EXEC_PREFIX)/lib
DATADIR		?= $(PREFIX)/share
INCLUDEDIR	?= $(PREFIX)/include
UNITDIR		?= /lib/systemd/system

PROGRAM		= keepalive
PACKAGE		= $(PROGRAM)-`git describe --tags`

# pkg-config dependencies here
PKGC_LIBS	= glib-2.0
PKGC_LIBS	+= gio-2.0
PKGC_LIBS	+= mce

# pkg-config flags
PKGC_CFLAGS	= `pkg-config --cflags $(PKGC_LIBS)`
PKGC_LDFLAGS	= `pkg-config --libs $(PKGC_LIBS)`

OPTFLAGS	?= -D_FILE_OFFSET_BITS=64
CFLAGS		?= -g -Wall
CFLAGS		+= $(OPTFLAGS)
CFLAGS		+= $(PKGC_CFLAGS)
LDFLAGS		+= $(PKGC_LDFLAGS)

all: $(PROGRAM)

distclean: clean
	$(RM) -rf $(PACKAGE)*
clean:
	$(RM) *.o $(PROGRAM) *~

install: all
	install -d $(DESTDIR)/$(BINDIR)
	install $(PROGRAM) $(DESTDIR)/$(BINDIR)
	install -d $(DESTDIR)/$(SBINDIR)
	install scripts/btrfs-balance $(DESTDIR)/$(SBINDIR)
	install -d $(DESTDIR)/$(UNITDIR)
	install -t $(DESTDIR)/$(UNITDIR) service/btrfs-balance.service service/btrfs-balancer.timer
	install -d $(DESTDIR)/$(UNITDIR)/multi-user.target.wants
	ln -s ../btrfs-balancer.timer $(DESTDIR)/$(UNITDIR)/multi-user.target.wants/btrfs-balancer.timer



dist: clean
	rm -rf $(PACKAGE)
	mkdir $(PACKAGE)
	cp -a *.c Makefile $(PACKAGE)
	(cd $(PACKAGE); find . -name .svn -o -name *~ | xargs rm -rf)
	tar cjf $(PACKAGE).tar.bz2 $(PACKAGE)
	rm -rf $(PACKAGE)

.PHONY: all clean distclean install dist
