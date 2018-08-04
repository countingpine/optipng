# bcc32.mk
# Generated from Makefile.in
# Preconfigured for Borland C++
#
# Usage: make -f build\bcc32.mk

CC = bcc32
CFLAGS = -O2 -d -k- -w
CPP = #cpp32 -P-
CPPFLAGS =
LD = $(CC)
LDFLAGS = $(CFLAGS)
DIFF = fc
RM_F = del /q

LIB_LIBPNG =
LIB_ZLIB =
LIBM =
LIBS = #noeh32.lib
ALL_LIBS = $(LIB_LIBPNG) $(LIB_ZLIB) $(LIBM) $(LIBS)

OPTIPNG_DIR = ..\optipng
CEXCEPT_DIR = ..\cexcept
OPNGREDUC_DIR = ..\opngreduc
OPNGREDUC_LIB = opngreduc.lib
OPNGREDUC_MK = build\bcc32.mk
PNGXTERN_DIR = ..\pngxtern
PNGXTERN_LIB = pngxtern.lib
PNGXTERN_MK = build\bcc32.mk
LIBPNG_DIR = ..\libpng
LIBPNG_LIB = libpng.lib
LIBPNG_MK = scripts\makefile.bc32
LIBPNG_MK_DEF = PNGLIBCONF_H_PREBUILT=pnglibconf.h.optipng
ZLIB_DIR = ..\zlib
ZLIB_LIB = zlib.lib
ZLIB_MK = win32\Makefile.bor
GIF_DIR = ..\gifread
GIF_LIB = gifread.lib
GIF_MK = build\bcc32.mk
PNM_DIR = ..\pnmio
PNM_LIB = pnmio.lib
PNM_MK = build\bcc32.mk
TIFF_DIR = ..\minitiff
TIFF_LIB = minitiff.lib
TIFF_MK = build\bcc32.mk

OPTIPNG_OBJS = \
  optipng.obj \
  optim.obj \
  bitset.obj \
  ratio.obj \
  osys.obj \
  wildargs.obj

OPTIPNG_DEPLIB_ZLIB = $(ZLIB_DIR)\$(ZLIB_LIB)
OPTIPNG_DEPLIB_LIBPNG = $(LIBPNG_DIR)\$(LIBPNG_LIB)

OPTIPNG_DEPLIBS = \
  $(OPNGREDUC_DIR)\$(OPNGREDUC_LIB) \
  $(PNGXTERN_DIR)\$(PNGXTERN_LIB) \
  $(OPTIPNG_DEPLIB_LIBPNG) \
  $(OPTIPNG_DEPLIB_ZLIB) \
  $(GIF_DIR)\$(GIF_LIB) \
  $(PNM_DIR)\$(PNM_LIB) \
  $(TIFF_DIR)\$(TIFF_LIB)

OPTIPNG_DEPINCLUDE_ZLIB = -I$(ZLIB_DIR)
OPTIPNG_DEPINCLUDE_LIBPNG = -I$(LIBPNG_DIR)
OPTIPNG_DEPINCLUDES = \
  -I$(CEXCEPT_DIR) \
  $(OPTIPNG_DEPINCLUDE_ZLIB) \
  $(OPTIPNG_DEPINCLUDE_LIBPNG) \
  -I$(OPNGREDUC_DIR) \
  -I$(PNGXTERN_DIR)

OPTIPNG_TESTS = \
  test\bitset_test.exe \
  test\ratio_test.exe
OPTIPNG_TESTOBJS = \
  test\bitset_test.obj \
  test\ratio_test.obj \
OPTIPNG_TESTOUT = *.out.png test\*.out

all: optipng.exe

optipng.exe: $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS)
	$(LD) $(LDFLAGS) -e$@ $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS) $(ALL_LIBS)

.c.obj:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(OPTIPNG_DEPINCLUDES) -o$@ $<

