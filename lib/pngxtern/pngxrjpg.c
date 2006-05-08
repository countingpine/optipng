/*
 * pngxrjpg.c - libpng external I/O: JPEG reader stub.
 * Copyright (C) 2001-2006 Cosmin Truta.
 *
 * From the compression point of view, JPEG-to-PNG conversion
 * is not a worthwhile task, and the complexity incurred by
 * JPEG decoding is not justified.  Instead, we provide a stub
 * that recognizes the JPEG format, without actually processing
 * it.
 */

#include "pngxtern.h"


/* FF D8 FF E0 --> JFIF */
/* FF D8 FF E1 --> EXIF */
/* FF D8 FF F7 --> JPEG-LS */
/* FF 4F FF 51 --> JPEG-2000 codestream */
/* 00 00 00 0C 6A 50 20 20 0D 0A 87 0A --> JPEG-2000 .jp2 */


#define JPEG_SIG_JP2_SIZE 12
#define JPEG_SIG_JPC_SIZE 4
#define JPEG_SIG_SIZE_MAX 12

static const png_byte jpeg_sig_jp2[JPEG_SIG_JP2_SIZE] =
{ 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a };

static const png_byte jpeg_sig_jpc[JPEG_SIG_JPC_SIZE] =
{ 0xff, 0x4f, 0xff, 0x51 };


int PNGAPI
pngx_sig_is_jpeg(png_bytep sig, png_size_t len)
{
   if (len < JPEG_SIG_SIZE_MAX)
      return -1;
   if (sig[0] == 0xff && sig[1] == 0xd8 && sig[2] == 0xff &&
       ((int)sig[3] >= 0xe0 && (int)sig[3] <= 0xfd))
      return 1;  /* JPEG (JFIF, EXIF, etc.), JPEG-LS, etc. */
   if (memcmp(sig, jpeg_sig_jp2, JPEG_SIG_JP2_SIZE) == 0)
      return 2;  /* JPEG-2000 .jp2 */
   if (memcmp(sig, jpeg_sig_jpc, JPEG_SIG_JPC_SIZE) == 0)
      return 2;  /* JPEG-2000 codestream */
   return 0;     /* not JPEG */
}


png_charp PNGAPI
pngx_read_jpeg(png_structp png_ptr, png_infop info_ptr, FILE *fp)
{
   png_byte buf[JPEG_SIG_SIZE_MAX];
   int sig_code;

   if (fread(buf, JPEG_SIG_SIZE_MAX, 1, fp) != 1)
      if (&info_ptr)
         png_error(png_ptr, "Can't read file");
   sig_code = pngx_sig_is_jpeg(buf, JPEG_SIG_SIZE_MAX);
   switch (sig_code)
   {
   case 1:
      png_error(png_ptr, "JPEG support is not implemented");
   case 2:
      png_error(png_ptr, "JPEG-2000 support is not implemented");
   default:
      png_error(png_ptr, "Inconsistent input file");  /* should not happen */
   }
   return "JPEG";
}
