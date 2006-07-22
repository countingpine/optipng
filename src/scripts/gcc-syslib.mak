# Makefile for OptiPNG
# gcc (generic) -- using the system-supplied zlib
#
# Use this makefile if you have a reason to use the system-supplied zlib
# (e.g. if you want to link zlib in dynamically)
# Be aware that compression might be weaker by a tiny fraction!
#
# Otherwise, just use the regular makefile (gcc.mak)
#
# Sorry, right now, there's no easy way to use the system-supplied libpng!
# (send email to the author if you need to know more on this issue)
#
# Type "optipng -v" to see what libraries are used by an already-built program
#
# Usage: make -f scripts/gcc-syslib.mak


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
PNGLIB   = libpng.a
PNGXLIB  = pngxtern.a
PNGMAK   = scripts/makefile.gcc
PNGXMAK  = scripts/gcc.mak
PNGDIR   = ../lib/libpng
PNGXDIR  = ../lib/pngxtern
BACKHERE = ../../src

OBJS = optipng.o opngio.o opngreduc.o cbitset.o osys.o strutil.o
LIBS = $(PNGXDIR)/$(PNGXLIB) $(PNGDIR)/$(PNGLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) -o $(OPTIPNG) $(LDFLAGS) $(OBJS) $(LIBS) -lz


.c.o:
	$(CC) -c $(CFLAGS) -I$(PNGDIR) -I$(PNGXDIR) $*.c

optipng.o  : optipng.c proginfo.h opng.h cexcept.h cbitset.h osys.h strutil.h
opngio.o   : opngio.c opng.h
opngreduc.o: opngreduc.c opng.h
cbitset.o  : cbitset.c cbitset.h
osys.o     : osys.c osys.h
strutil.o  : strutil.c strutil.h


$(PNGXDIR)/$(PNGXLIB): $(PNGDIR)/$(PNGLIB)
	cd $(PNGXDIR); \
	$(MAKE) -f $(PNGXMAK) $(PNGXLIB); \
	cd $(BACKHERE)

$(PNGDIR)/$(PNGLIB):
	cd $(PNGDIR); \
	$(MAKE) -f $(PNGMAK) $(PNGLIB); \
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
