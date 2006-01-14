# Makefile for pngxtern
# Unix (generic) -- if you have gcc, use gcc.mak instead
#
# Usage: make -f scripts/unix.mak

# Locations of the zlib and libpng libraries and include files
ZDIR   = ../zlib
PNGDIR = ../libpng

# Compiler, linker, librarian and other tools
CC = cc
LD = $(CC)
AR = ar rcs
RANLIB = ranlib
RM = rm -f

CDEBUG = -g
LDDEBUG =
CRELEASE = -O
LDRELEASE =
CFLAGS = -I$(ZDIR) -I$(PNGDIR) -Wall $(CRELEASE)
LDFLAGS = -L. -L$(ZDIR) -L$(PNGDIR) -lpng -lz -lm $(LDRELEASE)

# File extensions
O=.o
A=.a

# Variables
OBJS = pngxread$(O) pngxrbmp$(O) pngxrgif$(O) pngxrpnm$(O) \
       gifread$(O) pnmerror$(O) pnmread$(O) pnmwrite$(O)

# Targets
all: pngxtern$(A)

pngxtern$(A): $(OBJS)
	$(AR) $@ $(OBJS)
	$(RANLIB) $@

gifread$(O): gif/gifread.c gif/gifread.h
	$(CC) $(CFLAGS) -c $<

pnmerror$(O): pnm/pnmerror.c pnm/pnmio.h
	$(CC) $(CFLAGS) -c $<

pnmread$(O): pnm/pnmread.c pnm/pnmio.h
	$(CC) $(CFLAGS) -c $<

pnmwrite$(O): pnm/pnmwrite.c pnm/pnmio.h
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *$(O) pngxtern$(A)

pngxread$(O): pngxtern.h
pngxrbmp$(O): pngxtern.h 
pngxrgif$(O): pngxtern.h 
pngxrpnm$(O): pngxtern.h 

