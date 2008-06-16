# Makefile for OptiPNG
# gcc (generic)
#
# If you are on Unix and you wish to use the system-supplied
# zlib and libpng libraries (e.g. for security purposes),
# use unix-secure.mak CC="gcc" CFLAGS="-O2 -W -Wall"
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

OPTIPNG  = optipng$(EXE)
ZLIB     = libz.a
PNGLIB   = libpng.a
PNGXLIB  = pngxtern.a
ZMAK     = Makefile
PNGMAK   = scripts/makefile.gcc
PNGXMAK  = scripts/gcc.mak
ZDIR     = ../lib/zlib
PNGDIR   = ../lib/libpng
PNGXDIR  = ../lib/pngxtern
BACKHERE = ../../src

OBJS = optipng.o opngreduc.o cbitset.o osys.o strutil.o
LIBS = $(PNGXDIR)/$(PNGXLIB) $(PNGDIR)/$(PNGLIB) $(ZDIR)/$(ZLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) -o $(OPTIPNG) $(LDFLAGS) $(OBJS) $(LIBS)


.c.o:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) -I$(PNGXDIR) $*.c

optipng.o  : optipng.c proginfo.h opngreduc.h \
             cexcept.h cbitset.h osys.h strutil.h
opngreduc.o: opngreduc.c opngreduc.h
cbitset.o  : cbitset.c cbitset.h
osys.o     : osys.c osys.h
strutil.o  : strutil.c strutil.h


$(PNGXDIR)/$(PNGXLIB): $(ZDIR)/$(ZLIB) $(PNGDIR)/$(PNGLIB)
	cd $(PNGXDIR); \
	$(MAKE) -f $(PNGXMAK) $(PNGXLIB); \
	cd $(BACKHERE)

$(PNGDIR)/$(PNGLIB): $(ZDIR)/$(ZLIB)
	cd $(PNGDIR); \
	$(MAKE) -f $(PNGMAK) $(PNGLIB); \
	cd $(BACKHERE)

$(ZDIR)/$(ZLIB):
	cd $(ZDIR); \
	./configure; \
	$(MAKE) -f $(ZMAK) $(ZLIB); \
	cd $(BACKHERE)


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
	cd $(BACKHERE)
	cd $(PNGDIR); \
	$(MAKE) -f $(PNGMAK) clean; \
	cd $(BACKHERE)
	cd $(ZDIR); \
	$(MAKE) -f $(ZMAK) clean; \
	cd $(BACKHERE)
