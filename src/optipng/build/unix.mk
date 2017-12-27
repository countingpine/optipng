# unix.mk
# Generated from Makefile.in
# Preconfigured for Unix (generic)
#
# Usage: make -f build/unix.mk

.PHONY: all test check clean distclean install uninstall
.PRECIOUS: Makefile
.SUFFIXES: .c .o .a

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
mandir = $(prefix)/man
man1dir = $(mandir)/man1

CC = cc
CFLAGS = -O
CPP = $(CC) -E
CPPFLAGS =
LD = $(CC)
LDFLAGS = -s
MKDIR_P = mkdir -p
CP_FP = cp -f -p
RM_F = rm -f

LIB_LIBPNG =
#LIB_LIBPNG = -lpng
LIB_ZLIB =
#LIB_ZLIB = -lz
LIBM = -lm
LIBS =
ALL_LIBS = $(LIB_LIBPNG) $(LIB_ZLIB) $(LIBM) $(LIBS)

OPTIPNG_DIR = ../optipng
CEXCEPT_DIR = ../cexcept
OPNGREDUC_DIR = ../opngreduc
OPNGREDUC_LIB = libopngreduc.a
OPNGREDUC_MK = build/unix.mk
PNGXTERN_DIR = ../pngxtern
PNGXTERN_LIB = libpngxtern.a
PNGXTERN_MK = build/unix.mk
LIBPNG_DIR = ../libpng
LIBPNG_LIB = libpng.a
#LIBPNG_LIB = -lpng
LIBPNG_MK = scripts/makefile.gcc
LIBPNG_MK_DEF = PNGLIBCONF_H_PREBUILT=pnglibconf.h.optipng
ZLIB_DIR = ../zlib
ZLIB_LIB = libz.a
#ZLIB_LIB = -lz
ZLIB_MK = Makefile
GIF_DIR = ../gifread
GIF_LIB = libgifread.a
GIF_MK = build/unix.mk
PNM_DIR = ../pnmio
PNM_LIB = libpnmio.a
PNM_MK = build/unix.mk
TIFF_DIR = ../minitiff
TIFF_LIB = libminitiff.a
TIFF_MK = build/unix.mk

OPTIPNG_OBJS = \
  optipng.o \
  optim.o \
  bitset.o \
  ioutil.o \
  ratio.o \
  wildargs.o

OPTIPNG_DEPLIB_ZLIB = $(ZLIB_DIR)/$(ZLIB_LIB)
#OPTIPNG_DEPLIB_ZLIB =
OPTIPNG_DEPLIB_LIBPNG = $(LIBPNG_DIR)/$(LIBPNG_LIB)
#OPTIPNG_DEPLIB_ZLIB =

OPTIPNG_DEPLIBS = \
  $(OPNGREDUC_DIR)/$(OPNGREDUC_LIB) \
  $(PNGXTERN_DIR)/$(PNGXTERN_LIB) \
  $(OPTIPNG_DEPLIB_LIBPNG) \
  $(OPTIPNG_DEPLIB_ZLIB) \
  $(GIF_DIR)/$(GIF_LIB) \
  $(PNM_DIR)/$(PNM_LIB) \
  $(TIFF_DIR)/$(TIFF_LIB)

OPTIPNG_DEPINCLUDE_ZLIB = -I$(ZLIB_DIR)
#OPTIPNG_DEPINCLUDE_ZLIB =
OPTIPNG_DEPINCLUDE_LIBPNG = -I$(LIBPNG_DIR)
#OPTIPNG_DEPINCLUDE_LIBPNG =
OPTIPNG_DEPINCLUDES = \
  -I$(CEXCEPT_DIR) \
  $(OPTIPNG_DEPINCLUDE_ZLIB) \
  $(OPTIPNG_DEPINCLUDE_LIBPNG) \
  -I$(OPNGREDUC_DIR) \
  -I$(PNGXTERN_DIR)

OPTIPNG_TESTS = \
  test/bitset_test$(EXEEXT) \
  test/ratio_test$(EXEEXT)
OPTIPNG_TESTOBJS = \
  test/bitset_test.o \
  test/ratio_test.o
