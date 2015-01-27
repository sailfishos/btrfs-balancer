SHELL		= /bin/sh

PREFIX		?= /usr
EXEC_PREFIX	?= $(PREFIX)
BINDIR		?= $(EXEC_PREFIX)/bin
SBINDIR		?= $(EXEC_PREFIX)/sbin
LIBDIR		?= $(EXEC_PREFIX)/lib
DATADIR		?= $(PREFIX)/share
INCLUDEDIR	?= $(PREFIX)/include
UNITDIR		?= /lib/systemd/system

PACKAGE		= btrfs-balancer-`git describe --tags`


all:

distclean: clean
	$(RM) -rf $(PACKAGE)*
clean:
	$(RM) *~

install: all
	install -d $(DESTDIR)/$(SBINDIR)
	install scripts/btrfs-balance $(DESTDIR)/$(SBINDIR)
	install -d $(DESTDIR)/$(UNITDIR)
	install -t $(DESTDIR)/$(UNITDIR) service/btrfs-balance.service service/btrfs-balancer.timer
	install -d $(DESTDIR)/$(UNITDIR)/multi-user.target.wants
	ln -s ../btrfs-balancer.timer $(DESTDIR)/$(UNITDIR)/multi-user.target.wants/btrfs-balancer.timer



dist: clean
	git archive --format=tar.gz --prefix=$(PACKAGE)/ -o $(PACKAGE).tar.gz master

.PHONY: all clean distclean install dist
