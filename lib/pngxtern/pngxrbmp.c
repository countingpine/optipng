/*
 * pngxrbmp.c - libpng external I/O: BMP reader.
 * Copyright (C) 2001-2006 Cosmin Truta.
 *
 * This code was derived from "bmp2png.c" by MIYASAKA Masaru, and
 * is distributed under the same copyright and warranty terms as libpng.
 *
 * Note: only uncompressed BMP files are currently recognized.
 */

#include "pngxtern.h"


/**
 * BMP file header macros
 * Public domain by MIYASAKA Masaru
 * Updated by Cosmin Truta
 **/

/* BMP file signature */
#define BMP_SIGNATURE       0x4d42  /* "BM" */
#define BMP_SIG_BYTES       2

/* BITMAPFILEHEADER */
#define BFH_WTYPE           0       /* WORD   bfType;           */
#define BFH_DSIZE           2       /* DWORD  bfSize;           */
#define BFH_WRESERVED1      6       /* WORD   bfReserved1;      */
#define BFH_WRESERVED2      8       /* WORD   bfReserved2;      */
#define BFH_DOFFBITS        10      /* DWORD  bfOffBits;        */
#define BFH_DBIHSIZE        14      /* DWORD  biSize;           */
#define FILEHED_SIZE        14      /* sizeof(BITMAPFILEHEADER) */
#define BIHSIZE_SIZE        4       /* sizeof(biSize)           */

/* BITMAPINFOHEADER */
#define BIH_DSIZE           0       /* DWORD  biSize;             */
#define BIH_LWIDTH          4       /* LONG   biWidth;            */
#define BIH_LHEIGHT         8       /* LONG   biHeight;           */
#define BIH_WPLANES         12      /* WORD   biPlanes;           */
#define BIH_WBITCOUNT       14      /* WORD   biBitCount;         */
#define BIH_DCOMPRESSION    16      /* DWORD  biCompression;      */
#define BIH_DSIZEIMAGE      20      /* DWORD  biSizeImage;        */
#define BIH_LXPELSPERMETER  24      /* LONG   biXPelsPerMeter;    */
#define BIH_LYPELSPERMETER  28      /* LONG   biYPelsPerMeter;    */
#define BIH_DCLRUSED        32      /* DWORD  biClrUsed;          */
#define BIH_DCLRIMPORTANT   36      /* DWORD  biClrImportant;     */
#define B4H_DREDMASK        40      /* DWORD  bV4RedMask;         */
#define B4H_DGREENMASK      44      /* DWORD  bV4GreenMask;       */
#define B4H_DBLUEMASK       48      /* DWORD  bV4BlueMask;        */
#define B4H_DALPHAMASK      52      /* DWORD  bV4AlphaMask;       */
#define B4H_DCSTYPE         56      /* DWORD  bV4CSType;          */
#define B4H_XENDPOINTS      60      /* CIEXYZTRIPLE bV4Endpoints; */
#define B4H_DGAMMARED       96      /* DWORD  bV4GammaRed;        */
#define B4H_DGAMMAGREEN     100     /* DWORD  bV4GammaGreen;      */
#define B4H_DGAMMABLUE      104     /* DWORD  bV4GammaBlue;       */
#define B5H_DINTENT         108     /* DWORD  bV5Intent;          */
#define B5H_DPROFILEDATA    112     /* DWORD  bV5ProfileData;     */
#define B5H_DPROFILESIZE    116     /* DWORD  bV5ProfileSize;     */
#define B5H_DRESERVED       120     /* DWORD  bV5Reserved;        */
#define INFOHED_SIZE        40      /* sizeof(BITMAPINFOHEADER)   */
#define BMPV4HED_SIZE       108     /* sizeof(BITMAPV4HEADER)     */
#define BMPV5HED_SIZE       124     /* sizeof(BITMAPV5HEADER)     */

/* BITMAPCOREHEADER */
#define BCH_DSIZE           0       /* DWORD  bcSize;           */
#define BCH_WWIDTH          4       /* WORD   bcWidth;          */
#define BCH_WHEIGHT         6       /* WORD   bcHeight;         */
#define BCH_WPLANES         8       /* WORD   bcPlanes;         */
#define BCH_WBITCOUNT       10      /* WORD   bcBitCount;       */
#define COREHED_SIZE        12      /* sizeof(BITMAPCOREHEADER) */

/* RGB */
#define RGB_BLUE            0       /* BYTE   rgbBlue;     */
#define RGB_GREEN           1       /* BYTE   rgbGreen;    */
#define RGB_RED             2       /* BYTE   rgbRed;      */
#define RGB_RESERVED        3       /* BYTE   rgbReserved; */
#define RGBTRIPLE_SIZE      3       /* sizeof(RGBTRIPLE)   */
#define RGBQUAD_SIZE        4       /* sizeof(RGBQUAD)     */

