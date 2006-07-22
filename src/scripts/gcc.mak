# Makefile for OptiPNG
# gcc (generic)
#
# Usage: make -f scripts/gcc.mak


prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
mandir=${prefix}/man
man1dir=${mandir}/man1

CC = gcc
LD = $(CC)
CFLAGS  = -O2 -Wall
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

OBJS = optipng.o opngio.o opngreduc.o cbitset.o osys.o strutil.o
LIBS = $(PNGXDIR)/$(PNGXLIB) $(PNGDIR)/$(PNGLIB) $(ZDIR)/$(ZLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) -o $(OPTIPNG) $(LDFLAGS) $(OBJS) $(LIBS)


.c.o:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) -I$(PNGXDIR) $*.c

optipng.o  : optipng.c proginfo.h opng.h cexcept.h cbitset.h osys.h strutil.h
opngio.o   : opngio.c opng.h
opngreduc.o: opngreduc.c opng.h
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
	-@if [ ! -d ${bindir} ]; then mkdir -p ${bindir}; fi
	-@if [ ! -d ${man1dir} ]; then mkdir -p ${man1dir}; fi
	-@rm -f ${bindir}/$(OPTIPNG)
	-@rm -f ${man1dir}/optipng.1
	cp -p $(OPTIPNG) ${bindir}
	cp -p ../man/optipng.1 ${man1dir}


uninstall:
	rm -f ${bindir}/$(OPTIPNG)
	rm -f ${man1dir}/optipng.1


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
