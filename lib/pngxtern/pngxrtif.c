/*
 * pngxrtif.c - libpng external I/O: TIFF reader.
 * Copyright (C) 2001-2006 Cosmin Truta.
 */

#include "pngxtern.h"


static const png_byte tiff_sig_m[4] = { 0x4d, 0x4d, 0x00, 0x2a };
static const png_byte tiff_sig_i[4] = { 0x49, 0x49, 0x2a, 0x00 };


int PNGAPI
pngx_sig_is_tiff(png_bytep sig, png_size_t len)
{
   if (len <= 4)
      return -1;
   return (memcmp(sig, tiff_sig_m, 4) == 0 || memcmp(sig, tiff_sig_i, 4) == 0);
}


png_charp PNGAPI
pngx_read_tiff(png_structp png_ptr, png_infop info_ptr, FILE *fp)
{
   if (&png_ptr || &info_ptr || &fp)
      png_error(png_ptr, "TIFF read support is not implemented");
   return "TIFF";
}
