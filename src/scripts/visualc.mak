# Makefile for OptiPNG
# Microsoft Visual C++
#
# Usage: nmake -f scripts\visualc.mak


CC = cl
LD = cl
CFLAGS  = -MD -O2
LDFLAGS = -MD

OPTIPNG  = optipng.exe
ZLIB     = zlib.lib
PNGLIB   = libpng.lib
ZMAK     = win32\Makefile.msc
PNGMAK   = scripts\makefile.vcwin32
ZDIR     = ..\lib\zlib
PNGDIR   = ..\lib\libpng
BACKHERE = ..\..\src

OBJS = optipng.obj opngio.obj opngreduc.obj cbitset.obj osys.obj wildargs.obj
LIBS = $(PNGDIR)\$(PNGLIB) $(ZDIR)\$(ZLIB)


$(OPTIPNG): $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS)


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
	cd $(PNGDIR)
	$(MAKE) -f $(PNGMAK) clean
	cd $(BACKHERE)
	cd $(ZDIR)
	$(MAKE) -f $(ZMAK) clean
	cd $(BACKHERE)