optipng.obj: optipng.c optipng.h bitset.h osys.h proginfo.h $(OPTIPNG_DEPLIBS)
optim.obj: optim.c optipng.h bitset.h osys.h ratio.h $(OPTIPNG_DEPLIBS)
bitset.obj: bitset.c bitset.h
ratio.obj: ratio.c ratio.h
osys.obj: osys.c osys.h
wildargs.obj: wildargs.c

$(OPNGREDUC_DIR)\$(OPNGREDUC_LIB): \
  $(OPTIPNG_DEPLIB_LIBPNG)
	cd $(OPNGREDUC_DIR)
	$(MAKE) -f $(OPNGREDUC_MK) $(OPNGREDUC_LIB)
	cd $(OPTIPNG_DIR)

$(PNGXTERN_DIR)\$(PNGXTERN_LIB): \
  $(OPTIPNG_DEPLIB_LIBPNG) \
  $(GIF_DIR)\$(GIF_LIB) \
  $(PNM_DIR)\$(PNM_LIB) \
  $(TIFF_DIR)\$(TIFF_LIB)
	cd $(PNGXTERN_DIR)
	$(MAKE) -f $(PNGXTERN_MK) $(PNGXTERN_LIB)
	cd $(OPTIPNG_DIR)

$(LIBPNG_DIR)\$(LIBPNG_LIB): \
  $(OPTIPNG_DEPLIB_ZLIB)
	cd $(LIBPNG_DIR)
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_MK_DEF)
	cd $(OPTIPNG_DIR)

$(ZLIB_DIR)\$(ZLIB_LIB):
	cd $(ZLIB_DIR)
	$(MAKE) -f $(ZLIB_MK)
	cd $(OPTIPNG_DIR)

$(GIF_DIR)\$(GIF_LIB):
	cd $(GIF_DIR)
	$(MAKE) -f $(GIF_MK) $(GIF_LIB)
	cd $(OPTIPNG_DIR)

$(PNM_DIR)\$(PNM_LIB):
	cd $(PNM_DIR)
	$(MAKE) -f $(PNM_MK) $(PNM_LIB)
	cd $(OPTIPNG_DIR)

$(TIFF_DIR)\$(TIFF_LIB):
	cd $(TIFF_DIR)
	$(MAKE) -f $(TIFF_MK) $(TIFF_LIB)
	cd $(OPTIPNG_DIR)

test: local-test test-libpng test-gifread test-minitiff

local-test: optipng.exe $(OPTIPNG_TESTS)
	-@$(RM_F) pngtest.out.png
	.\optipng.exe -o1 -q img\pngtest.png -out=pngtest.out.png
	-@echo optipng ... ok
	test\bitset_test.exe < test\bitset_test.dat > test\bitset_test.out
	fc test\bitset_test.expect test\bitset_test.out
	-@echo bitset_test ... ok
	test\ratio_test.exe > test\ratio_test.out
	-@echo ratio_test ... ok

test\bitset_test.exe: test\bitset_test.obj bitset.obj
	$(LD) $(LDFLAGS) -e$@ \
	  test\bitset_test.obj bitset.obj $(LIBS)

test\ratio_test.exe: test\ratio_test.obj ratio.obj
	$(LD) $(LDFLAGS) -e$@ \
	  test\ratio_test.obj ratio.obj $(LIBS)

test\bitset_test.obj: test\bitset_test.c bitset.h
	$(CC) -c -I. $(CPPFLAGS) $(CFLAGS) -o$@ $*.c

test\ratio_test.obj: test\ratio_test.c ratio.h
	$(CC) -c -I. $(CPPFLAGS) $(CFLAGS) -o$@ $*.c

test-libpng: test-zlib
	cd $(LIBPNG_DIR)
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_MK_DEF) test
	cd $(OPTIPNG_DIR)

# FIXME:
# Can't test zlib if NO_GZCOMPRES and NO_GZIP are enabled.
test-zlib:
#	cd $(ZLIB_DIR)
#	$(MAKE) -f $(ZLIB_MK) test
#	cd $(OPTIPNG_DIR)

