/*
 * pngxmem.c - libpng extension: memory allocation utilities.
 *
 * Copyright (C) 2001-2008 Cosmin Truta.
 * This software is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * This module contains functions proposed for addition to libpng.
 */

#include "pngx.h"


#if PNG_LIBPNG_VER < 10400
typedef png_uint_32 png_alloc_size_t;
/* Since libpng-1.4.x, png_alloc_size_t is either png_size_t or png_uint_32,
 * whichever is larger.
 */
#endif


png_bytepp PNGAPI
pngx_malloc_rows(png_structp png_ptr, png_infop info_ptr, int filler)
{
   png_bytep row;
   png_bytepp rows;
   png_alloc_size_t row_size;
   png_uint_32 height, i;

   /* Deallocate the currently-existing rows. */
#ifdef PNG_FREE_ME_SUPPORTED
   png_free_data(png_ptr, info_ptr, PNG_FREE_ROWS, 0);
#endif

   /* Allocate memory for the row index. */
   height = png_get_image_height(png_ptr, info_ptr);
   rows = (png_bytepp)png_malloc(png_ptr,
      (png_alloc_size_t)(height * sizeof(png_bytep)));
   if (rows == NULL)
      return NULL;

   /* Allocate memory for each row. */
   row_size = png_get_rowbytes(png_ptr, info_ptr);
   for (i = 0; i < height; ++i)
   {
      row = (png_bytep)png_malloc(png_ptr, row_size);
      if (row == NULL)  /* out of memory? */
      {
         /* Release the memory allocated up to the point of failure. */
         while (i > 0)
            png_free(png_ptr, rows[--i]);
         png_free(png_ptr, rows);
         return NULL;
      }
      if (filler >= 0)
         png_memset(row, filler, row_size);
      rows[i] = row;
   }

   /* Set the row pointers. */
   png_set_rows(png_ptr, info_ptr, rows);
   return rows;
}
