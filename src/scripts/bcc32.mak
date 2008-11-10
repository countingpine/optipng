# Makefile for OptiPNG
# Borland C++ for Win32
#
# Usage: make -f scripts\bcc32.mak


CC = bcc32
LD = $(CC)
CFLAGS  = -O2 -v -w
LDFLAGS = -v

OPTIPNG = optipng.exe
ZLIB    = zlib.lib
PNGLIB  = libpng.lib
PNGXLIB = pngxtern.lib
ZMAK    = win32\Makefile.bor
PNGMAK  = scripts\makefile.bc32
PNGXMAK = scripts\bcc32.mak
ZDIR    = ..\lib\zlib
PNGDIR  = ..\lib\libpng
PNGXDIR = ..\lib\pngxtern
BACKDIR = ..\..\src

OBJS = optipng.obj opngoptim.obj opngreduc.obj \
       cbitset.obj osys.obj strutil.obj wildargs.obj
INCS = -I$(ZDIR) -I$(PNGDIR) -I$(PNGXDIR)
LIBS = $(PNGXDIR)\$(PNGXLIB) $(PNGDIR)\$(PNGLIB) $(ZDIR)\$(ZLIB)
SYSLIBS = #noeh32.lib


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) $(SYSLIBS)


.c.obj:
	$(CC) -c $(CFLAGS) $(INCS) $*.c

optipng.obj  : optipng.c proginfo.h optipng.h cbitset.h osys.h strutil.h
opngoptim.obj: opngoptim.c optipng.h opngreduc.h cexcept.h cbitset.h osys.h
opngreduc.obj: opngreduc.c opngreduc.h
cbitset.obj  : cbitset.c cbitset.h
osys.obj     : osys.c osys.h
strutil.obj  : strutil.c strutil.h

wildargs.obj : xtra\wildargs.c
	$(CC) -c $(CFLAGS) xtra\wildargs.c


$(PNGXDIR)\$(PNGXLIB): $(ZDIR)\$(ZLIB) $(PNGDIR)\$(PNGLIB)
	cd $(PNGXDIR)
	$(MAKE) -f $(PNGXMAK) $(PNGXLIB)
	cd $(BACKDIR)

$(PNGDIR)\$(PNGLIB): $(ZDIR)\$(ZLIB)
	cd $(PNGDIR)
	$(MAKE) -f $(PNGMAK) $(PNGLIB)
	cd $(BACKDIR)

$(ZDIR)\$(ZLIB):
	cd $(ZDIR)
	$(MAKE) -f $(ZMAK) $(ZLIB)
	cd $(BACKDIR)


clean:
	-del $(OPTIPNG)
	-del *.tds
	-del *.lib
	-del *.obj
	cd $(PNGXDIR)
	$(MAKE) -f $(PNGXMAK) clean
	cd $(BACKDIR)
	cd $(PNGDIR)
	$(MAKE) -f $(PNGMAK) clean
	cd $(BACKDIR)
	cd $(ZDIR)
	$(MAKE) -f $(ZMAK) clean
	cd $(BACKDIR)
