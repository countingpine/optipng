# Makefile for pngxtern
# Borland C++ for Win32
#
# Usage: make -f scripts\bcc32.mak

CC  = bcc32
LIB = tlib

ZDIR   = ..\zlib
PNGDIR = ..\libpng

!ifdef DEBUG
CDEBUG = -v
LDEBUG = -v
!else
CDEBUG =
LDEBUG =
!endif

# STACKOFLOW = 1
!ifdef STACKOFLOW
CDEBUG = $(CDEBUG) -N
LDEBUG = $(LDEBUG) -N
!endif

# -O2 optimize for speed
# -d  merge duplicate strings
# -k- turn off standard stack frame
# -w  display all warnings
CFLAGS = -O2 -d -k- -w $(CDEBUG)

# -M  generate map file
LDFLAGS = -M $(LDEBUG)

LIBNAME = pngxtern.lib


## Variables
OBJS = \
	pngxio.obj    \
	pngxmem.obj   \
	pngxset.obj   \
	pngxread.obj  \
	pngxwrite.obj \
	pngxrbmp.obj  \
	pngxrgif.obj  \
	pngxrjpg.obj  \
	pngxrpnm.obj  \
	pngxrtif.obj  \
	gifread.obj   \
	pnmin.obj     \
	pnmout.obj    \
	pnmutil.obj   \
	minitiff.obj  \
	tiffread.obj  \
	tiffwrite.obj

LIBOBJS = \
	+pngxio.obj    \
	+pngxmem.obj   \
	+pngxset.obj   \
	+pngxread.obj  \
	+pngxwrite.obj \
	+pngxrbmp.obj  \
	+pngxrgif.obj  \
	+pngxrjpg.obj  \
	+pngxrpnm.obj  \
	+pngxrtif.obj  \
	+gifread.obj   \
	+pnmin.obj     \
	+pnmout.obj    \
	+pnmutil.obj   \
	+minitiff.obj  \
	+tiffread.obj  \
	+tiffwrite.obj


## Targets
all: $(LIBNAME)

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
pngxrtif.obj:  pngxrtif.c pngx.h pngxtern.h
gifread.obj:   gif\gifread.c gif\gifread.h
pnmin.obj:     pnm\pnmin.c pnm\pnmio.h
pnmout.obj:    pnm\pnmout.c pnm\pnmio.h
pnmutil.obj:   pnm\pnmutil.c pnm\pnmio.h
minitiff.obj:  minitiff\minitiff.c minitiff\minitiff.h
tiffread.obj:  minitiff\tiffread.c minitiff\minitiff.h minitiff\tiffdef.h
tiffwrite.obj: minitiff\tiffwrite.c minitiff\minitiff.h minitiff\tiffdef.h


$(LIBNAME): $(OBJS)
	-del $(LIBNAME)
	$(LIB) $(LIBNAME) @&&|
$(LIBOBJS)
|


## Cleanup
clean:
	-del $(LIBNAME)
	-del *.obj
	-del *.tds


# End of makefile for pngxtern
