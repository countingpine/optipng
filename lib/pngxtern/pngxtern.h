/*
 * pngxtern.h - external file format processing for libpng.
 *
 * Copyright (C) 2001-2006 Cosmin Truta.
 * The program is distributed under the same licensing and warranty
 * terms as libpng.
 */


#ifndef PNGXTERN_H
#define PNGXTERN_H


#define PNG_INTERNAL

#include "png.h"


/*
 * OPNG_ASSERT
 * Hopefully, there will be a PNG_ASSERT in libpng sometime.
 * Until then, we provide a simpler version, based on assert.
 */
#ifndef OPNG_ASSERT
#include <assert.h>
#define OPNG_ASSERT(cond) assert(cond)
#define OPNG_ASSERT_MSG(cond, msg) assert(cond)
#endif


extern PNG_EXPORT(png_charp, pngx_png_name)
   PNGARG((const char *));

extern PNG_EXPORT(png_charp, pngx_read_external)
   PNGARG((png_structp png_ptr, png_infop info_ptr, FILE *fp));

/** PNG, JNG, MNG **/
extern PNG_EXPORT(int, pngx_sig_is_png_jng_mng)
   PNGARG((png_bytep sig, png_size_t len));

/** BMP **/
extern PNG_EXPORT(int, pngx_sig_is_bmp)
   PNGARG((png_bytep sig, png_size_t len));
extern PNG_EXPORT(png_charp, pngx_read_bmp)
   PNGARG((png_structp png_ptr, png_infop info_ptr, FILE *fp));

/** GIF **/
extern PNG_EXPORT(int, pngx_sig_is_gif)
   PNGARG((png_bytep sig, png_size_t len));
extern PNG_EXPORT(png_charp, pngx_read_gif)
   PNGARG((png_structp png_ptr, png_infop info_ptr, FILE *fp));

/** JPEG **/
extern PNG_EXPORT(int, pngx_sig_is_jpeg)
   PNGARG((png_bytep sig, png_size_t len));
extern PNG_EXPORT(png_charp, pngx_read_jpeg)
   PNGARG((png_structp png_ptr, png_infop info_ptr, FILE *fp));

/** PNM **/
extern PNG_EXPORT(int, pngx_sig_is_pnm)
   PNGARG((png_bytep sig, png_size_t len));
extern PNG_EXPORT(png_charp,pngx_read_pnm)
   PNGARG((png_structp png_ptr, png_infop info_ptr, FILE *fp));

/** TIFF **/
extern PNG_EXPORT(int, pngx_sig_is_tiff)
   PNGARG((png_bytep sig, png_size_t len));
extern PNG_EXPORT(png_charp,pngx_read_tiff)
   PNGARG((png_structp png_ptr, png_infop info_ptr, FILE *fp));


/** Utilities **/
extern PNG_EXPORT(png_voidp, pngx_zmalloc)
   PNGARG((png_structp png_ptr, png_uint_32 size));

extern PNG_EXPORT(void, pngx_set_interlace_type)
   PNGARG((png_structp png_ptr, png_infop info_ptr, int interlace_type));


#endif  /* PNGXTERN_H */
