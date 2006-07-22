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
OBJS = pngxread.obj pngxwrite.obj \
       pngxrbmp.obj pngxrgif.obj pngxrjpg.obj pngxrpnm.obj pngxrtif.obj \
       gifread.obj \
       pnmerror.obj pnmread.obj pnmwrite.obj \
       minitiff.obj tiffread.obj tiffwrite.obj

# Targets
all: pngxtern.lib

pngxtern.lib: $(OBJS)
	-$(RM) $@
	$(AR) $(ARFLAGS) -out:$@ $(OBJS) $(ERRFILE)

.c.obj:
	$(CC) -c $(CFLAGS) -I$(ZDIR) -I$(PNGDIR) $< $(ERRFILE)

pngxread.obj:  pngxread.c pngxtern.h
pngxwrite.obj: pngxwrite.c pngxtern.h
pngxrbmp.obj:  pngxrbmp.c pngxtern.h
pngxrgif.obj:  pngxrgif.c pngxtern.h gif\gifread.h
pngxrjpg.obj:  pngxrjpg.c pngxtern.h
pngxrpnm.obj:  pngxrpnm.c pngxtern.h pnm\pnmio.h
pngxrtif.obj:  pngxrtif.c pngxtern.h minitiff\minitiff.h

gifread.obj:   gif\gifread.c  gif\gifread.h
	$(CC) -c $(CFLAGS) gif\gifread.c  $(ERRFILE)

pnmerror.obj:  pnm\pnmerror.c pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmerror.c $(ERRFILE)

pnmread.obj:   pnm\pnmread.c  pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmread.c  $(ERRFILE)

pnmwrite.obj:  pnm\pnmwrite.c pnm\pnmio.h
	$(CC) -c $(CFLAGS) pnm\pnmwrite.c $(ERRFILE)

minitiff.obj:  minitiff\minitiff.c minitiff\minitiff.h
	$(CC) -c $(CFLAGS) minitiff\minitiff.c  $(ERRFILE)

tiffread.obj:  minitiff\tiffread.c minitiff\minitiff.h minitiff\tiffdef.h
	$(CC) -c $(CFLAGS) minitiff\tiffread.c  $(ERRFILE)

tiffwrite.obj: minitiff\tiffwrite.c minitiff\minitiff.h minitiff\tiffdef.h
	$(CC) -c $(CFLAGS) minitiff\tiffwrite.c $(ERRFILE)

clean:
	-$(RM) *.obj
	-$(RM) pngxtern.lib

# End of makefile for pngxtern