/* Constants for the biCompression field */
#ifndef BI_RGB
#define BI_RGB              0L      /* Uncompressed format       */
#define BI_RLE8             1L      /* RLE format (8 bits/pixel) */
#define BI_RLE4             2L      /* RLE format (4 bits/pixel) */
#define BI_BITFIELDS        3L      /* Bitfield format           */
#define BI_JPEG             4L      /* JPEG format               */
#define BI_PNG              5L      /* PNG format                */
#endif


/**
 * BMP utilities
 **/
static unsigned int
bmp_get_word(png_bytep ptr)
{
   return (unsigned int)ptr[0] + ((unsigned int)ptr[1] << 8);
}

static png_uint_32
bmp_get_dword(png_bytep ptr)
{
   return ((png_uint_32)ptr[0])       + ((png_uint_32)ptr[1] << 8) +
          ((png_uint_32)ptr[2] << 16) + ((png_uint_32)ptr[3] << 24);
}


/**
 * BMP read support for pngxtern
 **/
int PNGAPI
pngx_sig_is_bmp(png_bytep sig, png_size_t len)
{
   if (len <= 4)
      return -1;
   return (bmp_get_word(sig) == BMP_SIGNATURE);
}


png_charp PNGAPI
pngx_read_bmp(png_structp png_ptr, png_infop info_ptr, FILE *fp)
{
   png_uint_32 width, height;
   unsigned int pixdepth, palnum, palsize;
   int topdown;
   png_size_t rowsize;
   png_byte bfh[FILEHED_SIZE + BMPV5HED_SIZE];
   png_bytep const bih = bfh + FILEHED_SIZE;
   png_byte rgbq[RGBQUAD_SIZE];
   png_uint_32 offbits, bihsize, skip;
   png_uint_32 compression;
   int bit_depth, color_type;
   png_color palette[256];
   png_color_8 sig_bit;
   png_uint_32 rowbytes;
   png_bytepp row_pointers;
   png_bytep sample_ptr, dest_ptr;
   unsigned int i;
   png_uint_32 x, y, yend;
   int yinc;

   /* Read the BMP header. */
   for (i = 0; ; ++i)  /* skip macbinary header */
   {
      if (fread(bfh, (FILEHED_SIZE + BIHSIZE_SIZE), 1, fp) != 1)
         goto err_read;
      if (bmp_get_word(bfh + BFH_WTYPE) == BMP_SIGNATURE)
         break;
      if (i > 0)
         png_error(png_ptr, "Not a BMP file");
      if (fread(bfh, (128 - FILEHED_SIZE - BIHSIZE_SIZE), 1, fp) != 1)
         goto err_read;
   }

   offbits = bmp_get_dword(bfh + BFH_DOFFBITS);
   bihsize = bmp_get_dword(bfh + BFH_DBIHSIZE);
   skip    = offbits - bihsize - FILEHED_SIZE;
   if (bihsize < COREHED_SIZE || bihsize > BMPV5HED_SIZE ||
       offbits < (bihsize + FILEHED_SIZE))
      png_error(png_ptr, "Not a BMP file or Invalid BMP file");

   if (fread((bih + BIHSIZE_SIZE), (bihsize - BIHSIZE_SIZE), 1, fp) != 1)
      goto err_read;

   if (bihsize == COREHED_SIZE)  /* OS/2 BMP */
   {
      width       = bmp_get_word(bih + BCH_WWIDTH);
      height      = bmp_get_word(bih + BCH_WHEIGHT);
      pixdepth    = bmp_get_word(bih + BCH_WBITCOUNT);
      topdown     = 0;
      compression = BI_RGB;
      palsize     = RGBTRIPLE_SIZE;
   }
   else  /* Windows BMP */
   {
      width       = bmp_get_dword(bih + BIH_LWIDTH);
      height      = bmp_get_dword(bih + BIH_LHEIGHT);
      pixdepth    = bmp_get_word(bih + BIH_WBITCOUNT);
      topdown     = 0;
      compression = bmp_get_dword(bih + BIH_DCOMPRESSION);
      palsize     = RGBQUAD_SIZE;
      if ((png_int_32)height < 0)  /* top-down BMP */
      {
         height  = (png_uint_32)(-(png_int_32)height);
         topdown = 1;
      }
   }

   /* Validate the BMP header. */
   if ((png_int_32)width <= 0 || (png_int_32)height <= 0 || pixdepth == 0)
      png_error(png_ptr, "Invalid BMP file");
   if (compression != BI_RGB)
      png_error(png_ptr, "Unsupported compression type in BMP");
   if (32 % pixdepth != 0 && pixdepth != 24)
      png_error(png_ptr, "Invalid pixel depth in BMP");

   /* Compute image parameters. */
   if (pixdepth <= 8)
   {
      palnum = skip / palsize;
      if (palnum > 256)
          palnum = 256;
      skip -= palsize * palnum;
      rowsize = rowbytes = (width + (32 / pixdepth) - 1) / (32 / pixdepth) * 4;
      bit_depth = pixdepth;
      color_type = (palnum > 0) ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_GRAY;
   }
   else
   {
      palnum = 0;
      bit_depth = 8;
      color_type = PNG_COLOR_TYPE_RGB;
      if (width > (png_size_t)(-4) / (pixdepth / 8))
         png_error(png_ptr, "Can't handle exceedingly large BMP dimensions");
      /* Overflow in rowbytes is checked inside png_set_IHDR(). */
      if (pixdepth == 16)
      {
         rowsize  = (png_size_t)((width * 2 + 3) & (~3));
         rowbytes = (width * 3 + 3) & (~3);
      }
      else if (pixdepth == 24)
         rowbytes = rowsize = (png_size_t)((width * 3 + 3) & (~3));
      else if (pixdepth == 32)
         rowbytes = rowsize = (png_size_t)(width * 4);
      else
         goto err_read;  /* this should never happen */
   }

   /* Set the PNG image type. */
   png_set_IHDR(png_ptr, info_ptr,
      width, height, bit_depth, color_type,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   if (pixdepth == 16)
   {
      sig_bit.red = sig_bit.green = sig_bit.blue = 5;
      png_set_sBIT(png_ptr, info_ptr, &sig_bit);
   }

   /* Allocate memory. */
   row_pointers = (png_bytepp)png_malloc(png_ptr, height * sizeof(png_bytep));
   for (y = 0; y < height; ++y)
      row_pointers[y] = (png_bytep)pngx_zmalloc(png_ptr, rowbytes);
   png_set_rows(png_ptr, info_ptr, row_pointers);

   if (palnum > 0)
   {
      for (i = 0; i < palnum; ++i)
      {
         if (fread(rgbq, palsize, 1, fp) != 1)
            goto err_read;
         palette[i].red   = rgbq[RGB_RED];
         palette[i].green = rgbq[RGB_GREEN];
         palette[i].blue  = rgbq[RGB_BLUE];
      }
      png_set_PLTE(png_ptr, info_ptr, palette, palnum);
   }

   if (skip > 0)
      fseek(fp, skip, SEEK_CUR);

   /* Read the image data. */
   if (topdown)
   {
      y = 0;
      yend = height - 1;
      yinc = 1;
   }
   else
   {
      y = height - 1;
      yend = 0;
      yinc = -1;
   }
   for ( ; ; )
   {
      if (fread(row_pointers[y], rowsize, 1, fp) != 1)
         goto err_read;
      if (pixdepth == 24)  /* BGR -> RGB */
      {
         sample_ptr = row_pointers[y];
         for (x = 0; x < width; ++x, sample_ptr += 3)
         {
            png_byte tmp  = sample_ptr[0];
            sample_ptr[0] = sample_ptr[2];
            sample_ptr[2] = tmp;
         }
      }
      else if (pixdepth == 32)  /* BGR0 -> RGB */
      {
         sample_ptr = dest_ptr = row_pointers[y];
         for (x = 0; x < width; ++x, sample_ptr += 4, dest_ptr += 3)
         {
            png_byte r = sample_ptr[2];
            png_byte g = sample_ptr[1];
            png_byte b = sample_ptr[0];
            dest_ptr[0] = r;
            dest_ptr[1] = g;
            dest_ptr[2] = b;
         }
      }
      else if (pixdepth == 16)  /* 0rrrrrgggggbbbbb -> RGB */
      {
         sample_ptr = row_pointers[y] + 2 * (width - 1);
         dest_ptr   = row_pointers[y] + 3 * (width - 1);
         for (x = 0; x < width; ++x, sample_ptr -= 2, dest_ptr -= 3)
         {
            unsigned int pixel15 = sample_ptr[0] + (sample_ptr[1] << 8);
            unsigned int r = (pixel15 & 0x7c00) >> 10;
            unsigned int g = (pixel15 & 0x03e0) >> 5;
            unsigned int b = (pixel15 & 0x001f);
            dest_ptr[0] = (png_byte)((r * 255 + 15) / 31);
            dest_ptr[1] = (png_byte)((g * 255 + 15) / 31);
            dest_ptr[2] = (png_byte)((b * 255 + 15) / 31);
         }
      }
      if (y == yend)
         break;
      y += yinc;
   }

   return "BMP";  /* success */

err_read:
   png_error(png_ptr, "Error reading BMP");
   return NULL;
}
