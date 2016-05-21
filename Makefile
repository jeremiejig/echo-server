
.SUFFICES:
.SUFFICES: .c .o

OBJ=main.o

# install dir variable

prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin

# Program variable
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL) -m 644

# Target

all: echo-server

echo-server: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $<

clean:
	rm *.o echo-server

install: all installdirs
	$(INSTALL_PROGRAM) echo-server $(DESTDIR)$(bindir)/echo-server

installdirs: mkinstalldirs
	./mkinstalldirs $(DESTDIR)$(bindir)

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall:
	rm $(DESTDIR)$(bindir)/echo-server


.PHONY: all clean
