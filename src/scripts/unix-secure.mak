# Makefile for OptiPNG
# Secure Unix: the latest and greatest zlib and libpng versions and
# security patches are installed in the system
#
# Be aware that compression might be weaker by a tiny fraction!
# If you prefer the slightly better compression provided by a customized
# zlib build, tailored for optimal PNG compression, use unix-std.mak
# or gcc.mak.
#
# Type "optipng -v" to see what libraries are used by the program.
#
# Usage: make -f scripts/unix-secure.mak


prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
mandir=${prefix}/man
man1dir=${mandir}/man1

CC = cc
LD = $(CC)
MAKE = make
CFLAGS  = -O
LDFLAGS = -s

OPTIPNG  = optipng
PNGXLIB  = pngxtern.a
PNGXMAK  = scripts/unix.mak
PNGXDIR  = ../lib/pngxtern
BACKHERE = ../../src

OBJS = optipng.o opngio.o opngreduc.o cbitset.o osys.o strutil.o
LIBS = $(PNGXDIR)/$(PNGXLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) -o $(OPTIPNG) $(LDFLAGS) $(OBJS) $(LIBS) -lpng -lz


.c.o:
	$(CC) -c $(CFLAGS) -DUNIX -I$(PNGXDIR) $*.c

optipng.o  : optipng.c proginfo.h opng.h cexcept.h cbitset.h osys.h strutil.h
opngio.o   : opngio.c opng.h
opngreduc.o: opngreduc.c opng.h
cbitset.o  : cbitset.c cbitset.h
osys.o     : osys.c osys.h
strutil.o  : strutil.c strutil.h


$(PNGXDIR)/$(PNGXLIB):
	cd $(PNGXDIR); \
	$(MAKE) -f $(PNGXMAK) $(PNGXLIB); \
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
