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
AR_RC = ar rcs
RANLIB = ranlib
RM_F = rm -f

CDEBUG = -g
LDDEBUG =
CRELEASE = -O
LDRELEASE = -s
CFLAGS = $(CRELEASE)
LDFLAGS = $(LDRELEASE)
LIBS = -lpng -lz -lm

# File extensions
O=.o
A=.a

# Variables
OBJS = pngxread$(O) pngxwrite$(O) \
       pngxrbmp$(O) pngxrgif$(O) pngxrjpg$(O) pngxrpnm$(O) pngxrtif$(O) \
       gifread$(O) \
       pnmerror$(O) pnmread$(O) pnmwrite$(O) \
       minitiff$(O) tiffread$(O) tiffwrite$(O)

# Targets
all: pngxtern$(A)

.c$(O):
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) $<

pngxtern$(A): $(OBJS)
	$(AR_RC) $@ $(OBJS)
	$(RANLIB) $@

gifread$(O): gif/gifread.c gif/gifread.h
	$(CC) -c $(CFLAGS) $<

pnmerror$(O): pnm/pnmerror.c pnm/pnmio.h
	$(CC) -c $(CFLAGS) $<

pnmread$(O): pnm/pnmread.c pnm/pnmio.h
	$(CC) -c $(CFLAGS) $<

pnmwrite$(O): pnm/pnmwrite.c pnm/pnmio.h
	$(CC) -c $(CFLAGS) $<

minitiff$(O): minitiff/minitiff.c minitiff/minitiff.h
	$(CC) -c $(CFLAGS) $<

tiffread$(O): minitiff/tiffread.c minitiff/minitiff.h minitiff/tiffdef.h
	$(CC) -c $(CFLAGS) $<

tiffwrite$(O): minitiff/tiffwrite.c minitiff/minitiff.h minitiff/tiffdef.h
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM_F) *$(O) pngxtern$(A)

pngxread$(O):  pngxread.c pngxtern.h
pngxwrite$(O): pngxwrite.c pngxtern.h
pngxrbmp$(O):  pngxrbmp.c pngxtern.h
pngxrgif$(O):  pngxrgif.c pngxtern.h
pngxrjpg$(O):  pngxrjpg.c pngxtern.h
pngxrpnm$(O):  pngxrpnm.c pngxtern.h
pngxrtif$(O):  pngxrtif.c pngxtern.h

