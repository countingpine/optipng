# Makefile for OptiPNG
# Unix (generic) -- if you have gcc, use gcc.mak instead
#
# Usage: make -f scripts/unix.mak


CC = cc
LD = $(CC)
RM = rm -f
MAKE = make
CFLAGS  = -O
LDFLAGS =

OPTIPNG  = optipng
ZLIB     = libz.a
PNGLIB   = libpng.a
PNGXLIB  = pngxtern.a
ZMAK     = Makefile
PNGMAK   = scripts/makefile.std
PNGXMAK  = scripts/unix.mak
ZDIR     = ../lib/zlib
PNGDIR   = ../lib/libpng
PNGXDIR  = ../lib/pngxtern
BACKHERE = ../../src

OBJS = optipng.o opngio.o opngreduc.o cbitset.o osys.o
LIBS = $(PNGXDIR)/$(PNGXLIB) $(PNGDIR)/$(PNGLIB) $(ZDIR)/$(ZLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) -o $(OPTIPNG) $(LDFLAGS) $(OBJS) $(LIBS)


.c.o:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) -I$(PNGXDIR) $*.c

optipng.o  : optipng.c   opng.h osys.h cbitset.h cexcept.h
opngio.o   : opngio.c    opng.h
opngreduc.o: opngreduc.c opng.h
cbitset.o  : cbitset.c   cbitset.h
osys.o     : osys.c      osys.h


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


clean:
	$(RM) $(OPTIPNG) $(OBJS)
	cd $(PNGXDIR); \
	$(MAKE) -f $(PNGXMAK) clean; \
	cd $(BACKHERE)
	cd $(PNGDIR); \
	$(MAKE) -f $(PNGMAK) clean; \
	cd $(BACKHERE)
	cd $(ZDIR); \
	$(MAKE) -f $(ZMAK) clean; \
	cd $(BACKHERE)
