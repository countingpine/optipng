# Makefile for OptiPNG
# Borland C++ for Win32
#
# Usage: make -f scripts\bcw32.mak


CC = bcc32
LD = bcc32
CFLAGS  = -O2 -v -w
LDFLAGS = -v

OPTIPNG  = optipng.exe
ZLIB     = zlib.lib
PNGLIB   = libpng.lib
ZMAK     = win32\Makefile.bor
PNGMAK   = scripts\makefile.bc32
ZDIR     = ..\lib\zlib
PNGDIR   = ..\lib\libpng
BACKHERE = ..\..\src

OBJS = optipng.obj opngio.obj opngreduc.obj cbitset.obj osys.obj wildargs.obj
LIBS = $(PNGDIR)\$(PNGLIB) $(ZDIR)\$(ZLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) noeh32.lib


.c.obj:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) $*.c

optipng.obj  : optipng.c   opng.h osys.h cbitset.h cexcept.h
opngio.obj   : opngio.c    opng.h
opngreduc.obj: opngreduc.c opng.h
cbitset.obj  : cbitset.c   cbitset.h
osys.obj     : osys.c      osys.h

wildargs.obj : xtra\wildargs.c
	$(CC) -c $(CFLAGS) xtra\wildargs.c


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
	-del *.exe
	-del *.tds
	cd $(PNGDIR)
	$(MAKE) -f $(PNGMAK) clean
	cd $(BACKHERE)
	cd $(ZDIR)
	$(MAKE) -f $(ZMAK) clean
	cd $(BACKHERE)
