# Makefile for OptiPNG
# Unix (generic) -- if you have gcc, use gcc.mak instead
#
# Usage: make -f scripts/unix.mak


CC = cc
LD = cc
RM = rm -f
MAKE = make
CFLAGS  = -O
LDFLAGS =

OPTIPNG  = optipng
ZLIB     = libz.a
PNGLIB   = libpng.a
ZMAK     = Makefile
PNGMAK   = scripts/makefile.std
ZDIR     = ../lib/zlib
PNGDIR   = ../lib/libpng
BACKHERE = ../../src

OBJS = optipng.o opngio.o opngreduc.o cbitset.o osys.o
LIBS = $(PNGDIR)/$(PNGLIB) $(ZDIR)/$(ZLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) -o $(OPTIPNG) $(LDFLAGS) $(OBJS) $(LIBS)


.c.o:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) $*.c

optipng.o  : optipng.c   opng.h osys.h cbitset.h cexcept.h
opngio.o   : opngio.c    opng.h
opngreduc.o: opngreduc.c opng.h
cbitset.o  : cbitset.c   cbitset.h
osys.o     : osys.c      osys.h


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
	cd $(PNGDIR); \
	$(MAKE) -f $(PNGMAK) clean; \
	cd $(BACKHERE)
	cd $(ZDIR); \
	$(MAKE) -f $(ZMAK) clean; \
	cd $(BACKHERE)
