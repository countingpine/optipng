/*
 * pngxrgif.c - libpng external I/O: GIF reader.
 * Copyright (C) 2001-2005 Cosmin Truta.
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
   unsigned char *color_table, int num_colors)
{
   png_color palette[256];
   int i;

   OPNG_ASSERT(num_colors <= 256);
   for (i = 0; i < num_colors; ++i)
   {
       palette[i].red   = color_table[3 * i];
       palette[i].green = color_table[3 * i + 1];
       palette[i].blue  = color_table[3 * i + 2];
   }
   png_set_PLTE(png_ptr, info_ptr, palette, num_colors);
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
   png_set_tRNS(png_ptr, info_ptr, trans, transparent + 1, NULL);
}


#if 0  /* ... need to implement ... */
static void
pngx_gif_blah(png_structp png_ptr, png_infop info_ptr,
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
   static unsigned char *buf = NULL;
   static unsigned int bufsize = 1024;
   struct GIFScreen screen;
   struct GIFImage image;
   struct GIFExtension ext;
   struct GIFGraphicCtlExt graphicExt;
   unsigned char *colorTable;
   unsigned int numColors;
   int multiImage;
   png_uint_32 width, height;
   png_bytepp row_pointers;
   png_uint_32 i;

   /* Set up the custom error handling. */
   pngx_err_ptr = png_ptr;
   GIFError     = pngx_gif_error;
   GIFWarning   = pngx_gif_warning;

   /* Read the GIF screen. */
   GIFReadScreen(&screen, fp);

   /* Set up the PNG header. */
   width  = screen.Width;
   height = screen.Height;
   png_set_IHDR(png_ptr, info_ptr,
      width, height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   /* Allocate memory. */
   row_pointers = (png_bytepp)
      png_malloc(png_ptr, height * sizeof(png_bytep));
   for (i = 0; i < height; ++i)
   {
      row_pointers[i] = (png_bytep)png_malloc(png_ptr, width);
      memset(row_pointers[i], screen.Background, width);
   }
   if (buf == NULL)
      buf = (unsigned char *)malloc(bufsize);

   GIFInitImage(&image, &screen, row_pointers);
   GIFInitExtension(&ext, &screen, buf, bufsize);
   png_set_rows(png_ptr, info_ptr, row_pointers);
   multiImage = 0;

   /* If there is a global palette, use it, before errors appear. */
   if (screen.GlobalColorFlag)
      pngx_set_GIF_palette(png_ptr, info_ptr,
         screen.GlobalColorTable, screen.GlobalNumColors);

   /* Iterate over the GIF file. */
   for ( ;; )
   {
      switch (GIFReadNextBlock(&image, &ext, fp))
      {
         case GIF_TERMINATOR:  /* ';' */
         {
            if (image.Rows != NULL)
               png_error(png_ptr, "Empty GIF file");
            return "GIF";
         }
         case GIF_IMAGE:       /* ',' */
         {
            if (image.Rows != NULL)
            {
               if (image.InterlaceFlag)
                  pngx_set_interlace_type(png_ptr, info_ptr,
                     PNG_INTERLACE_ADAM7);
               colorTable = GIFGetColorTable(&image, &numColors);
               if (colorTable != screen.GlobalColorTable)
                  pngx_set_GIF_palette(png_ptr, info_ptr,
                     colorTable, numColors);
               image.Rows = NULL;  /* only the first image is needed */
            }
            else
            {
               if (!multiImage)
               {
                  png_error(png_ptr,
                     "Multi-image/animated GIF cannot be properly converted to PNG");
                  multiImage = 1;
               }
            }
            break;
         }
         case GIF_EXTENSION:   /* '!' */
         {
            if (image.Rows != NULL && ext.Label == GIF_GRAPHICCTL)
            {
               GIFGetGraphicCtl(&ext, &graphicExt);
               if (graphicExt.TransparentFlag)
                  pngx_set_GIF_transparent(png_ptr, info_ptr,
                     graphicExt.Transparent);
            }
         }
      }
   }
}
