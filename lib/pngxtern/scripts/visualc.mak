# Makefile for pngxtern
# Microsoft Visual C++
#
# Usage: nmake -f scripts\visualc.mak

# Compiler, linker, librarian and other tools
CC = cl
LD = link
AR = lib
CFLAGS  = -nologo -MD -O2 -W3 -D_CRT_SECURE_NO_WARNINGS
LDFLAGS = -nologo
ARFLAGS = -nologo
RM = del

ZDIR   = ..\zlib
PNGDIR = ..\libpng

# Variables
PNGX_OBJS = \
        pngxio.obj pngxmem.obj pngxset.obj
PNGXTERN_OBJS = \
        pngxread.obj pngxwrite.obj \
        pngxrbmp.obj pngxrgif.obj pngxrjpg.obj pngxrpnm.obj pngxrtif.obj
PNGXTERN_XOBJS = \
        gifread.obj \
        pnmin.obj pnmout.obj pnmutil.obj \
        minitiff.obj tiffread.obj tiffwrite.obj
OBJS = $(PNGX_OBJS) $(PNGXTERN_OBJS) $(PNGXTERN_XOBJS)

# Targets
all: pngxtern.lib

pngxtern.lib: $(OBJS)
	-$(RM) $@
	$(AR) $(ARFLAGS) -out:$@ $(OBJS)

.c.obj:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) $<

pngxio.obj:    pngxio.c pngx.h
pngxmem.obj:   pngxmem.c pngx.h
pngxset.obj:   pngxset.c pngx.h
pngxread.obj:  pngxread.c pngx.h pngxtern.h
pngxwrite.obj: pngxwrite.c pngx.h pngxtern.h
pngxrbmp.obj:  pngxrbmp.c pngx.h pngxtern.h
pngxrgif.obj:  pngxrgif.c pngx.h pngxtern.h gif\gifread.h
pngxrjpg.obj:  pngxrjpg.c pngx.h pngxtern.h
pngxrpnm.obj:  pngxrpnm.c pngx.h pngxtern.h pnm\pnmio.h
pngxrtif.obj:  pngxrtif.c pngx.h pngxtern.h minitiff\minitiff.h

gifread.obj:   gif\gifread.c gif\gifread.h
	$(CC) -c $(CFLAGS) gif\gifread.c

pnmin.obj:     pnm\pnmin.c pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmin.c

pnmout.obj:    pnm\pnmout.c pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmout.c

pnmutil.obj:   pnm\pnmutil.c pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmutil.c

minitiff.obj:  minitiff\minitiff.c minitiff\minitiff.h
	$(CC) -c $(CFLAGS) minitiff\minitiff.c

tiffread.obj:  minitiff\tiffread.c minitiff\minitiff.h minitiff\tiffdef.h
	$(CC) -c $(CFLAGS) minitiff\tiffread.c

tiffwrite.obj: minitiff\tiffwrite.c minitiff\minitiff.h minitiff\tiffdef.h
	$(CC) -c $(CFLAGS) minitiff\tiffwrite.c

clean:
	-$(RM) pngxtern.lib
	-$(RM) *.obj

# End of makefile for pngxtern
