# visualc.mk
# Generated from Makefile.in
# Preconfigured for Microsoft Visual C++
#
# Usage: nmake -f build\visualc.mk

CC = cl -nologo
CFLAGS = -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -MD -O2 -W4
CPP = cl -nologo -E
CPPFLAGS =
LD = link -nologo
LDFLAGS =
MKDIR_P = mkdir
CP_FP = copy
RM_F = del /q

LIB_LIBPNG =
LIB_ZLIB =
LIBM =
LIBS =
ALL_LIBS = $(LIB_LIBPNG) $(LIB_ZLIB) $(LIBM) $(LIBS)

OPTIPNG_DIR = ..\optipng
CEXCEPT_DIR = ..\cexcept
OPNGREDUC_DIR = ..\opngreduc
OPNGREDUC_LIB = opngreduc.lib
OPNGREDUC_MK = build\visualc.mk
PNGXTERN_DIR = ..\pngxtern
PNGXTERN_LIB = pngxtern.lib
PNGXTERN_MK = build\visualc.mk
LIBPNG_DIR = ..\libpng
LIBPNG_LIB = libpng.lib
LIBPNG_MK = scripts\makefile.vcwin32
LIBPNG_MK_DEF = PNGLIBCONF_H_PREBUILT=pnglibconf.h.optipng
ZLIB_DIR = ..\zlib
ZLIB_LIB = zlib.lib
ZLIB_MK = win32\Makefile.msc
GIF_DIR = ..\gifread
GIF_LIB = gifread.lib
GIF_MK = build\visualc.mk
PNM_DIR = ..\pnmio
PNM_LIB = pnmio.lib
PNM_MK = build\visualc.mk
TIFF_DIR = ..\minitiff
TIFF_LIB = minitiff.lib
TIFF_MK = build\visualc.mk

OPTIPNG_OBJS = \
  optipng.obj \
  optim.obj \
  bitset.obj \
  ioutil.obj \
  ratio.obj \
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
  test\ratio_test.obj
OPTIPNG_TESTOUT = *.out.png test\*.out

all: optipng.exe

optipng.exe: $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS)
	$(LD) $(LDFLAGS) -out:$@ $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS) $(ALL_LIBS)

.c.obj:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(OPTIPNG_DEPINCLUDES) -Fo$@ $<

optipng.obj: optipng.c optipng.h bitset.h proginfo.h $(OPTIPNG_DEPLIBS)
optim.obj: optim.c optipng.h bitset.h ioutil.h ratio.h $(OPTIPNG_DEPLIBS)
bitset.obj: bitset.c bitset.h
ioutil.obj: ioutil.c ioutil.h
ratio.obj: ratio.c ratio.h
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

test: local-test test-gifread test-minitiff

local-test: optipng.exe $(OPTIPNG_TESTS)
	-@$(RM_F) pngtest.out.png
	.\optipng.exe -o1 -q img\pngtest.png -out=pngtest.out.png
	-@echo optipng ... ok
	test\bitset_test.exe > test\bitset_test.out
	-@echo bitset_test ... ok
	test\ratio_test.exe > test\ratio_test.out
	-@echo ratio_test ... ok

test\bitset_test.exe: test\bitset_test.obj bitset.obj
	$(LD) $(LDFLAGS) -out:$@ \
	  test\bitset_test.obj bitset.obj $(LIBS)

test\ratio_test.exe: test\ratio_test.obj ratio.obj
	$(LD) $(LDFLAGS) -out:$@ \
	  test\ratio_test.obj ratio.obj $(LIBS)

test\bitset_test.obj: test\bitset_test.c bitset.h
	$(CC) -c -I. $(CPPFLAGS) $(CFLAGS) -Fo$@ $*.c

test\ratio_test.obj: test\ratio_test.c ratio.h
	$(CC) -c -I. $(CPPFLAGS) $(CFLAGS) -Fo$@ $*.c

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

local-clean:
	-$(RM_F) optipng.exe optipng.exe.manifest $(OPTIPNG_OBJS)
	-$(RM_F) $(OPTIPNG_TESTS) $(OPTIPNG_TESTOBJS) $(OPTIPNG_TESTOUT)

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
  local-distclean \
  distclean-opngreduc \
  distclean-pngxtern-gif-pnm-tiff \
  distclean-libpng \
  distclean-zlib

local-distclean: local-clean
	-$(RM_F) Makefile

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
	$(MAKE) -f $(ZLIB_MK) clean
	cd $(OPTIPNG_DIR)

install: optipng.exe
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(MKDIR_P) $(DESTDIR)$(man1dir)
	-@$(RM_F) $(DESTDIR)$(bindir)\optipng.exe $(DESTDIR)$(bindir)\optipng.exe.manifest
	-@$(RM_F) $(DESTDIR)$(man1dir)\optipng.1
	$(CP_FP) optipng.exe $(DESTDIR)$(bindir)
	$(CP_FP) man\optipng.1 $(DESTDIR)$(man1dir)

uninstall:
	-$(RM_F) $(DESTDIR)$(bindir)\optipng.exe $(DESTDIR)$(bindir)\optipng.exe.manifest
	-$(RM_F) $(DESTDIR)$(man1dir)\optipng.1
