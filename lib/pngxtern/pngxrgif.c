/*
 * pngxrgif.c - libpng external I/O: GIF reader.
 * Copyright (C) 2001-2006 Cosmin Truta.
 */

#include "pngxtern.h"
#include "gif/gifread.h"


static /* PRIVATE */ png_structp pngx_err_ptr = NULL;

static void /* PRIVATE */
pngx_gif_error(const char *msg)
{
   png_error(pngx_err_ptr, msg);
}

static void /* PRIVATE */
pngx_gif_warning(const char *msg)
{
   png_warning(pngx_err_ptr, msg);
}


int PNGAPI
pngx_sig_is_gif(png_bytep sig, png_size_t len)
{
   if (len < 6)
      return -1;
   if (memcmp(sig, "GIF87a", 6) == 0 ||
       memcmp(sig, "GIF89a", 6) == 0)
      return 1;
   return 0;
}


static void /* PRIVATE */
pngx_set_GIF_palette(png_structp png_ptr, png_infop info_ptr,
   unsigned char *color_table, unsigned int num_colors)
{
   png_color palette[256];
   unsigned int i;

   OPNG_ASSERT(color_table != NULL && num_colors <= 256);
   for (i = 0; i < num_colors; ++i)
   {
       palette[i].red   = color_table[3 * i];
       palette[i].green = color_table[3 * i + 1];
       palette[i].blue  = color_table[3 * i + 2];
   }
   png_set_PLTE(png_ptr, info_ptr, palette, (int)num_colors);
}


static void /* PRIVATE */
pngx_set_GIF_transparent(png_structp png_ptr, png_infop info_ptr,
   unsigned int transparent)
{
   png_byte trans[256];
   unsigned int i;

   OPNG_ASSERT(transparent < 256);
   for (i = 0; i < transparent; ++i)
      trans[i] = 255;
   trans[transparent] = 0;
   png_set_tRNS(png_ptr, info_ptr, trans, (int)transparent + 1, NULL);
}


#if 0  /* ... need to implement ... */
static void
pngx_set_GIF_meta(png_structp png_ptr, png_infop info_ptr,
   struct GIFImage *image, struct GIFExtension *ext)
{
   /* If the GIF specifies an aspect ratio, turn it into a pHYs chunk. */
   if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49)
      png_set_pHYs(png_ptr, info_ptr,
         GifScreen.AspectRatio+15, 64, PNG_RESOLUTION_UNKNOWN);

   /* If the GIF specifies an image offset, turn it into a oFFs chunk. */
   if (img->offset_x > 0 && img->offset_y > 0)
      png_set_oFFs(png_ptr, info_ptr,
         img->offset_x, img->offset_y, PNG_OFFSET_PIXEL);
}
#endif


png_charp PNGAPI
pngx_read_gif(png_structp png_ptr, png_infop info_ptr, FILE *fp)
{
   /* GIF-specific data */
   static unsigned char *buf = NULL;
   const unsigned int bufsize = 1024;
   struct GIFScreen screen;
   struct GIFImage image;
   struct GIFExtension ext;
   struct GIFGraphicCtlExt graphicExt;
   int code;
   unsigned char *colorTable;
   unsigned int numColors;
   unsigned int transparent;
   unsigned int numImages;
   /* PNG-specific data */
   png_uint_32 width, height;
   int interlace_type;
   png_bytepp row_pointers;
   png_uint_32 i;

   /* Set up the custom error handling. */
   pngx_err_ptr = png_ptr;
   GIFError     = pngx_gif_error;
   GIFWarning   = pngx_gif_warning;

   /* Read the GIF screen. */
   GIFReadScreen(&screen, fp);

   /* Allocate memory. */
   width  = screen.Width;
   height = screen.Height;
   row_pointers = (png_bytepp)png_malloc(png_ptr, height * sizeof(png_bytep));
   for (i = 0; i < height; ++i)
   {
      row_pointers[i] = (png_bytep)png_malloc(png_ptr, width);
      png_memset(row_pointers[i], screen.Background, width);
   }
   /* Use a static buffer to avoid memory leaks in case of
    * erroneous exits from previous pngx_read_gif() calls.
    * Do NOT use an automatic buffer because the GIF routines
    * might call realloc() later.
    */
   if (buf == NULL)
      buf = (unsigned char *)png_malloc(png_ptr, bufsize);

   GIFInitImage(&image, &screen, row_pointers);
   GIFInitExtension(&ext, &screen, buf, bufsize);
   transparent = (unsigned int)-1;
   numImages   = 0;

   /* Iterate over the GIF file. */
   for ( ; ; )
   {
      code = GIFReadNextBlock(&image, &ext, fp);
      if (code == GIF_IMAGE)  /* ',' */
      {
         if (image.Rows != NULL)
         {
            interlace_type = image.InterlaceFlag ?
               PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;
            colorTable = GIFGetColorTable(&image, &numColors);

            /* Create and populate the PNG structures. */
            png_set_IHDR(png_ptr, info_ptr,
               width, height, 8, PNG_COLOR_TYPE_PALETTE, interlace_type,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            png_set_rows(png_ptr, info_ptr, row_pointers);
            pngx_set_GIF_palette(png_ptr, info_ptr, colorTable, numColors);
            if (transparent < 256)
               pngx_set_GIF_transparent(png_ptr, info_ptr, transparent);

            /* Inform the GIF routines not to read the upcoming images. */
            image.Rows = NULL;
         }
         ++numImages;
         if (numImages == 2)
            png_warning(png_ptr,
               "Multi-image/animated GIF cannot be entirely converted to PNG");
      }
      else if (code == GIF_EXTENSION && ext.Label == GIF_GRAPHICCTL)  /* '!' */
      {
         GIFGetGraphicCtl(&ext, &graphicExt);
         if (image.Rows != NULL && graphicExt.TransparentFlag)
            if (transparent >= 256)
               transparent = graphicExt.Transparent;
      }
      else if (code == GIF_TERMINATOR)  /* ';' */
         break;
   }

   png_free(png_ptr, buf);
   buf = NULL;
   if (image.Rows != NULL)
      png_error(png_ptr, "No image in GIF file");

   /* FIXME:
    * Deallocate row_pointers to fix memory leak,
    * if a png_error() occured before calling png_set_rows().
    */

   return "GIF";  /* success */
}
