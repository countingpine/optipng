# Makefile for OptiPNG
# gcc (generic)
#
# Usage: make -f scripts/gcc.mak


prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
mandir=$(prefix)/man
man1dir=$(mandir)/man1

CC = gcc
LD = $(CC)
CFLAGS  = -O2 -W -Wall
LDFLAGS = -s

OPTIPNG = optipng$(EXE)
ZLIB    = libz.a
PNGLIB  = libpng.a
PNGXLIB = pngxtern.a
ZMAK    = Makefile
PNGMAK  = scripts/makefile.gcc
PNGXMAK = scripts/gcc.mak
ZDIR    = ../lib/zlib
PNGDIR  = ../lib/libpng
PNGXDIR = ../lib/pngxtern
BACKDIR = ../../src

OBJS = optipng.o opngoptim.o opngreduc.o cbitset.o osys.o
INCS = -I$(ZDIR) -I$(PNGDIR) -I$(PNGXDIR)
LIBS = $(PNGXDIR)/$(PNGXLIB) $(PNGDIR)/$(PNGLIB) $(ZDIR)/$(ZLIB)
SYSLIBS = -lm


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) -o $(OPTIPNG) $(LDFLAGS) $(OBJS) $(LIBS) $(SYSLIBS)


.c.o:
	$(CC) -c $(CFLAGS) $(INCS) $*.c

optipng.o  : optipng.c proginfo.h optipng.h cbitset.h osys.h
opngoptim.o: opngoptim.c optipng.h opngreduc.h cexcept.h cbitset.h osys.h
opngreduc.o: opngreduc.c opngreduc.h
cbitset.o  : cbitset.c cbitset.h
osys.o     : osys.c osys.h


$(PNGXDIR)/$(PNGXLIB): $(ZDIR)/$(ZLIB) $(PNGDIR)/$(PNGLIB)
	cd $(PNGXDIR); \
	$(MAKE) -f $(PNGXMAK) $(PNGXLIB); \
	cd $(BACKDIR)

$(PNGDIR)/$(PNGLIB): $(ZDIR)/$(ZLIB)
	cd $(PNGDIR); \
	$(MAKE) -f $(PNGMAK) $(PNGLIB); \
	cd $(BACKDIR)

$(ZDIR)/$(ZLIB):
	cd $(ZDIR); \
	$(MAKE) -f $(ZMAK) $(ZLIB); \
	cd $(BACKDIR)


install: $(OPTIPNG)
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(man1dir)
	-@rm -f $(DESTDIR)$(bindir)/$(OPTIPNG)
	-@rm -f $(DESTDIR)$(man1dir)/optipng.1
	cp -p $(OPTIPNG) $(DESTDIR)$(bindir)
	cp -p ../man/optipng.1 $(DESTDIR)$(man1dir)


uninstall:
	rm -f $(DESTDIR)$(bindir)/$(OPTIPNG)
	rm -f $(DESTDIR)$(man1dir)/optipng.1


clean:
	rm -f $(OPTIPNG) $(OBJS)
	cd $(PNGXDIR); \
	$(MAKE) -f $(PNGXMAK) clean; \
	cd $(BACKDIR)
	cd $(PNGDIR); \
	$(MAKE) -f $(PNGMAK) clean; \
	cd $(BACKDIR)
	cd $(ZDIR); \
	$(MAKE) -f $(ZMAK) clean; \
	cd $(BACKDIR)
