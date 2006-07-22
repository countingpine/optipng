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
	pngxread.obj \
	pngxrbmp.obj \
	pngxrgif.obj \
	pngxrjpg.obj \
	pngxrpnm.obj \
	pngxrtif.obj \
	gifread.obj  \
	pnmerror.obj \
	pnmread.obj  \
	pnmwrite.obj \
	minitiff.obj \
	tiffread.obj \
	tiffwrite.obj

LIBOBJS = \
	+pngxread.obj \
	+pngxrbmp.obj \
	+pngxrgif.obj \
	+pngxrjpg.obj \
	+pngxrpnm.obj \
	+pngxrtif.obj \
	+gifread.obj  \
	+pnmerror.obj \
	+pnmread.obj  \
	+pnmwrite.obj \
	+minitiff.obj \
	+tiffread.obj \
	+tiffwrite.obj


## Targets
all: $(LIBNAME)

.c.obj:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) $<

pngxread.obj:  pngxread.c pngxtern.h
pngxwrite.obj: pngxwrite.c pngxtern.h
pngxrbmp.obj:  pngxrbmp.c pngxtern.h
pngxrgif.obj:  pngxrgif.c pngxtern.h gif\gifread.h
pngxrjpg.obj:  pngxrjpg.c pngxtern.h
pngxrpnm.obj:  pngxrpnm.c pngxtern.h pnm\pnmio.h
pngxrtif.obj:  pngxrtif.c pngxtern.h
gifread.obj:   gif\gifread.c gif\gifread.h
pnmerror.obj:  pnm\pnmerror.c pnm\pnmio.h
pnmread.obj:   pnm\pnmread.c pnm\pnmio.h
pnmwrite.obj:  pnm\pnmwrite.c pnm\pnmio.h
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
	-del *.obj
	-del $(LIBNAME)
	-del *.tds


# End of makefile
