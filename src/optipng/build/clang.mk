# clang.mk
# Generated from Makefile.in
# Preconfigured for clang
#
# Usage: make -f build/clang.mk

.PHONY: all test check clean distclean install uninstall
.PRECIOUS: Makefile
.SUFFIXES: .c .o .a

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
mandir = $(prefix)/man
man1dir = $(mandir)/man1

CC = clang
CFLAGS = -O2 -Wall -Wextra
CPP = $(CC) -E
CPPFLAGS =
LD = $(CC)
LDFLAGS = -s
DIFF = diff -b -u
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
LIBPNG_DIR = ../libpng
LIBPNG_LIB = libpng.a
#LIBPNG_LIB = -lpng
LIBPNG_MK = scripts/makefile.gcc
ZLIB_DIR = ../zlib
ZLIB_LIB = libz.a
#ZLIB_LIB = -lz
ZLIB_MK = Makefile
PNGXTERN_DIR = ../pngxtern
PNGXTERN_LIB = libpngxtern.a
PNGXTERN_MK = build/clang.mk
GIF_DIR = ../gifread
GIF_LIB = libgifread.a
GIF_MK = build/clang.mk
PNM_DIR = ../pnmio
PNM_LIB = libpnmio.a
PNM_MK = build/clang.mk
TIFF_DIR = ../minitiff
TIFF_LIB = libminitiff.a
TIFF_MK = build/clang.mk

OPTIPNG_OBJS = \
  optipng.o \
  opngoptim.o \
  opngreduc.o \
  cbitset.o \
  osys.o \
  wildargs.o

OPTIPNG_DEPLIB_ZLIB = $(ZLIB_DIR)/$(ZLIB_LIB)
#OPTIPNG_DEPLIB_ZLIB =
OPTIPNG_DEPLIB_LIBPNG = $(LIBPNG_DIR)/$(LIBPNG_LIB)
#OPTIPNG_DEPLIB_ZLIB =
OPTIPNG_DEPLIBS = \
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
  -I$(PNGXTERN_DIR)

OPTIPNG_TESTS = test/cbitset_test$(EXEEXT) test/print_ratio_test$(EXEEXT)
OPTIPNG_TESTOUT = *.out.png test/*.out

all: optipng$(EXEEXT)

optipng$(EXEEXT): $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS)
	$(LD) $(LDFLAGS) -o $@ $(OPTIPNG_OBJS) $(OPTIPNG_DEPLIBS) $(ALL_LIBS)

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(OPTIPNG_DEPINCLUDES) $<

optipng.o: optipng.c optipng.h cbitset.h osys.h proginfo.h
opngoptim.o: opngoptim.c optipng.h opngreduc.h cbitset.h osys.h
opngreduc.o: opngreduc.c opngreduc.h
cbitset.o: cbitset.c cbitset.h
osys.o: osys.c osys.h
wildargs.o: wildargs.c

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
	$(MAKE) -f $(LIBPNG_MK) $(LIBPNG_LIB) && \
	cd $(OPTIPNG_DIR)

$(ZLIB_DIR)/$(ZLIB_LIB):
	cd $(ZLIB_DIR) && \
	$(MAKE) -f $(ZLIB_MK) $(ZLIB_LIB) && \
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

test: local-test test-libpng test-gifread

.PHONY: local-test
local-test: optipng$(EXEEXT) $(OPTIPNG_TESTS)
	-@$(RM_F) pngtest.out.png
	./optipng$(EXEEXT) -o1 -q img/pngtest.png -out=pngtest.out.png
	-@echo optipng ... ok
	test/cbitset_test$(EXEEXT) < test/cbitset_test.dat > test/cbitset_test.out
	diff -b -u test/cbitset_test.expect test/cbitset_test.out
	-@echo cbitset_test ... ok
	test/print_ratio_test$(EXEEXT) > test/print_ratio_test.out
	-@echo print_ratio_test ... ok
	# There is no expect file for print_ratio_test, as its output may vary.

test/cbitset_test$(EXEEXT): \
  test/cbitset_test.c cbitset.o cbitset.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -I. -o $@ \
	  test/cbitset_test.c cbitset.o $(LIBS)

test/print_ratio_test$(EXEEXT): \
  test/print_ratio_test.c test/sprint_ratio.generated.c test/print_ratio.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ test/print_ratio_test.c test/sprint_ratio.generated.c

#test/sprint_ratio.generated.c: test/extract_print_ratio.sh opngoptim.c
#	$(SHELL) -c test/extract_print_ratio.sh

.PHONY: test-libpng
test-libpng: test-zlib
	cd $(LIBPNG_DIR) && \
	$(MAKE) -f $(LIBPNG_MK) test && \
	cd $(OPTIPNG_DIR)

.PHONY: test-zlib
test-zlib:
	cd $(ZLIB_DIR) && \
	$(MAKE) -f $(ZLIB_MK) test && \
	cd $(OPTIPNG_DIR)

.PHONY: test-gifread
test-gifread:
	cd $(GIF_DIR) && \
	$(MAKE) -f $(GIF_MK) test && \
	cd $(OPTIPNG_DIR)

check: test

clean: \
  local-clean \
  clean-pngxtern-gif-pnm-tiff \
  clean-libpng \
  clean-zlib

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
	$(MAKE) -f $(LIBPNG_MK) clean && \
	cd $(OPTIPNG_DIR)

.PHONY: clean-zlib
clean-zlib:
	cd $(ZLIB_DIR) && \
	$(MAKE) -f $(ZLIB_MK) clean && \
	cd $(OPTIPNG_DIR)

distclean: \
  local-clean \
  distclean-pngxtern-gif-pnm-tiff \
  distclean-libpng \
  distclean-zlib
	-$(RM_F) Makefile

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
	$(MAKE) -f $(LIBPNG_MK) clean && \
	cd $(OPTIPNG_DIR)

.PHONY: distclean-zlib
distclean-zlib:
	cd $(ZLIB_DIR) && \
	$(MAKE) -f $(ZLIB_MK) distclean && \
	cd $(OPTIPNG_DIR)

.PHONY: local-clean
local-clean:
	-$(RM_F) optipng$(EXEEXT) $(OPTIPNG_OBJS)
	-$(RM_F) $(OPTIPNG_TESTS) $(OPTIPNG_TESTOUT)

install: optipng$(EXEEXT)
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(man1dir)
	-@$(RM_F) $(DESTDIR)$(bindir)/optipng$(EXEEXT)
	-@$(RM_F) $(DESTDIR)$(man1dir)/optipng.1
	cp -f -p optipng$(EXEEXT) $(DESTDIR)$(bindir)
	cp -f -p man/optipng.1 $(DESTDIR)$(man1dir)

uninstall:
	-$(RM_F) $(DESTDIR)$(bindir)/optipng$(EXEEXT)
	-$(RM_F) $(DESTDIR)$(man1dir)/optipng.1
