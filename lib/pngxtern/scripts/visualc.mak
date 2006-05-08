# Makefile for pngxtern
# Microsoft Visual C++
#
# Usage: nmake -f scripts\visualc.mak

# Compiler, linker, librarian and other tools
CC = cl
LD = link
AR = lib
CFLAGS  = -nologo -MD -O2 -W3
LDFLAGS = -nologo
ARFLAGS = -nologo
RM = del

ZDIR   = ..\zlib
PNGDIR = ..\libpng

#uncomment next to put error messages in a file
#ERRFILE= >> pngerrs.log

# Variables
OBJS = pngxread.obj pngxrbmp.obj pngxrgif.obj pngxrjpg.obj pngxrpnm.obj \
       pngxrtif.obj gifread.obj pnmerror.obj pnmread.obj pnmwrite.obj

# Targets
all: pngxtern.lib

pngxtern.lib: $(OBJS)
	-$(RM) $@
	$(AR) $(ARFLAGS) -out:$@ $(OBJS) $(ERRFILE)

.c.obj:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) $< $(ERRFILE)

pngxread.obj: pngxread.c pngxtern.h
pngxrbmp.obj: pngxrbmp.c pngxtern.h
pngxrgif.obj: pngxrgif.c pngxtern.h gif\gifread.h
pngxrjpg.obj: pngxrjpg.c pngxtern.h
pngxrpnm.obj: pngxrpnm.c pngxtern.h pnm\pnmio.h
pngxrtif.obj: pngxrtif.c pngxtern.h

gifread.obj:  gif\gifread.c  gif\gifread.h
	$(CC) -c $(CFLAGS) gif\gifread.c  $(ERRFILE)

pnmerror.obj: pnm\pnmerror.c pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmerror.c $(ERRFILE)

pnmread.obj:  pnm\pnmread.c  pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmread.c  $(ERRFILE)

pnmwrite.obj: pnm\pnmwrite.c pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmwrite.c $(ERRFILE)

clean:
	-$(RM) *.obj
	-$(RM) pngxtern.lib

# End of makefile for pngxtern

