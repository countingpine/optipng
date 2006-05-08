/*
 * pngxread.c - libpng external I/O: read utility functions.
 * Copyright (C) 2001-2006 Cosmin Truta.
 */

#include "pngxtern.h"
#include <ctype.h>


png_charp PNGAPI
pngx_png_name(const char *filename)
{
   png_charp result;
   size_t len = strlen(filename);
   if (len >= 4 && filename[len - 4] == '.' &&
       tolower(filename[len - 3]) == 'p' &&
       tolower(filename[len - 2]) == 'n' &&
       tolower(filename[len - 1]) == 'g')
      return (png_charp)filename;
   do
   {
      --len;
      if (filename[len] == '.')
         break;
   } while (len > 0);
   if (filename[len] != '.')
      len = strlen(filename);
   result = (png_charp)malloc(len + 5);
   strcpy(result, filename);
   strcpy(result + len, ".png");
   return result;
}


int PNGAPI
pngx_sig_is_png_jng_mng(png_bytep sig, png_size_t len)
{
   if (len < 4)
      return -1;
   if (memcmp(sig, "\211PNG", 4) == 0 ||
       memcmp(sig, "\212MNG", 4) == 0 ||
       memcmp(sig, "\213JNG", 4) == 0)
      return 1;
   return 0;
}


png_charp PNGAPI
pngx_read_external(png_structp png_ptr, png_infop info_ptr, FILE *fp)
{
   png_byte buf[256];
   png_size_t num;

   num = fread(buf, 1, sizeof(buf), fp);
   rewind(fp);

   if (pngx_sig_is_png_jng_mng(buf, num) > 0)
      return NULL;

   if (pngx_sig_is_bmp(buf, num) > 0)
      return pngx_read_bmp(png_ptr, info_ptr, fp);
   if (pngx_sig_is_gif(buf, num) > 0)
      return pngx_read_gif(png_ptr, info_ptr, fp);
   if (pngx_sig_is_jpeg(buf, num) > 0)
      return pngx_read_jpeg(png_ptr, info_ptr, fp);
   if (pngx_sig_is_pnm(buf, num) > 0)
      return pngx_read_pnm(png_ptr, info_ptr, fp);
   if (pngx_sig_is_tiff(buf, num) > 0)
      return pngx_read_tiff(png_ptr, info_ptr, fp);

   png_error(png_ptr, "Unrecognized image file format");
   return NULL;
}


png_voidp PNGAPI
pngx_zmalloc(png_structp png_ptr, png_uint_32 size)
{
   png_voidp result;

   if (size == 0)
      return NULL;

   result = (png_voidp)calloc(size, 1);
   if (result == NULL && (png_ptr->flags & PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
      png_error(png_ptr, "Out of memory!");

   return result;
}


void PNGAPI
pngx_set_interlace_type(png_structp png_ptr, png_infop info_ptr,
   int interlace_type)
{
   if (png_ptr != NULL && info_ptr != NULL)
   {
      if (interlace_type < 0 || interlace_type >= PNG_INTERLACE_LAST)
         png_error(png_ptr, "Unknown interlace method in IHDR");
      info_ptr->interlace_type = (png_byte)interlace_type;
   }
}
