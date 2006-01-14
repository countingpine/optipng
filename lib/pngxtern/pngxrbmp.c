/*
 * pngxrbmp.c - libpng external I/O: BMP reader.
 * Copyright (C) 2001-2005 Cosmin Truta.
 *
 * Note: only uncompressed BMPs are currently recognized.
 */

#include "pngxtern.h"


/**
 * BMP file header macros
 * Public domain by MIYASAKA Masaru (July 14, 2000)
 **/

/* BMP file signature */
#define BMP_SIGNATURE       0x4d42
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
#define BIH_DSIZE           0       /* DWORD  biSize;           */
#define BIH_LWIDTH          4       /* LONG   biWidth;          */
#define BIH_LHEIGHT         8       /* LONG   biHeight;         */
#define BIH_WPLANES         12      /* WORD   biPlanes;         */
#define BIH_WBITCOUNT       14      /* WORD   biBitCount;       */
#define BIH_DCOMPRESSION    16      /* DWORD  biCompression;    */
#define BIH_DSIZEIMAGE      20      /* DWORD  biSizeImage;      */
#define BIH_LXPELSPERMETER  24      /* LONG   biXPelsPerMeter;  */
#define BIH_LYPELSPERMETER  28      /* LONG   biYPelsPerMeter;  */
#define BIH_DCLRUSED        32      /* DWORD  biClrUsed;        */
#define BIH_DCLRIMPORTANT   36      /* DWORD  biClrImportant;   */
#define INFOHED_SIZE        40      /* sizeof(BITMAPINFOHEADER) */
#define BMPV4HED_SIZE       108     /* sizeof(BITMAPV4HEADER)   */
#define BMPV5HED_SIZE       124     /* sizeof(BITMAPV5HEADER)   */

/* BITMAPCOREHEADER */
#define BCH_DSIZE           0       /* DWORD  bcSize;           */
#define BCH_WWIDTH          4       /* WORD   bcWidth;          */
#define BCH_WHEIGHT         6       /* WORD   bcHeight;         */
#define BCH_WPLANES         8       /* WORD   bcPlanes;         */
#define BCH_WBITCOUNT       10      /* WORD   bcBitCount;       */
#define COREHED_SIZE        12      /* sizeof(BITMAPCOREHEADER) */

/* RGBQUAD */
#define RGBQ_BLUE           0       /* BYTE   rgbBlue;     */
#define RGBQ_GREEN          1       /* BYTE   rgbGreen;    */
#define RGBQ_RED            2       /* BYTE   rgbRed;      */
#define RGBQ_RESERVED       3       /* BYTE   rgbReserved; */
#define RGBQUAD_SIZE        4       /* sizeof(RGBQUAD)     */

/* RGBTRIPLE */
#define RGBT_BLUE           0       /* BYTE   rgbtBlue;    */
#define RGBT_GREEN          1       /* BYTE   rgbtGreen;   */
#define RGBT_RED            2       /* BYTE   rgbtRed;     */
#define RGBTRIPLE_SIZE      3       /* sizeof(RGBTRIPLE)   */

/* Constants for the biCompression field */
#ifndef BI_RGB
#define BI_RGB              0L      /* Uncompressed format       */
#define BI_RLE8             1L      /* RLE format (8 bits/pixel) */
#define BI_RLE4             2L      /* RLE format (4 bits/pixel) */
#define BI_BITFIELDS        3L      /* Bitfield format           */
#define BI_JPEG             4L      /* JPEG format ?             */
#define BI_PNG              5L      /* PNG format ?              */
#endif


/**
 * BMP utilities
 **/
static unsigned int
bmp_get_word(png_bytep ptr)
{
   return
      (unsigned int)ptr[0] + ((unsigned int)ptr[1] << 8);
}

static png_uint_32
bmp_get_dword(png_bytep ptr)
{
   return
      (png_uint_32)ptr[0]         + ((png_uint_32)ptr[1] << 8) +
      ((png_uint_32)ptr[2] << 16) + ((png_uint_32)ptr[3] << 24);
}


/**
 * BMP read support for pngxtern
 **/
int PNGAPI
pngx_sig_is_bmp(png_bytep sig, png_size_t len)
{
   if (len < 2)
      return -1;
   if (sig[0] == 'B' && sig[1] == 'M')
      return 1;
   return 0;
}


