# Makefile for OptiPNG
# gcc (generic)
#
# Usage: make -f scripts/gcc.mak


CC = gcc
LD = gcc
RM = rm -f
CFLAGS  = -O2 -Wall
LDFLAGS = -s

OPTIPNG  = optipng$(EXE)
ZLIB     = libz.a
PNGLIB   = libpng.a
ZMAK     = Makefile
PNGMAK   = scripts/makefile.gcc
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
