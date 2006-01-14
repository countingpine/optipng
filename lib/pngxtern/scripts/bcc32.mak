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

# -X- turn on dependency generation in the object file
# -w  set all warnings on
# -O2 optimize for speed
# -Z  global optimization
CFLAGS = -O2 -Z -X- -w -I$(ZDIR) -I$(PNGDIR) $(CDEBUG)

# -M  generate map file
LDFLAGS = -M -L$(ZDIR) $(LDEBUG)

LIBNAME = pngxtern.lib


## Variables
OBJS = \
	pngxread.obj \
	pngxrbmp.obj \
	pngxrgif.obj \
	pngxrpnm.obj \
	gifread.obj  \
	pnmerror.obj \
	pnmread.obj  \
	pnmwrite.obj

LIBOBJS = \
	+pngxread.obj \
	+pngxrbmp.obj \
	+pngxrgif.obj \
	+pngxrpnm.obj \
	+gifread.obj  \
	+pnmerror.obj \
	+pnmread.obj  \
	+pnmwrite.obj


## Targets
all: $(LIBNAME)

.c.obj:
	$(CC) -c $(CFLAGS) $<

pngxread.obj: pngxread.c pngxtern.h
pngxrbmp.obj: pngxrbmp.c pngxtern.h
pngxrgif.obj: pngxrgif.c pngxtern.h gif\gifread.h
pngxrpnm.obj: pngxrpnm.c pngxtern.h pnm\pnmio.h
gifread.obj:  gif\gifread.c  gif\gifread.h
pnmerror.obj: pnm\pnmerror.c pnm\pnmio.h
pnmread.obj:  pnm\pnmread.c  pnm\pnmio.h
pnmwrite.obj: pnm\pnmwrite.c pnm\pnmio.h


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