png_charp PNGAPI
pngx_read_bmp(png_structp png_ptr, png_infop info_ptr, FILE *fp)
{
   png_uint_32 width, height;
   unsigned int pixdepth, palnum;
   int topdown;
   png_uint_32 rowbytes;
   png_byte bfh[FILEHED_SIZE+BMPV5HED_SIZE];
   png_bytep const bih = bfh + FILEHED_SIZE;
   png_byte rgbq[RGBQUAD_SIZE];
   png_uint_32 offbits, bihsize, skip;
   png_uint_32 compression, palsize;
   int bit_depth, color_type;
   png_color palette[256];
   png_bytepp row_pointers;
   png_bytep sample_ptr, dest_ptr;
   unsigned int i;
   png_uint_32 x, y, yend;
   int yinc;

   /* Read the BMP header. */
   for (i = 0; ; ++i)  /* skip macbinary header */
   {
      if (fread(bfh, (FILEHED_SIZE+BIHSIZE_SIZE), 1, fp) != 1)
         goto err_read;
      if (bmp_get_word(bfh + BFH_WTYPE) == BMP_SIGNATURE)
         break;
      if (i > 0)
         png_error(png_ptr, "Not a BMP file");
      if (fread(bfh, (128-FILEHED_SIZE-BIHSIZE_SIZE), 1, fp) != 1)
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

   if (bihsize == COREHED_SIZE)  /* OS/2-Type BMP */
   {
      width       = bmp_get_word(bih + BCH_WWIDTH);
      height      = bmp_get_word(bih + BCH_WHEIGHT);
      pixdepth    = bmp_get_word(bih + BCH_WBITCOUNT);
      topdown     = 0;
      compression = BI_RGB;
      palsize     = RGBTRIPLE_SIZE;
   }
   else  /* Windows-Type BMP */
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
   if (pixdepth == 24)
      palnum = 0;
   else
   {
      palnum = skip / palsize;
      skip   = skip % palsize;
   }

   /* Validate the BMP header. */
   if ((png_int_32)width <= 0 || (png_int_32)height <= 0 ||
        pixdepth == 0 ||
       (pixdepth != 1 && pixdepth != 4 && pixdepth % 8 != 0))
      png_error(png_ptr, "Invalid BMP file");
   if (pixdepth == 16 || pixdepth > 32)
      png_error(png_ptr, "Unsupported bit depth in BMP");
   if (compression != BI_RGB)
      png_error(png_ptr, "Unsupported compression type in BMP");

   /* Set the PNG image type. */
   if (pixdepth >= 24)
   {
      bit_depth  = 8;
      color_type = PNG_COLOR_TYPE_RGB;
   }
   else if (pixdepth == 8 && palnum == 0)
   {
      bit_depth  = 8;
      color_type = PNG_COLOR_TYPE_GRAY;
   }
   else
   {
      bit_depth = pixdepth;
      color_type = PNG_COLOR_TYPE_PALETTE;
   }
   png_set_IHDR(png_ptr, info_ptr,
      width, height, bit_depth, color_type,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   /* Allocate memory. */
   rowbytes = (width * pixdepth + 31) / 32 * 4;  /* not working on 16 bits! */
   row_pointers = (png_bytepp)png_malloc(png_ptr, height * sizeof(png_bytep));
   for (y = 0; y < height; ++y)
      row_pointers[y] = (png_bytep)pngx_zmalloc(png_ptr, rowbytes);
   png_set_rows(png_ptr, info_ptr, row_pointers);

   if (palnum > 0)
   {
      if (palnum > 256)
      {
         png_warning(png_ptr, "Palette too big in BMP; truncated");
         palnum = 256;
      }
      for (i = 0; i < palnum; ++i)
      {
         if (fread(rgbq, palsize, 1, fp) != 1)
            goto err_read;
         palette[i].red   = rgbq[RGBQ_RED];
         palette[i].green = rgbq[RGBQ_GREEN];
         palette[i].blue  = rgbq[RGBQ_BLUE];
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
   for ( ;; )
   {
      if (fread(row_pointers[y], rowbytes, 1, fp) != 1)
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
      if (y == yend)
         break;
      y += yinc;
   }

   return "BMP";  /* success */

err_read:
   png_error(png_ptr, "Error reading BMP");
   return NULL;
}
