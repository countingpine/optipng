/*
 * pngxrpnm.c - libpng external I/O: PNM reader.
 * Copyright (C) 2001-2006 Cosmin Truta.
 */

#include "pngxtern.h"
#include "pnm/pnmio.h"
#include <ctype.h>


static /* PRIVATE */ png_structp pngx_err_ptr = NULL;


static void /* PRIVATE */
pngx_pnm_error(pnm_struct *pnm_ptr, const char *msg)
{
   png_error(pngx_err_ptr, msg);
   if (&pnm_ptr)
      return;
}


static void /* PRIVATE */
pngx_pnm_warning(pnm_struct *pnm_ptr, const char *msg)
{
   png_warning(pngx_err_ptr, msg);
   if (&pnm_ptr)
      return;
}


int PNGAPI
pngx_sig_is_pnm(png_bytep sig, png_size_t len)
{
   if (len < 8)  /* the smallest valid PNM file is the 8-byte PBM (P4) */
      return -1;
   if ((sig[0] == 'P') &&
       (sig[1] >= '1') && (sig[1] <= '6') &&
       (isspace(sig[2]) || sig[2] == '#'))
      return 1;
   return 0;
}


png_charp PNGAPI
pngx_read_pnm(png_structp png_ptr, png_infop info_ptr, FILE *fp)
{
   pnm_struct pnminfo;
   unsigned int pnmsample_size;
   int pnmoverflow;
   unsigned int *pnmrow;
   png_uint_32 rowbytes;
   png_bytepp row_pointers;
   unsigned int sig_bit;
   unsigned int i, j, k;

   pngx_err_ptr = png_ptr;
   pnm_error    = pngx_pnm_error;
   pnm_warning  = pngx_pnm_warning;
   pnmoverflow  = 0;

   pnm_read_header(fp, &pnminfo);

   if (pnminfo.type_code == PNM_P3 || pnminfo.type_code == PNM_P6)
      pnmsample_size = 3;
   else
      pnmsample_size = 1;
   if (pnminfo.maxval > 65535)
      png_error(png_ptr, "Sample depth too big in PNM");
   else if (pnminfo.maxval > 255)
      pnmsample_size *= 2;
   else if (pnminfo.maxval <= 0)  /* this should be checked inside pnmio */
      png_error(png_ptr, "[internal error] Invalid sample depth in PNM");

   pnmrow = (unsigned int *)
      png_malloc(png_ptr, pnmsample_size * pnminfo.width * sizeof(unsigned int));

   png_set_IHDR(png_ptr, info_ptr,
      pnminfo.width, pnminfo.height,
      (pnminfo.maxval <= 255) ? 8 : 16,
      (pnmsample_size <= 2) ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   for (sig_bit = 16; (1U << sig_bit) - 1 > pnminfo.maxval; --sig_bit)
      ;
   if ((1U << sig_bit) - 1 != pnminfo.maxval)
      png_warning(png_ptr,
         "Possibly inexact sample conversion from PNM to PNG");
   else if (sig_bit != 8 && sig_bit != 16
            && (pnmsample_size > 1 || 8 % sig_bit != 0))
   {
      png_color_8 sbit;
      sbit.red = sbit.green = sbit.blue = sbit.gray = (png_byte)sig_bit;
      sbit.alpha = 0;
      png_set_sBIT(png_ptr, info_ptr, &sbit);
   }

   rowbytes = png_get_rowbytes(png_ptr, info_ptr);
   row_pointers = (png_bytepp)
      png_malloc(png_ptr, pnminfo.height * sizeof(png_bytep));
   for (i = 0; i < pnminfo.height; ++i)
      row_pointers[i] = (png_bytep)pngx_zmalloc(png_ptr, rowbytes);
   png_set_rows(png_ptr, info_ptr, row_pointers);

   if (pnminfo.maxval <= 255)
   {
      for (i = 0; i < pnminfo.height; ++i)
      {
         pnm_read_row(fp, &pnminfo, pnmrow);
         if (pnminfo.maxval == 255)
         {
            for (j = 0; j < pnmsample_size * pnminfo.width; ++j)
               row_pointers[i][j] = (png_byte)pnmrow[j];
         }
         else
         {
            for (j = 0; j < pnmsample_size * pnminfo.width; ++j)
            {
               unsigned int b = pnmrow[j];
               if (b > pnminfo.maxval)
               {
                  b = pnminfo.maxval;
                  pnmoverflow = 1;
               }
               row_pointers[i][j] =
                  (png_byte)((b * 255 + pnminfo.maxval/2) / pnminfo.maxval);
            }
         }
      }
   }
   else
   {
      for (i = 0; i < pnminfo.height; ++i)
      {
         pnm_read_row(fp, &pnminfo, pnmrow);
         if (pnminfo.maxval == 65535)
         {
            for (j = k = 0; k < pnmsample_size * pnminfo.width; ++j, k+=2)
            {
               row_pointers[i][k] = (png_byte)(pnmrow[j] / 256);
               row_pointers[i][k + 1] = (png_byte)(pnmrow[j] % 256);
            }
         }
         else
         {
            for (j = k = 0; k < pnmsample_size * pnminfo.width; ++j, k+=2)
            {
               png_uint_32 b = pnmrow[j];
               if (b > pnminfo.maxval)
               {
                  b = pnminfo.maxval;
                  pnmoverflow = 1;
               }
               b = (b * 65535U + pnminfo.maxval/2) / pnminfo.maxval;
               row_pointers[i][k] = (png_byte)(b / 256);
               row_pointers[i][k + 1] = (png_byte)(b % 256);
            }
         }
      }
   }

   free(pnmrow);

   if (pnmoverflow)
      png_warning(png_ptr, "Overflow in PNM pixels");

   switch (pnminfo.type_code)
   {
      case PNM_P1:
      case PNM_P4:
         return "PBM";
      case PNM_P2:
      case PNM_P5:
         return "PGM";
      case PNM_P3:
      case PNM_P6:
         return "PPM";
      default:
         return "PNM";
   }
}
