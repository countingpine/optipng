# Makefile for OptiPNG
# Borland C++ for Win32
#
# Usage: make -f scripts\bcc32.mak


CC = bcc32
LD = $(CC)
CFLAGS  = -O2 -v -w
LDFLAGS = -v

OPTIPNG  = optipng.exe
ZLIB     = zlib.lib
PNGLIB   = libpng.lib
PNGXLIB  = pngxtern.lib
ZMAK     = win32\Makefile.bor
PNGMAK   = scripts\makefile.bc32
PNGXMAK  = scripts\bcc32.mak
ZDIR     = ..\lib\zlib
PNGDIR   = ..\lib\libpng
PNGXDIR  = ..\lib\pngxtern
BACKHERE = ..\..\src

OBJS = optipng.obj opngio.obj opngreduc.obj cbitset.obj osys.obj strutil.obj \
       wildargs.obj
LIBS = $(PNGXDIR)\$(PNGXLIB) $(PNGDIR)\$(PNGLIB) $(ZDIR)\$(ZLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS)  # noeh32.lib


.c.obj:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) -I$(PNGXDIR) $*.c

optipng.obj  : optipng.c proginfo.h opng.h cexcept.h cbitset.h osys.h strutil.h
opngio.obj   : opngio.c opng.h
opngreduc.obj: opngreduc.c opng.h
cbitset.obj  : cbitset.c cbitset.h
osys.obj     : osys.c osys.h
strutil.obj  : strutil.c strutil.h

wildargs.obj : xtra\wildargs.c
	$(CC) -c $(CFLAGS) xtra\wildargs.c


$(PNGXDIR)\$(PNGXLIB): $(ZDIR)\$(ZLIB) $(PNGDIR)\$(PNGLIB)
	cd $(PNGXDIR)
	$(MAKE) -f $(PNGXMAK) $(PNGXLIB)
	cd $(BACKHERE)

$(PNGDIR)\$(PNGLIB): $(ZDIR)\$(ZLIB)
	cd $(PNGDIR)
	$(MAKE) -f $(PNGMAK) $(PNGLIB)
	cd $(BACKHERE)

$(ZDIR)\$(ZLIB):
	cd $(ZDIR)
	$(MAKE) -f $(ZMAK) $(ZLIB)
	cd $(BACKHERE)


clean:
	-del *.obj
	-del *.lib
	-del $(OPTIPNG)
	-del *.tds
	cd $(PNGXDIR)
	$(MAKE) -f $(PNGXMAK) clean
	cd $(BACKHERE)
	cd $(PNGDIR)
	$(MAKE) -f $(PNGMAK) clean
	cd $(BACKHERE)
	cd $(ZDIR)
	$(MAKE) -f $(ZMAK) clean
	cd $(BACKHERE)