test-gifread:
	cd $(GIF_DIR)
	$(MAKE) -f $(GIF_MK) test
	cd $(OPTIPNG_DIR)

test-minitiff:
	cd $(TIFF_DIR)
	$(MAKE) -f $(TIFF_MK) test
	cd $(OPTIPNG_DIR)

check: test

clean: \
  local-clean \
  clean-opngreduc \
  clean-pngxtern-gif-pnm-tiff \
  clean-libpng \
  clean-zlib

clean-opngreduc:
	cd $(OPNGREDUC_DIR)
	$(MAKE) -f $(OPNGREDUC_MK) clean
	cd $(OPTIPNG_DIR)

clean-pngxtern-gif-pnm-tiff:
	cd $(PNGXTERN_DIR)
	$(MAKE) -f $(PNGXTERN_MK) clean
	cd $(OPTIPNG_DIR)
	cd $(GIF_DIR)
	$(MAKE) -f $(GIF_MK) clean
	cd $(OPTIPNG_DIR)
	cd $(PNM_DIR)
	$(MAKE) -f $(PNM_MK) clean
	cd $(OPTIPNG_DIR)
	cd $(TIFF_DIR)
	$(MAKE) -f $(TIFF_MK) clean
	cd $(OPTIPNG_DIR)

clean-libpng:
	cd $(LIBPNG_DIR)
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_MK_DEF) clean
	cd $(OPTIPNG_DIR)

clean-zlib:
	cd $(ZLIB_DIR)
	$(MAKE) -f $(ZLIB_MK) clean
	cd $(OPTIPNG_DIR)

distclean: \
  local-clean \
  distclean-opngreduc \
  distclean-pngxtern-gif-pnm-tiff \
  distclean-libpng \
  distclean-zlib
	-$(RM_F) Makefile man\Makefile

distclean-opngreduc:
	cd $(OPNGREDUC_DIR)
	$(MAKE) -f $(OPNGREDUC_MK) distclean
	cd $(OPTIPNG_DIR)

distclean-pngxtern-gif-pnm-tiff:
	cd $(PNGXTERN_DIR)
	$(MAKE) -f $(PNGXTERN_MK) distclean
	cd $(OPTIPNG_DIR)
	cd $(GIF_DIR)
	$(MAKE) -f $(GIF_MK) distclean
	cd $(OPTIPNG_DIR)
	cd $(PNM_DIR)
	$(MAKE) -f $(PNM_MK) distclean
	cd $(OPTIPNG_DIR)
	cd $(TIFF_DIR)
	$(MAKE) -f $(TIFF_MK) distclean
	cd $(OPTIPNG_DIR)

distclean-libpng:
	cd $(LIBPNG_DIR)
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_MK_DEF) clean
	cd $(OPTIPNG_DIR)

distclean-zlib:
	cd $(ZLIB_DIR)
	$(MAKE) -f $(ZLIB_MK) distclean
	cd $(OPTIPNG_DIR)

local-clean:
	-$(RM_F) optipng.exe $(OPTIPNG_OBJS)
	-$(RM_F) $(OPTIPNG_TESTS) $(OPTIPNG_TESTOBJS) $(OPTIPNG_TESTOUT)
	-$(RM_F) *.tds

install: optipng.exe
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(man1dir)
	-@$(RM_F) $(DESTDIR)$(bindir)\optipng.exe
	-@$(RM_F) $(DESTDIR)$(man1dir)\optipng.1
	cp -f -p optipng.exe $(DESTDIR)$(bindir)
	cp -f -p man\optipng.1 $(DESTDIR)$(man1dir)

uninstall:
	-$(RM_F) $(DESTDIR)$(bindir)\optipng.exe
	-$(RM_F) $(DESTDIR)$(man1dir)\optipng.1
