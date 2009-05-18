
OptiPNG-supporting libraries
============================


IMPORTANT NOTICE
----------------
  OptiPNG uses MODIFIED versions of zlib and libpng, which are
  NOT SUPPORTED by their authors.
  If you have comments, complaints or bug reports regarding these
  particular library versions, you may use the OptiPNG support channels.
  Please DO NOT send such reports to the authors of these libraries!
  (You may send them compliments, though.)


Library list
------------

zlib
  * authors:  Jean-loup Gailly and Mark Adler
  * version:  1.2.3-optipng
  * location: lib/zlib/
  * derived from original
    - version:  1.2.3
    - location: http://www.zlib.net/
    - changes:  lib/zlib/ChangeLog (see the "1.2.3-optipng" section)
    - details:  lib/lib_diff/zlib.diff

libpng
  * authors:  Glenn Randers-Pehrson and the PNG Development Group
  * version:  1.2.33-optipng
  * location: lib/libpng/
  * derived from original
    - version:  1.2.33
    - location: http://www.libpng.org/pub/png/libpng.html
    - changes:  lib/libpng/CHANGES (see the "1.2.33-optipng" section)
    - details:  lib/lib_diff/libpng.diff

pngxtern
  * author:   Cosmin Truta
  * version:  0.6.3
  * location: lib/pngxtern/
  * supported formats: BMP, GIF, PNM (PBM, PGM, PPM), and TIFF (uncompressed)
    - BMP read support derived from bmp2png by MIYASAKA Masaru
    - GIF read support derived from giftopnm by David Koblas
    - PNM support based on pnmio library (version 0.3) by Cosmin Truta
    - TIFF read support based on minitiff library (version 0.1) by Cosmin Truta

