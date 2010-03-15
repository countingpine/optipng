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
INCS = -I$(ZDIR) -I$(PNGDIR)
LIBS = -lpng -lz -lm

# File extensions
O=.o
A=.a

# Variables
PNGX_OBJS = \
        pngxio$(O) pngxmem$(O) pngxset$(O)
PNGXTERN_OBJS = \
        pngxread$(O) pngxwrite$(O) \
        pngxrbmp$(O) pngxrgif$(O) pngxrjpg$(O) pngxrpnm$(O) pngxrtif$(O)
PNGXTERN_XOBJS = \
        gifread$(O) \
        pnmin$(O) pnmout$(O) pnmutil$(O) \
        minitiff$(O) tiffread$(O) tiffwrite$(O)
OBJS = $(PNGX_OBJS) $(PNGXTERN_OBJS) $(PNGXTERN_XOBJS)

# Targets
all: pngxtern$(A)

.c$(O):
	$(CC) -c $(CFLAGS) $(INCS) $<

pngxtern$(A): $(OBJS)
	$(AR_RC) $@ $(OBJS)
	$(RANLIB) $@

gifread$(O): gif/gifread.c gif/gifread.h
	$(CC) -c $(CFLAGS) $<

pnmin$(O): pnm/pnmin.c pnm/pnmio.h
	$(CC) -c $(CFLAGS) $<

pnmout$(O): pnm/pnmout.c pnm/pnmio.h
	$(CC) -c $(CFLAGS) $<

pnmutil$(O): pnm/pnmutil.c pnm/pnmio.h
	$(CC) -c $(CFLAGS) $<

minitiff$(O): minitiff/minitiff.c minitiff/minitiff.h
	$(CC) -c $(CFLAGS) $<

tiffread$(O): minitiff/tiffread.c minitiff/minitiff.h minitiff/tiffdef.h
	$(CC) -c $(CFLAGS) $<

tiffwrite$(O): minitiff/tiffwrite.c minitiff/minitiff.h minitiff/tiffdef.h
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM_F) *$(O) pngxtern$(A)

pngxio$(O):    pngxio.c pngx.h
pngxmem$(O):   pngxmem.c pngx.h
pngxset$(O):   pngxset.c pngx.h
pngxread$(O):  pngxread.c pngx.h pngxtern.h
pngxwrite$(O): pngxwrite.c pngx.h pngxtern.h
pngxrbmp$(O):  pngxrbmp.c pngx.h pngxtern.h
pngxrgif$(O):  pngxrgif.c pngx.h pngxtern.h gif/gifread.h
pngxrjpg$(O):  pngxrjpg.c pngx.h pngxtern.h
pngxrpnm$(O):  pngxrpnm.c pngx.h pngxtern.h pnm/pnmio.h
pngxrtif$(O):  pngxrtif.c pngx.h pngxtern.h minitiff/minitiff.h