OPTIPNG_TESTOUT = *.out.png test/*.out

all: optipng$(EXEEXT)

optipng$(EXEEXT): $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS)
	$(LD) $(LDFLAGS) -o $@ $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS) $(ALL_LIBS)

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(OPTIPNG_DEPINCLUDES) -o $@ $<

optipng.o: optipng.c optipng.h bitset.h proginfo.h $(OPTIPNG_DEPLIBS)
optim.o: optim.c optipng.h bitset.h ioutil.h ratio.h $(OPTIPNG_DEPLIBS)
bitset.o: bitset.c bitset.h
ioutil.o: ioutil.c ioutil.h
ratio.o: ratio.c ratio.h
wildargs.o: wildargs.c

$(OPNGREDUC_DIR)/$(OPNGREDUC_LIB): \
  $(OPTIPNG_DEPLIB_LIBPNG)
	cd $(OPNGREDUC_DIR) && \
	$(MAKE) -f $(OPNGREDUC_MK) $(OPNGREDUC_LIB) && \
	cd $(OPTIPNG_DIR)

$(PNGXTERN_DIR)/$(PNGXTERN_LIB): \
  $(OPTIPNG_DEPLIB_LIBPNG) \
  $(GIF_DIR)/$(GIF_LIB) \
  $(PNM_DIR)/$(PNM_LIB) \
  $(TIFF_DIR)/$(TIFF_LIB)
	cd $(PNGXTERN_DIR) && \
	$(MAKE) -f $(PNGXTERN_MK) $(PNGXTERN_LIB) && \
	cd $(OPTIPNG_DIR)

$(LIBPNG_DIR)/$(LIBPNG_LIB): \
  $(OPTIPNG_DEPLIB_ZLIB)
	cd $(LIBPNG_DIR) && \
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_MK_DEF) && \
	cd $(OPTIPNG_DIR)

$(ZLIB_DIR)/$(ZLIB_LIB):
	cd $(ZLIB_DIR) && \
	$(MAKE) -f $(ZLIB_MK) && \
	cd $(OPTIPNG_DIR)

$(GIF_DIR)/$(GIF_LIB):
	cd $(GIF_DIR) && \
	$(MAKE) -f $(GIF_MK) $(GIF_LIB) && \
	cd $(OPTIPNG_DIR)

$(PNM_DIR)/$(PNM_LIB):
	cd $(PNM_DIR) && \
	$(MAKE) -f $(PNM_MK) $(PNM_LIB) && \
	cd $(OPTIPNG_DIR)

$(TIFF_DIR)/$(TIFF_LIB):
	cd $(TIFF_DIR) && \
	$(MAKE) -f $(TIFF_MK) $(TIFF_LIB) && \
	cd $(OPTIPNG_DIR)

test: local-test test-gifread test-minitiff

.PHONY: local-test
local-test: optipng$(EXEEXT) $(OPTIPNG_TESTS)
	-@$(RM_F) pngtest.out.png
	./optipng$(EXEEXT) -o1 -q img/pngtest.png -out=pngtest.out.png
	-@echo optipng ... ok
	test/bitset_test$(EXEEXT) > test/bitset_test.out
	-@echo bitset_test ... ok
	test/ratio_test$(EXEEXT) > test/ratio_test.out
	-@echo ratio_test ... ok

test/bitset_test$(EXEEXT): test/bitset_test.o bitset.o
	$(LD) $(LDFLAGS) -o $@ \
	  test/bitset_test.o bitset.o $(LIBS)

test/ratio_test$(EXEEXT): test/ratio_test.o ratio.o
	$(LD) $(LDFLAGS) -o $@ \
	  test/ratio_test.o ratio.o $(LIBS)

test/bitset_test.o: test/bitset_test.c bitset.h
	$(CC) -c -I. $(CPPFLAGS) $(CFLAGS) -o $@ $*.c

test/ratio_test.o: test/ratio_test.c ratio.h
	$(CC) -c -I. $(CPPFLAGS) $(CFLAGS) -o $@ $*.c

.PHONY: test-gifread
test-gifread:
	cd $(GIF_DIR) && \
	$(MAKE) -f $(GIF_MK) test && \
	cd $(OPTIPNG_DIR)

.PHONY: test-minitiff
test-minitiff:
	cd $(TIFF_DIR) && \
	$(MAKE) -f $(TIFF_MK) test && \
	cd $(OPTIPNG_DIR)

check: test

clean: \
  local-clean \
  clean-opngreduc \
  clean-pngxtern-gif-pnm-tiff \
  clean-libpng \
  clean-zlib

.PHONY: local-clean
local-clean:
	-$(RM_F) optipng$(EXEEXT) $(OPTIPNG_OBJS)
	-$(RM_F) $(OPTIPNG_TESTS) $(OPTIPNG_TESTOBJS) $(OPTIPNG_TESTOUT)

.PHONY: clean-opngreduc
clean-opngreduc:
	cd $(OPNGREDUC_DIR) && \
	$(MAKE) -f $(OPNGREDUC_MK) clean && \
	cd $(OPTIPNG_DIR)

.PHONY: clean-pngxtern-gif-pnm-tiff
clean-pngxtern-gif-pnm-tiff:
	cd $(PNGXTERN_DIR) && \
	$(MAKE) -f $(PNGXTERN_MK) clean && \
	cd $(OPTIPNG_DIR)
	cd $(GIF_DIR) && \
	$(MAKE) -f $(GIF_MK) clean && \
	cd $(OPTIPNG_DIR)
	cd $(PNM_DIR) && \
	$(MAKE) -f $(PNM_MK) clean && \
	cd $(OPTIPNG_DIR)
	cd $(TIFF_DIR) && \
	$(MAKE) -f $(TIFF_MK) clean && \
	cd $(OPTIPNG_DIR)

.PHONY: clean-libpng
clean-libpng:
	cd $(LIBPNG_DIR) && \
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_MK_DEF) clean && \
	cd $(OPTIPNG_DIR)

.PHONY: clean-zlib
clean-zlib:
	cd $(ZLIB_DIR) && \
	$(MAKE) -f $(ZLIB_MK) clean && \
	cd $(OPTIPNG_DIR)

distclean: \
  local-distclean \
  distclean-opngreduc \
  distclean-pngxtern-gif-pnm-tiff \
  distclean-libpng \
  distclean-zlib

.PHONY: local-distclean
local-distclean: local-clean
	-$(RM_F) Makefile
	cd man && \
	$(MAKE) distclean && \
	cd ..

.PHONY: distclean-opngreduc
distclean-opngreduc:
	cd $(OPNGREDUC_DIR) && \
	$(MAKE) -f $(OPNGREDUC_MK) distclean && \
	cd $(OPTIPNG_DIR)

.PHONY: distclean-pngxtern-gif-pnm-tiff
distclean-pngxtern-gif-pnm-tiff:
	cd $(PNGXTERN_DIR) && \
	$(MAKE) -f $(PNGXTERN_MK) distclean && \
	cd $(OPTIPNG_DIR)
	cd $(GIF_DIR) && \
	$(MAKE) -f $(GIF_MK) distclean && \
	cd $(OPTIPNG_DIR)
	cd $(PNM_DIR) && \
	$(MAKE) -f $(PNM_MK) distclean && \
	cd $(OPTIPNG_DIR)
	cd $(TIFF_DIR) && \
	$(MAKE) -f $(TIFF_MK) distclean && \
	cd $(OPTIPNG_DIR)

.PHONY: distclean-libpng
distclean-libpng:
	cd $(LIBPNG_DIR) && \
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_MK_DEF) clean && \
	cd $(OPTIPNG_DIR)

.PHONY: distclean-zlib
distclean-zlib:
	cd $(ZLIB_DIR) && \
	$(MAKE) -f $(ZLIB_MK) distclean && \
	cd $(OPTIPNG_DIR)

install: optipng$(EXEEXT)
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(MKDIR_P) $(DESTDIR)$(man1dir)
	-@$(RM_F) $(DESTDIR)$(bindir)/optipng$(EXEEXT)
	-@$(RM_F) $(DESTDIR)$(man1dir)/optipng.1
	$(CP_FP) optipng$(EXEEXT) $(DESTDIR)$(bindir)
	$(CP_FP) man/optipng.1 $(DESTDIR)$(man1dir)

uninstall:
	-$(RM_F) $(DESTDIR)$(bindir)/optipng$(EXEEXT)
	-$(RM_F) $(DESTDIR)$(man1dir)/optipng.1
