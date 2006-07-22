This directory contains the libraries required by OptiPNG.

IMPORTANT NOTE:
  The libraries zlib and libpng are MODIFIED.
  These are NOT supported by their authors, but by the author of OptiPNG.
  Please DO NOT send bug reports to the authors of these libraries!


zlib
  * version: 1.2.3-optipng
  * changes: zlib/ChangeLog (see the "1.2.3-optipng" section)
    - details: lib-diff/zlib.diff
  * derived from original
    - version: 1.2.3
    - available at: http://www.zlib.net/

libpng
  * version: 1.2.12-optipng
  * changes: libpng/CHANGES (see the "1.2.12-optipng" section)
    - details: lib-diff/libpng.diff
  * derived from original
    - version: 1.2.12
    - available at: http://www.libpng.org/pub/png/libpng.html

pngxtern
  * version: 0.3
  * supports: BMP, GIF, PNM (PBM, PGM, PPM) and TIFF
    - BMP read support derived from bmp2png by MIYASAKA Masaru
    - GIF read support derived from giftopnm by David Koblas
    - PNM support based on pnmio library (version 0.2.1) by Cosmin Truta
    - TIFF read support based on minitiff library (version 0.1) by Cosmin Truta
