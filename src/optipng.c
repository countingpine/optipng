/**
 ** OptiPNG: a PNG optimization program.
 ** http://www.cs.toronto.edu/~cosmin/pngtech/optipng/
 **
 ** Copyright (C) 2001-2006 Cosmin Truta.
 ** The program is distributed under the same licensing and warranty
 ** terms as libpng.
 **
 ** This program functions as follows:
 ** For each input image, it reduces the bit depth, color type and
 ** palette without losing any information; combines several methods
 ** and strategies of compression; and reencodes the IDAT data using
 ** the best method found. If none of these methods yield a smaller
 ** IDAT, then the original IDAT is preserved.
 ** The output file will have all the IDAT data in a single chunk.
 **
 ** The idea of running multiple trials with different PNG filters
 ** and zlib parameters is based on the pngcrush program by
 ** Glenn Randers-Pehrson.
 **
 ** Requirements:
 **    ANSI C or ISO C compiler and library.
 **    zlib version 1.2.1 or newer (version 1.2.3 is bundled).
 **    libpng version 1.0.18 or a newer libpng version 1.0.X;
 **       or libpng version 1.2.8 or newer;
 **       with the following options enabled:
 **       - PNG_bKGD,hIST,sBIT,tRNS_SUPPORTED
 **       - PNG_INFO_IMAGE_SUPPORTED
 **       - PNG_FREE_ME_SUPPORTED
 **       - OPNG_IMAGE_REDUCTIONS_SUPPORTED
 **       (version 1.0.18 is bundled).
 **    pngxtern (version 0.2 is bundled).
 **    cbitset (version 0.1 is bundled).
 **    cexcept version 1.0.0 or newer (version 2.0.0 is bundled).
 **/

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opng.h"
#include "cbitset.h"
#include "cexcept.h"
#include "osys.h"


#define OPTIPNG_NAME        "OptiPNG"
#define OPTIPNG_VERSION     "0.5"
#define OPTIPNG_COPYRIGHT   "Copyright (C) 2001-2006 Cosmin Truta"


static const char *msg_intro =
   OPTIPNG_NAME " " OPTIPNG_VERSION ": Advanced PNG optimizer.\n"
   OPTIPNG_COPYRIGHT ".\n\n";

static const char *msg_license =
   "This program is open-source software. See LICENSE for more details.\n"
   "\n"
   "Portions of this software are based in part on the work of:\n"
   "  Jean-loup Gailly and Mark Adler (zlib)\n"
   "  Glenn Randers-Pehrson and the PNG Development Group (libpng)\n"
   "  Miyasaka Masaru (BMP support)\n"
   "  David Koblas (GIF support)\n"
   "\n";

static const char *msg_short_help =
   "Type \"optipng -h\" for advanced help.\n"
   "\n"
   "Usage:\n"
   "    optipng [options] files ...\n"
   "Files:\n"
   "    Image files of type: PNG, BMP, GIF, or PNM\n"
   "Basic options:\n"
   "    -h, -help\t\tshow the advanced help\n"
   "    -v\t\t\tverbose mode / show copyright, version and build info\n"
   "    -o  <level>\t\toptimization level (0-7)\t\tdefault 2\n"
   "    -i  <type>\t\tinterlace type (0-1)\t\t\tdefault <input>\n"
   "    -k, -keep\t\tkeep a backup of the modified files\n"
   "    -log\t\tlog messages to \"optipng.log\"\n"
   "    -q, -quiet\t\tquiet mode\n"
   "Examples:\n"
   "    optipng -o5 file.png\t\t\t(moderately slow)\n"
   "    optipng -o7 file.png\t\t\t(very slow)\n"
   "    optipng -i1 -zc4,9 -zs0-2 -f0-2,4-5 file1.png file2.gif file3.bmp\n";

static const char *msg_help =
   "Usage:\n"
   "    optipng [options] files ...\n"
   "Files:\n"
   "    Image files of type: PNG, BMP, GIF, or PNM\n"
   "Basic options:\n"
   "    -h, -help\t\tshow this help\n"
   "    -v\t\t\tverbose mode / show copyright, version and build info\n"
   "    -o  <level>\t\toptimization level (0-7)\t\tdefault 2\n"
#if 0  /* not implemented */
   "    -b  <depth>\t\tbit depth (1,2,4,8,16)\t\t\tdefault <min>\n"
   "    -c  <type>\t\tcolor type (0,2,3,4,6)\t\t\tdefault <input>\n"
#endif
   "    -i  <type>\t\tinterlace type (0-1)\t\t\tdefault <input>\n"
   "    -k, -keep\t\tkeep a backup of the modified files\n"
   "    -log\t\tlog messages to \"optipng.log\"\n"
   "    -q, -quiet\t\tquiet mode\n"
   "Advanced options:\n"
   "    -zc <levels>\tzlib compression levels (1-9)\t\tdefault 9\n"
   "    -zm <levels>\tzlib memory levels (1-9)\t\tdefault 8\n"
   "    -zs <strategies>\tzlib compression strategies (0-3)\tdefault 0-3\n"
#ifdef WBITS_8_OK
   "    -zw <window size>\tzlib window size (32k,16k,8k,4k,2k,1k,512,256)\n"
#else
   "    -zw <window size>\tzlib window size (32k,16k,8k,4k,2k,1k,512)\n"
#endif
   "    -f  <filters>\tPNG delta filters (0-5)\t\t\tdefault 0,5\n"
   "    -nb\t\t\tno bit depth reduction\n"
   "    -nc\t\t\tno color type reduction\n"
   "    -no\t\t\tno output (simulation mode)\n"
   "    -np\t\t\tno palette reduction\n"
#if 0  /* text chunk optimization is not implemented */
   "    -nt\t\t\tno text chunk optimization\n"
#endif
   "    -nz\t\t\tno IDAT recompression (also disable reductions)\n"
   "    -fix\t\tenable error recovery\n"
   "    -force\t\twrite a new output, even if it is bigger than the input\n"
   "    -full\t\tproduce a full report on IDAT (might reduce speed)\n"
   "    -preserve\t\tpreserve file attributes if possible\n"
   "    --\t\t\tstop option switch parsing\n"
   "Optimization level presets:\n"
   "    -o0  <=>  -nz\n"
   "    -o1  <=>  [apply libpng heuristics]\t\t(1 trial)\n"
   "    -o2  <=>  -zc9 -zm8 -zs0-3 -f0,5\t\t(8 trials)\n"
   "    -o3  <=>  -zc9 -zm8-9 -zs0-3 -f0,5\t\t(16 trials)\n"
   "    -o4  <=>  -zc9 -zm8 -zs0-3 -f0-5\t\t(24 trials)\n"
   "    -o5  <=>  -zc9 -zm8-9 -zs0-3 -f0-5\t\t(48 trials)\n"
   "    -o6  <=>  -zc1-9 -zm8 -zs0-3 -f0-5\t\t(120 trials)\n"
   "    -o7  <=>  -zc1-9 -zm8-9 -zs0-3 -f0-5\t(240 trials)\n"
   "Examples:\n"
   "    optipng -o5 file.png\t\t\t(moderately slow)\n"
   "    optipng -o7 file.png\t\t\t(very slow)\n"
   "    optipng -i1 -zc4,9 -zs0-2 -f0-2,4-5 file1.png file2.png\n"
   "Notes:\n"
   "  - The options are cummulative; e.g.\n"
   "    -f0 -f5  <=>  -f0,5\n"
   "    -zs0 -zs1 -zs2  <=>  -zs0,1,2  <=>  -zs0-2\n"
   "  - The option letters are case-insensitive.\n"
   "  - The libpng heuristics consist of:\n"
   "    -o1  <=>  -zc9 -zm8 -zs0 -f0\t\t(if PLTE is present)\n"
   "    -o1  <=>  -zc9 -zm8 -zs1 -f5\t\t(if PLTE is not present)\n"
   "  - The zlib window size is set to a minimum that does not affect\n"
   "    the compression ratio.\n"
   "  - The output file will have all IDAT in a single chunk, even if\n"
   "    no recompression is performed.\n"
   "  - The most exhaustive search  -zc1-9 -zm1-9 -zs0-3 -f0-5  (1080 trials)\n"
   "    is offered only as an advanced option, and it is not recommended.\n";


/** Log file name **/
#define LOG_FILE_NAME "optipng.log"


/** Program tables, limits and presets **/
#define OPTIM_LEVEL_MIN     0
#define OPTIM_LEVEL_MAX     7
#define OPTIM_LEVEL_DEFAULT 2

/*  "-"  <=>  "MIN-MAX"  */

#define COMPR_LEVEL_MIN     1
#define COMPR_LEVEL_MAX     9
static const char *optim_compr_level_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "9", "9", "9", "9", "-", "-" };

#define MEM_LEVEL_MIN       1
#define MEM_LEVEL_MAX       9
static const char *optim_mem_level_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "8", "8-", "8", "8-", "8", "8-" };

#define STRATEGY_MIN        0
#define STRATEGY_MAX        3
static const char *optim_strategy_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "-", "-", "-", "-", "-", "-" };

#define FILTER_MIN          0
#define FILTER_MAX          5
static const char *optim_filter_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "0,5", "0,5", "-", "-", "-", "-" };


/** The only ancillary chunks handled by libpng and OptiPNG **/
static const png_byte sig_bKGD[4] = { 0x62, 0x4b, 0x47, 0x44 };
static const png_byte sig_hIST[4] = { 0x68, 0x49, 0x53, 0x54 };
static const png_byte sig_sBIT[4] = { 0x73, 0x42, 0x49, 0x54 };
static const png_byte sig_tRNS[4] = { 0x74, 0x52, 0x4e, 0x53 };
/** The chunks for which OptiPNG provides special handling **/
static const png_byte sig_IDAT[4] = { 0x49, 0x44, 0x41, 0x54 };
static const png_byte sig_IEND[4] = { 0x49, 0x45, 0x4e, 0x44 };


/** User exception setup -- see cexcept.h for more info **/
define_exception_type(const char *);
struct exception_context the_exception_context[1];


/** OptiPNG-specific info **/
static struct opng_image_struct
{
   png_uint_32 width, height;
   int bit_depth, color_type, compression_type, filter_type, interlace_type;
   png_bytepp row_pointers;       /* IDAT */
   png_colorp palette;            /* PLTE */
   int num_palette;
   png_color_16p background_ptr;
   png_color_16 background;       /* bKGD */
   png_uint_16p hist;             /* hIST */
   png_color_8p sig_bit_ptr;
   png_color_8 sig_bit;           /* sBIT */
   png_bytep trans;               /* tRNS */
   int num_trans;
   png_color_16p trans_values_ptr;
   png_color_16 trans_values;
   png_unknown_chunkp unknowns;
   int num_unknowns;
} opng_image;

static struct opng_info_struct
{
   int input_is_png;
   int valid;
   png_uint_32 file_size, idat_size;
   png_uint_32 best_file_size, best_idat_size, total_idat_size;
   unsigned int num_idat_chunks;
   png_uint_32 crt_row, last_row;
   int crt_ipass, last_ipass;
   png_uint_32 reductions;
   int best_compr_level, best_mem_level, best_strategy, best_filter;
} opng_info;

static struct cmdline_struct
{
   int has_files;
   int help, ver;
   int interlace;
   int keep, log, quiet;
   int nb, nc, no, np, nz;
   int fix, force, full, preserve;
   bitset compr_level_table, mem_level_table, strategy_table, filter_table;
   int window_bits;
} cmdline;

static struct global_struct
{
   FILE *logfile;
   unsigned int err_count, fix_count;
} global;


/** Global variables, for quick access and bonus style points **/
static png_structp read_ptr, write_ptr;
static png_infop read_info_ptr, write_info_ptr;
static png_infop read_end_info_ptr, write_end_info_ptr;


/** Safe memory deallocation **/
static void
opng_free(void *ptr)
{
   /* NOT happy about the standard behavior of free()... */
   if (ptr != NULL)
      free(ptr);
}


/** Message display w/ logging **/
static void
opng_printf(const char *fmt, ...)
{
   va_list arg_ptr;
   FILE *confile;

   va_start(arg_ptr, fmt);

   /* If the message starts with '!', it is sent to stderr. */
   if (fmt[0] == '!')
   {
      ++fmt;
      confile = stderr;
   }
   else
      confile = stdout;
   if (cmdline.quiet)
      confile = NULL;

   if (confile != NULL)
      vfprintf(confile, fmt, arg_ptr);
   if (global.logfile != NULL)
   {
      vfprintf(global.logfile, fmt, arg_ptr);
      fflush(global.logfile);
   }

   va_end(arg_ptr);
}


/** Image info display w/ logging **/
static void
opng_print_image_info(int print_dim, int print_type, int print_interlaced)
{
   static const char *color_type_name[7] =
   {
      "grayscale", "[invalid]", "RGB", "palette",
      "grayscale-alpha", "[invalid]", "RGB-alpha"
   };
   int something_printed = 0;

   if (print_dim)
   {
      opng_printf("%ux%u",
         (unsigned int)opng_image.width, (unsigned int)opng_image.height);
      something_printed = 1;
   }
   if (print_type)
   {
      if (something_printed)
         opng_printf(" ");
      assert(opng_image.color_type < 7);
      opng_printf("%d-bit %s", opng_image.bit_depth,
         color_type_name[opng_image.color_type]);
      something_printed = 1;
   }
   if (print_interlaced)
   {
      if (something_printed)
         opng_printf(" ");
      opng_printf((opng_image.interlace_type == PNG_INTERLACE_ADAM7) ?
         "interlaced" : "non-interlaced");
   }
}


/** Percentage display w/ logging **/
static void
opng_print_percentage(png_uint_32 num, png_uint_32 denom)
{
   if (num <= PNG_MAX_UINT / 100 && denom <= PNG_MAX_UINT / 100)
      num *= 100;
   else
      denom = (denom + 50) / 100;  /* reduce precision to prevent overflow */

   if (denom == 0)
   {
      opng_printf("INFTY%%");
      return;
   }

   num += denom / 200;
   opng_printf("%lu.%02u%%",
      (unsigned long)(num / denom),
      (unsigned int)(num % denom * 100 / denom));
}


/** Size change display w/ logging **/
static void
opng_print_size_difference(png_uint_32 init_size, png_uint_32 final_size,
   int print_percentage)
{
   long difference = (long)final_size - (long)init_size;
   int increase;

   if (difference == 0)
   {
      opng_printf("no change");
      return;
   }

   increase = 1;
   if (difference < 0)
   {
      difference = -difference;
      increase = 0;
   }

   opng_printf("%ld bytes", difference);
   if (print_percentage && init_size > 0)
   {
      opng_printf(" = ");
      opng_print_percentage(difference, init_size);
   }
   opng_printf(increase ? " increase" : " decrease");
}


/** Progress calculation w/ printing and logging **/
static int
opng_progress(void)
{
   static const int progress_factor[7] = {1, 1, 2, 4, 8, 16, 32};
   png_uint_32 height, crt_row, progress;
   int i;

   if (opng_info.crt_row >= opng_info.last_row &&
       opng_info.crt_ipass >= opng_info.last_ipass)
   {
      opng_printf("100%%");
      return 1;  /* finished */
   }

   if (opng_image.interlace_type == PNG_INTERLACE_ADAM7)
   {
      assert(opng_info.crt_ipass < 7);

      /* This code is accurate only if opng_image.height >= 8 */
      height = opng_image.height;
      crt_row = opng_info.crt_row;
      if (height > PNG_MAX_UINT / 64)
      {
         /* Reduce precision to prevent overflow. */
         height  = (height + 32) / 64;
         crt_row = (crt_row + 32) / 64;
      }

      /* Accumulate the previous passes, and the current one */
      progress = 0;
      for (i = 0; i < opng_info.crt_ipass; ++i)
         progress += progress_factor[i] * height;
      progress += progress_factor[i] * crt_row;
      /* Compute the percentage, and make sure it's not beyond 100% */
      height *= 64;
      if (progress < height)
         opng_print_percentage(progress, height);
      else  /* this may happen only if precision was reduced */
         opng_printf("100%%");  /* ... but it isn't really finished */
   }
   else  /* PNG_INTERLACE_NONE */
   {
      assert(opng_info.crt_ipass == 0);
      opng_print_percentage(opng_info.crt_row, opng_image.height);
   }
   return 0;  /* unfinished */
}


/** User error handling **/
static void
opng_error(png_structp png_ptr, png_const_charp msg)
{
   if (&png_ptr)  /* dummy, keep compilers happy */
   {
      opng_info.valid = 0;
      Throw msg;
   }
}


/** User warning **/
static void
opng_warning(png_structp png_ptr, png_const_charp msg)
{
   if (&png_ptr)  /* dummy, keep compilers happy */
   {
      opng_info.valid = 0;
      opng_printf("!Warning: %s\n", msg);
   }
}


/** Query for chunk handling **/
static int
opng_handle_as_unknown(png_bytep chunk_type)
{
   if ((chunk_type[0] & 0x20) == 0  /* critical chunk? */  ||
       memcmp(chunk_type, sig_bKGD, 4) == 0 ||
       memcmp(chunk_type, sig_hIST, 4) == 0 ||
       memcmp(chunk_type, sig_sBIT, 4) == 0 ||
       memcmp(chunk_type, sig_tRNS, 4) == 0)
      return 0;
   return 1;
}


/** User chunk keeping **/
static void
opng_set_keep_unknown_chunk(png_structp png_ptr, png_bytep chunk_type)
{
   png_byte chunk_name[5];

   memcpy(chunk_name, chunk_type, 4);
   chunk_name[4] = 0;
   if (!png_handle_as_unknown(png_ptr, chunk_name))
      png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS,
         chunk_name, 1);
}


/** User progress meter **/
static void
opng_read_write_status(png_structp png_ptr, png_uint_32 row_num, int pass)
{
   if (&png_ptr)  /* dummy, keep compilers happy */
   {
      opng_info.crt_row = row_num;
      opng_info.crt_ipass = pass;
   }
}


/** User reading/writing of data **/
static void
opng_read_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   static png_byte crt_chunk_len[4], crt_chunk_hdr[4];
   static png_uint_32 crt_idat_crc;
   static int crt_chunk_is_idat;
   FILE *fp = (FILE *)png_get_io_ptr(png_ptr);
   int io_state = opng_get_io_state(png_ptr);
   int io_state_loc = io_state & OPNG_IO_MASK_LOC;

   if (length == 0)
      return;
   assert(data != NULL);

   /* Reading is done at the beginning. */
   if (io_state & OPNG_IO_READING)
   {
      assert(fp != NULL);
      if (fread(data, length, 1, fp) != 1)
         png_error(png_ptr,
            "Can't read the input file or unexpected end of file");
   }

   /* Update file_size, idat_size, etc. */
   opng_info.file_size += length;
   if (io_state_loc == OPNG_IO_LEN)
   {
      assert(length == 4);
      memcpy(crt_chunk_len, data, 4);
   }
   else if (io_state_loc == OPNG_IO_HDR)
   {
      assert(length == 4);
      memcpy(crt_chunk_hdr, data, 4);
      if (memcmp(data, sig_IDAT, 4) == 0)
      {
         crt_chunk_is_idat = 1;
         ++opng_info.num_idat_chunks;
         opng_info.idat_size += png_get_uint_32(crt_chunk_len);
         /* Abandon trial if IDAT is already bigger than the smallest IDAT
          * previously found, or if it can't fit into a single chunk.
          */
         if (fp == NULL)
         {
            if ((opng_info.idat_size > opng_info.best_idat_size
                 && !cmdline.full)
                || opng_info.idat_size > PNG_MAX_UINT)
               Throw NULL;
         }
      }
      else
      {
         crt_chunk_is_idat = 0;
         if (opng_handle_as_unknown(data))
            opng_set_keep_unknown_chunk(png_ptr, data);
      }
   }

   /* Don't write anything during trials. */
   if (fp == NULL || !(io_state & OPNG_IO_WRITING))
      return;

   /* Here comes an elaborate way of writing the data, in which
    * multiple IDATs are collapsed in a single chunk.
    * Normally, the user-supplied I/O routines are not so complicated.
    */
   if (opng_info.total_idat_size == 0)  /* if the target is unknown */
      crt_chunk_is_idat = 0;  /* act normally by pretending this isn't IDAT */
   switch (io_state_loc)
   {
      case OPNG_IO_LEN:
         /* Postpone the chunk length writing, to make sure it's not IDAT. */
         return;
      case OPNG_IO_HDR:
      {
         if (crt_chunk_is_idat)
         {
            if (opng_info.num_idat_chunks == 1)  /* the first */
            {
               /* Write the total IDAT length instead of the current one. */
               png_save_uint_32(crt_chunk_len, opng_info.total_idat_size);
               crt_idat_crc = crc32(0, sig_IDAT, 4);
            }
            else
            {
               opng_info.file_size -= 8;
               return;
            }
         }
         else
         {
            /* Sanity check: */
            /* Make sure IDAT is written completely, or not at all. */
            if (opng_info.num_idat_chunks != 0 &&
                opng_info.idat_size != opng_info.total_idat_size)
               png_error(png_ptr, "Internal error (inconsistent IDAT)");
         }
         /* Write the length before the header. */
         if (fwrite(crt_chunk_len, 4, 1, fp) != 1)
            length = 0;  /* this will trigger the error later */
         break;
      }
      case OPNG_IO_DATA:
      {
         if (crt_chunk_is_idat)
            crt_idat_crc = crc32(crt_idat_crc, data, length);
         break;
      }
      case OPNG_IO_CRC:
      {
         if (crt_chunk_is_idat)
         {
            if (opng_info.idat_size < opng_info.total_idat_size)
            {
               opng_info.file_size -= 4;
               return;
            }
            if (opng_info.idat_size > opng_info.total_idat_size)
               png_error(png_ptr, "Internal error (IDAT too big)");
            png_save_uint_32(data, crt_idat_crc);
         }
      }
   }
   if (fwrite(data, length, 1, fp) != 1)
      png_error(png_ptr, "Can't write the output file");
}


/** Image info transfer **/
static void
opng_get_image_info(png_structp png_ptr, png_infop info_ptr,
   png_infop end_info_ptr, int get_ancillary)
{
   memset(&opng_image, 0, sizeof(opng_image));

   png_debug(0, "Loading info struct\n");
   png_get_IHDR(png_ptr, info_ptr,
      &opng_image.width, &opng_image.height, &opng_image.bit_depth,
      &opng_image.color_type, &opng_image.interlace_type,
      &opng_image.compression_type, &opng_image.filter_type);
   opng_image.row_pointers = png_get_rows(png_ptr, info_ptr);
   png_get_PLTE(png_ptr, info_ptr,
      &opng_image.palette, &opng_image.num_palette);

   if (!get_ancillary)
      return;

   if (png_get_bKGD(png_ptr, info_ptr, &opng_image.background_ptr))
   {
      /* Double copying (pointer + value) is necessary here
       * due to an inconsistency in the libpng design.
       */
      opng_image.background = *opng_image.background_ptr;
      opng_image.background_ptr = &opng_image.background;
   }
   png_get_hIST(png_ptr, info_ptr, &opng_image.hist);
   if (png_get_sBIT(png_ptr, info_ptr, &opng_image.sig_bit_ptr))
   {
      /* Same problem... */
      opng_image.sig_bit = *opng_image.sig_bit_ptr;
      opng_image.sig_bit_ptr = &opng_image.sig_bit;
   }
   if (png_get_tRNS(png_ptr, info_ptr,
      &opng_image.trans, &opng_image.num_trans,
      &opng_image.trans_values_ptr))
   {
      /* Same problem... */
      if (opng_image.trans_values_ptr != NULL)
      {
         opng_image.trans_values = *opng_image.trans_values_ptr;
         opng_image.trans_values_ptr = &opng_image.trans_values;
      }
   }
   opng_image.num_unknowns =
      png_get_unknown_chunks(png_ptr, info_ptr, &opng_image.unknowns);

   if (&end_info_ptr == NULL)  /* dummy, end_info_ptr is ignored */
      return;
}


/** Image info transfer **/
static void
opng_set_image_info(png_structp png_ptr, png_infop info_ptr,
   png_infop end_info_ptr, int set_ancillary)
{
   png_debug(0, "Storing info struct\n");
   png_set_IHDR(png_ptr, info_ptr,
      opng_image.width, opng_image.height, opng_image.bit_depth,
      opng_image.color_type, opng_image.interlace_type,
      opng_image.compression_type, opng_image.filter_type);
   png_set_rows(write_ptr, write_info_ptr, opng_image.row_pointers);
   if (opng_image.palette != NULL)
      png_set_PLTE(png_ptr, info_ptr,
         opng_image.palette, opng_image.num_palette);

   if (!set_ancillary)
      return;

   if (opng_image.background_ptr != NULL)
      png_set_bKGD(png_ptr, info_ptr, opng_image.background_ptr);
   if (opng_image.hist != NULL)
      png_set_hIST(png_ptr, info_ptr, opng_image.hist);
   if (opng_image.sig_bit_ptr != NULL)
      png_set_sBIT(png_ptr, info_ptr, opng_image.sig_bit_ptr);
   if (opng_image.trans != NULL || opng_image.trans_values_ptr != NULL)
      png_set_tRNS(png_ptr, info_ptr,
         opng_image.trans, opng_image.num_trans,
         opng_image.trans_values_ptr);
   if (opng_image.num_unknowns != 0)
   {
      int i;
      png_set_unknown_chunks(png_ptr, info_ptr,
         opng_image.unknowns, opng_image.num_unknowns);
      /* Is this really necessary? Shouldn't it be implemented in libpng? */
      for (i = 0; i < opng_image.num_unknowns; ++i)
         png_set_unknown_chunk_location(png_ptr, info_ptr,
            i, opng_image.unknowns[i].location);
   }

   if (&end_info_ptr == NULL)  /* dummy, end_info_ptr is ignored */
      return;
}


/** Image info cleanup **/
static void
opng_free_image_info(void)
{
   png_uint_32 i;
   int j;

   if (opng_image.row_pointers == NULL)
      return;  /* nothing to clean up */

   for (i = 0; i < opng_image.height; ++i)
      opng_free(opng_image.row_pointers[i]);
   opng_free(opng_image.row_pointers);
   opng_free(opng_image.palette);
   opng_free(opng_image.hist);
   opng_free(opng_image.trans);
   for (j = 0; j < opng_image.num_unknowns; ++j)
      opng_free(opng_image.unknowns[j].data);
   opng_free(opng_image.unknowns);
   /* DO NOT deallocate background_ptr, sig_bit_ptr, trans_values_ptr.
    * See the above complaint about an inconsistency in libpng.
    */
   memset(&opng_image, 0, sizeof(opng_image));
}


/** PNG file reading **/
static void
opng_read_png(FILE *infile)
{
   png_uint_32 reductions;
   const char *extern_fmt, *err_msg;

   opng_info.valid = 1;
   opng_info.file_size = opng_info.idat_size = 0;
   opng_info.num_idat_chunks = 0;

   assert(infile != NULL);

   Try
   {
      read_info_ptr = read_end_info_ptr = NULL;
      read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
         NULL, opng_error, opng_warning);
      if (read_ptr != NULL)
      {
         read_info_ptr = png_create_info_struct(read_ptr);
         if (read_info_ptr != NULL)
            read_end_info_ptr = png_create_info_struct(read_ptr);
      }
      if (read_end_info_ptr == NULL)  /* something went wrong on the way */
         Throw "Out of memory";

      png_set_keep_unknown_chunks(read_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);
      opng_set_read_fn(read_ptr, infile, opng_read_write_data);

      png_debug(0, "Reading input file\n");
      extern_fmt = pngx_read_external(read_ptr, read_info_ptr, infile);
      if (extern_fmt != NULL)
      {
         opng_printf("%s format detected\n", extern_fmt);
         opng_info.input_is_png = 0;
         fseek(infile, 0, SEEK_END);
         opng_info.file_size = ftell(infile);
         opng_info.idat_size = 0;
      }
      else
      {
         /* The format is not recognized by pngxtern, so assume it's PNG. */
         png_read_png(read_ptr, read_info_ptr, 0, NULL);
         opng_info.input_is_png = 1;
      }
   }
   Catch (err_msg)
   {
      /* If the critical info has been loaded, treat all errors as warnings.
         This enables a more advanced data recovery. */
      if (opng_validate_image(read_ptr, read_info_ptr))
         opng_warning(read_ptr, err_msg);
      else
      {
         /* Do the cleanup, then rethrow the exception. */
         png_data_freer(read_ptr, read_info_ptr,
            PNG_DESTROY_WILL_FREE_DATA, PNG_FREE_ALL);
         png_data_freer(read_ptr, read_end_info_ptr,
            PNG_DESTROY_WILL_FREE_DATA, PNG_FREE_ALL);
         png_destroy_read_struct(&read_ptr, &read_info_ptr,
            &read_end_info_ptr);
         Throw err_msg;
      }
   }

   opng_get_image_info(read_ptr, read_info_ptr, read_end_info_ptr, 0);
   opng_print_image_info(1, 1, 1);
   opng_printf("\n");

   png_debug(0, "Attempting to reduce image\n");
   reductions = OPNG_REDUCE_ALL;
   if (cmdline.nb)
      reductions &= ~OPNG_REDUCE_BIT_DEPTH;
   if (cmdline.nc)
      reductions &= ~OPNG_REDUCE_COLOR_TYPE;
   if (cmdline.np)
      reductions &= ~OPNG_REDUCE_PALETTE;
   opng_info.reductions = reductions =
      opng_reduce_image(read_ptr, read_info_ptr, reductions);

   opng_get_image_info(read_ptr, read_info_ptr, read_end_info_ptr, 1);

   if (reductions != OPNG_REDUCE_NONE)
   {
      if (reductions & (OPNG_REDUCE_BIT_DEPTH | OPNG_REDUCE_COLOR_TYPE))
      {
         opng_printf("The image is losslessly reduced to ");
         opng_print_image_info(0, 1, 0);
         opng_printf("\n");
      }
      if (reductions & OPNG_REDUCE_PALETTE)
         opng_printf(
            "The color palette or transparency is losslessly reduced.\n");
   }

   png_debug(0, "Destroying data structs\n");
   /* Leave the data for upcoming processing. */
   png_data_freer(read_ptr, read_info_ptr, PNG_USER_WILL_FREE_DATA,
      PNG_FREE_ALL);
   png_data_freer(read_ptr, read_end_info_ptr, PNG_USER_WILL_FREE_DATA,
      PNG_FREE_ALL);
   png_destroy_read_struct(&read_ptr, &read_info_ptr, &read_end_info_ptr);
}


/** PNG file writing **/
/** If the file name is NULL, opng_read_write_file() is still called,
    but no file is written. **/
static void
opng_write_png(FILE *outfile,
   int compression_level, int memory_level,
   int compression_strategy, int filter)
{
   const char *volatile err_msg;  /* volatile is required by cexcept */

   static int filter_table[FILTER_MAX + 1] =
   {
      PNG_FILTER_NONE, PNG_FILTER_SUB, PNG_FILTER_UP,
      PNG_FILTER_AVG, PNG_FILTER_PAETH, PNG_ALL_FILTERS
   };

   assert(compression_level >= COMPR_LEVEL_MIN &&
          compression_level <= COMPR_LEVEL_MAX);
   assert(memory_level >= MEM_LEVEL_MIN &&
          memory_level <= MEM_LEVEL_MAX);
   assert(compression_strategy >= STRATEGY_MIN &&
          compression_strategy <= STRATEGY_MAX);
   assert(filter >= FILTER_MIN &&
          filter <= FILTER_MAX);

   opng_info.file_size = opng_info.idat_size = 0;
   opng_info.num_idat_chunks = 0;

   Try
   {
      write_info_ptr = write_end_info_ptr = NULL;
      write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
         NULL, opng_error, opng_warning);
      if (write_ptr != NULL)
      {
         write_info_ptr = png_create_info_struct(write_ptr);
         if (write_info_ptr != NULL)
            write_end_info_ptr = png_create_info_struct(write_ptr);
      }
      if (write_end_info_ptr == NULL)  /* something went wrong on the way */
         Throw "Out of memory";

      png_set_compression_level(write_ptr, compression_level);
      png_set_compression_mem_level(write_ptr, memory_level);
      png_set_compression_strategy(write_ptr, compression_strategy);
      png_set_filter(write_ptr, PNG_FILTER_TYPE_BASE, filter_table[filter]);
      if (compression_strategy != Z_HUFFMAN_ONLY &&
          compression_strategy != Z_RLE)
      {
         if (cmdline.window_bits > 0)
            png_set_compression_window_bits(write_ptr, cmdline.window_bits);
      }
      else
      {
#ifdef WBITS_8_OK
         png_set_compression_window_bits(write_ptr, 8);
#else
         png_set_compression_window_bits(write_ptr, 9);
#endif
      }

      png_set_keep_unknown_chunks(write_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);

      /* The ancillary data is necessary during the final writing,
         and also whenever some reductions have been performed. */
      opng_set_image_info(write_ptr, write_info_ptr, write_end_info_ptr,
         outfile != NULL || opng_info.reductions != OPNG_REDUCE_NONE ? 1 : 0);

      /* Enable progress estimation during trials. */
      if (outfile == NULL)
      {
         opng_info.last_ipass = 0;
         if (!cmdline.full)
         {
            opng_info.last_row = opng_image.height - 1;
            if (opng_image.interlace_type == PNG_INTERLACE_ADAM7)
            {
               if (opng_image.height >= 8)
               {
                  opng_info.last_row &= ~1;
                  opng_info.last_ipass = 6;
               }
               else
                  opng_info.last_row = 0;
            }
            png_set_write_status_fn(write_ptr, opng_read_write_status);
         }
         else
            opng_info.last_row = 0;
      }

      png_debug(0, "Writing PNG file\n");
      opng_set_write_fn(write_ptr, outfile, opng_read_write_data, NULL);
      png_write_png(write_ptr, write_info_ptr, 0, NULL);

      err_msg = NULL;  /* everything is ok */
   }
   Catch (err_msg)
   {
      /* Set IDAT size to invalid. */
      opng_info.idat_size = PNG_MAX_UINT + 1;
   }

   png_debug(0, "Destroying data structs\n");
   png_destroy_info_struct(write_ptr, &write_end_info_ptr);
   png_destroy_write_struct(&write_ptr, &write_info_ptr);

   if (err_msg != NULL)
      Throw err_msg;
}


/** PNG file copying **/
static void
opng_copy_png(FILE *infile, FILE *outfile)
{
   volatile png_bytep buf;  /* volatile is required by cexcept */
   png_uint_32 buf_size, length;
   png_byte chunk_name[4];
   const char *volatile err_msg;

   assert(infile != NULL && outfile != NULL);

   opng_info.file_size = opng_info.idat_size = 0;
   opng_info.num_idat_chunks = 0;

   write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      NULL, opng_error, opng_warning);
   if (write_ptr == NULL)
      Throw "Out of memory";
   opng_set_write_fn(write_ptr, outfile, opng_read_write_data, NULL);

   Try
   {
      buf_size = 4096;
      buf = (png_bytep)png_malloc(write_ptr, buf_size);

      /* Copy the signature. */
      if (fread(buf, 8, 1, infile) != 1 || png_sig_cmp(buf, 0, 8) != 0)
         Throw "Not a PNG file";
      png_write_sig(write_ptr);

      do
      {
         /* Copy each chunk. */
         if (fread(buf, 8, 1, infile) != 1)  /* length + header */
            Throw "Read error";
         length = png_get_uint_32(buf);
         memcpy(chunk_name, buf + 4, 4);
         if (length > buf_size)
         {
            /* Don't use realloc() because it is slower. */
            opng_free(buf);
            buf_size = length;
            buf = (png_bytep)png_malloc(write_ptr, buf_size);
         }
         if (fread(buf, 1, length, infile) != length)  /* data */
            Throw "Read error";
         png_write_chunk(write_ptr, chunk_name, buf, length);
         fread(buf, 4, 1, infile);  /* crc */
      } while (memcmp(chunk_name, sig_IEND, 4) != 0);

      err_msg = NULL;  /* everything is ok */
   }
   Catch (err_msg)
   {
   }

   opng_free(buf);
   png_destroy_write_struct(&write_ptr, NULL);

   if (err_msg != NULL)
      Throw err_msg;
}


/** IDAT minimization via brute-force trials **/
static void
opng_minimize_idat(void)
{
   bitset compr_level_table = cmdline.compr_level_table;
   bitset mem_level_table   = cmdline.mem_level_table;
   bitset strategy_table    = cmdline.strategy_table;
   bitset filter_table      = cmdline.filter_table;
   int compr_level, mem_level, strategy, filter;

   /* Initialize the output. */
   opng_info.best_file_size   = opng_info.best_idat_size = PNG_MAX_UINT + 1;
   opng_info.best_compr_level = opng_info.best_mem_level =
      opng_info.best_strategy = opng_info.best_filter = -1;

   /* Replace the empty tables with the libpng's "best guess" heuristics. */
   if (compr_level_table == BITSET_EMPTY)
      BITSET_SET(compr_level_table, Z_BEST_COMPRESSION);  /* 9 */
   if (mem_level_table == BITSET_EMPTY)
      BITSET_SET(mem_level_table, 8);
   if (opng_image.bit_depth < 8 || opng_image.palette != NULL)
   {
      if (strategy_table == BITSET_EMPTY)
         BITSET_SET(strategy_table, Z_DEFAULT_STRATEGY);  /* 0 */
      if (filter_table == BITSET_EMPTY)
         BITSET_SET(filter_table, 0);
   }
   else
   {
      if (strategy_table == BITSET_EMPTY)
         BITSET_SET(strategy_table, Z_FILTERED);          /* 1 */
      if (filter_table == BITSET_EMPTY)
         BITSET_SET(filter_table, 5);
   }

   /* Iterate through the "hyper-rectangle" (zc, zm, zs, f). */
   opng_printf("Trying...\n");
   for (filter = FILTER_MIN; filter <= FILTER_MAX; ++filter)
      if (BITSET_GET(filter_table, filter))
         for (strategy = STRATEGY_MIN; strategy <= STRATEGY_MAX; ++strategy)
            if (BITSET_GET(strategy_table, strategy))
            {
               /* The compression level has no significance under
                  Z_HUFFMAN_ONLY or Z_RLE. */
               bitset saved_level_table = compr_level_table;
               if (strategy == Z_HUFFMAN_ONLY)
               {
                  compr_level_table = BITSET_EMPTY;
                  BITSET_SET(compr_level_table, 1);
               }
               else if (strategy == Z_RLE)
               {
                  compr_level_table = BITSET_EMPTY;
                  BITSET_SET(compr_level_table, 9);  /* use deflate_slow */
               }
               for (compr_level = COMPR_LEVEL_MAX;
                    compr_level >= COMPR_LEVEL_MIN; --compr_level)
                  if (BITSET_GET(compr_level_table, compr_level))
                  {
                     for (mem_level = MEM_LEVEL_MAX;
                          mem_level >= MEM_LEVEL_MIN; --mem_level)
                        if (BITSET_GET(mem_level_table, mem_level))
                        {
                           opng_printf(
                              "  zc = %d  zm = %d  zs = %d  f = %d\t\t",
                              compr_level, mem_level, strategy, filter);
                           opng_write_png(NULL,
                              compr_level, mem_level, strategy, filter);
                           if (opng_info.idat_size > PNG_MAX_UINT)
                           {
                              opng_printf("IDAT too big");
                              if (cmdline.ver)  /* verbose */
                              {
                                 opng_printf(" ... abandoned at ");
                                 opng_progress();
                              }
                              opng_printf("\n");
                              continue;
                           }
                           opng_printf("IDAT size = %lu\n",
                              (unsigned long)opng_info.idat_size);
                           if (opng_info.best_idat_size < opng_info.idat_size)
                              continue;
                           if (opng_info.best_idat_size == opng_info.idat_size
                               && opng_info.best_strategy >= Z_HUFFMAN_ONLY)
                              continue;  /* it's not a better combination */
                           opng_info.best_file_size   = opng_info.file_size;
                           opng_info.best_idat_size   = opng_info.idat_size;
                           opng_info.best_compr_level = compr_level;
                           opng_info.best_mem_level   = mem_level;
                           opng_info.best_strategy    = strategy;
                           opng_info.best_filter      = filter;
                        }
                  }
               compr_level_table = saved_level_table;
            }

   /* At least one trial must have been performed. */
   if (opng_info.best_compr_level < 0)  /* any best_ value can be tested */
      Throw "Invalid iteration parameters (-zc, -zm, -zs, -f)";

   if (opng_info.best_idat_size > PNG_MAX_UINT)
      Throw "No satisfactory IDAT was found";
}


/** PNG file optimization **/
static void
opng_optimize_png(const char *filename)
{
   static FILE *infile, *outfile;        /* static or volatile is required */
   volatile enum {none, join, recompress, create} action;    /* by cexcept */
   png_uint_32 init_file_size, init_idat_size;
   char bak_filename[FILENAME_MAX], out_filename[FILENAME_MAX];
   const char *volatile err_msg;

   opng_printf("** Processing %s\n", filename);

   memset(&opng_info, 0, sizeof(opng_info));
   action = none;
   if (cmdline.force)
      action = recompress;

   err_msg = NULL;  /* prepare for error handling */

   if ((infile = fopen(filename, "rb")) == NULL)
      Throw "Can't open the input file";
   Try
   {
      opng_read_png(infile);
   }
   Catch (err_msg)
   {
      /* assert(err_msg != NULL); */
   }
   fclose(infile);  /* finally */
   if (err_msg != NULL)
      Throw err_msg;  /* rethrow */

   /* If there's more than one IDAT in input, join all into a single one. */
   if (opng_info.num_idat_chunks > 1)
      action = join;

   /* If the input is not PNG, enforce full compression. */
   if (!opng_info.input_is_png)
   {
      action = create;
      /* Also make sure it's possible to write the output. */
      if (osys_fname_chext(out_filename, sizeof(out_filename), filename,
          ".png") == NULL)
         Throw "Can't create the output file (name too long)";
      if (osys_fname_cmp(filename, out_filename) == 0)
         action = recompress;
      else if ((outfile = fopen(out_filename, "rb")) != NULL)
      {
         fclose(outfile);
         if (!cmdline.keep)
            Throw "The output file exists, try backing it up (use -keep)";
      }
   }

   /* If the input is invalid, but recoverable, enforce full compression. */
   if (!opng_info.valid)
   {
      opng_printf("!Recoverable errors encountered. ");
      if (cmdline.fix)
      {
         opng_info.valid = 1;
         opng_printf("!Fixing...\n");
         if (action != create)
            action = recompress;
         ++global.fix_count;
      }
      else
      {
         opng_printf("!Rerun the program with the -fix option.\n");
         Throw "Previous error(s) not fixed";
      }
   }

   /* If the interlace type is changed, enforce full compression. */
   if (cmdline.interlace >= 0 &&
       opng_image.interlace_type != cmdline.interlace)
   {
      opng_image.interlace_type = cmdline.interlace;
      action = recompress;
   }

   init_file_size = opng_info.file_size;
   init_idat_size = opng_info.total_idat_size = opng_info.idat_size;
   opng_printf("Input file size = %lu bytes\n",
      (unsigned long)init_file_size);
   if (opng_info.input_is_png)
      opng_printf("Input IDAT size = %lu bytes\n",
         (unsigned long)init_idat_size);

   if (opng_info.input_is_png && cmdline.nz && action == recompress)
      opng_printf("!Warning: IDAT recompression is enforced.\n");

   /* Find the best parameters, and see if it's worth recompressing. */
   if (action == create || action == recompress || !cmdline.nz)
   {
      opng_minimize_idat();
      opng_printf("\nThe best parameters are:\n"
         "  zc = %d  zm = %d  zs = %d  f = %d\t\tIDAT size = %lu\n",
         opng_info.best_compr_level, opng_info.best_mem_level,
         opng_info.best_strategy, opng_info.best_filter,
         (unsigned long)opng_info.best_idat_size);
      if (action != create)
      {
         if (opng_info.reductions != OPNG_REDUCE_NONE)
         {
            if (opng_info.best_file_size < init_file_size)
               action = recompress;
         }
         else
         {
            if (opng_info.best_idat_size < init_idat_size)
               action = recompress;
         }
         if (action == recompress)
             opng_info.total_idat_size = opng_info.best_idat_size;
      }
      /* If action == create then opng_info.total_idat_size is set below. */
   }

   if (action == none)
   {
      opng_printf("\n%s is already optimized.\n\n", filename);
      return;
   }
   if (cmdline.no)
   {
      opng_printf("\nSimulation mode: %s not changed.\n\n", filename);
      return;
   }

   if (action == join || action == recompress)
   {
      /* Rename the input to a backup name, and write the output. */
      if (osys_fname_mkbak(bak_filename,sizeof(bak_filename),filename) == NULL
          || rename(filename, bak_filename) != 0)
         Throw "Can't back up the input file";
      Try
      {
         if ((outfile = fopen(filename, "wb")) == NULL)
            Throw "Can't open the output file";

         if (action == join)  /* copy in to out, collapsing IDAT */
         {
            if ((infile = fopen(bak_filename, "rb")) == NULL)
               Throw "Can't reopen the input file";
            Try
            {
               opng_copy_png(infile, outfile);
            }
            Catch (err_msg)
            {
               /* assert(err_msg != NULL); */
            }
            fclose(infile);  /* finally */
            if (err_msg != NULL)
               Throw err_msg;  /* rethrow */
         }
         else  /* action == recompress: full rewrite */
         {
            opng_write_png(outfile,
               opng_info.best_compr_level, opng_info.best_mem_level,
               opng_info.best_strategy, opng_info.best_filter);
         }
      }
      Catch (err_msg)
      {
         if (outfile != NULL)
            fclose(outfile);
         /* Restore the original input file and rethrow the exception. */
         if (remove(filename) != 0 || rename(bak_filename, filename) != 0)
            opng_printf("!Warning: "
               "The original file was not recovered from the backup.\n");
         Throw err_msg;  /* rethrow */
      }
      fclose(outfile);

      if (cmdline.preserve)
      {
         /* Preserve the file attributes, if possible. */
         osys_fattr_cpy(filename, bak_filename);
      }
      if (!cmdline.keep)
      {
         /* Remove the old file. */
         if (remove(bak_filename) != 0)
            Throw "Can't remove the backup file";
      }
   }
   else
   {
      assert(action == create);  /* this is much similar to recompression */

      /* Create a new output file whose name is in out_filename. */
      assert(out_filename[0] == filename[0]);
      opng_info.total_idat_size = opng_info.best_idat_size;
      if ((outfile = fopen(out_filename, "rb")) != NULL)
      {
         fclose(outfile);
         if (cmdline.keep)
         {
            /* Rename the input to a backup name, and write the output. */
            if (osys_fname_mkbak(bak_filename, sizeof(bak_filename),
                                 out_filename) == NULL
                || rename(out_filename, bak_filename) != 0)
               Throw "Can't back up the output file";
         }
         else
            Throw "The output file exists, try backing it up (use -keep)";
      }
      if ((outfile = fopen(out_filename, "wb")) == NULL)
         Throw "Can't open the output file";
      Try
      {
         opng_write_png(outfile,
            opng_info.best_compr_level, opng_info.best_mem_level,
            opng_info.best_strategy, opng_info.best_filter);
      }
      Catch (err_msg)
      {
         /* assert(err_msg != NULL); */
      }
      fclose(outfile);  /* finally */
      if (err_msg != NULL)
         Throw err_msg;  /* rethrow */

      if (cmdline.preserve)
         osys_fattr_cpy(out_filename, filename);
   }

   opng_printf("\nNew file size = %lu bytes (",
      (unsigned long)opng_info.file_size);
   opng_print_size_difference(init_file_size, opng_info.file_size, 1);

   opng_printf(")\nNew IDAT size = %lu bytes",
      (unsigned long)opng_info.idat_size);
   if (opng_info.input_is_png)
   {
      opng_printf(" (");
      opng_print_size_difference(init_idat_size, opng_info.idat_size, 0);
      opng_printf(")\n\n");
   }
   else
      opng_printf("\n\n");
}


/** Optimization level presets **/
static void
opng_load_level_presets(int optim_level)
{
   int load_defaults;

   if (optim_level < 0)
   {
      optim_level = OPTIM_LEVEL_DEFAULT;
      load_defaults = 1;
   }
   else
      load_defaults = 0;

   if (!load_defaults || cmdline.compr_level_table == BITSET_EMPTY)
      cmdline.compr_level_table |=
         text_to_bitset(optim_compr_level_presets[optim_level]);
   if (!load_defaults || cmdline.mem_level_table == BITSET_EMPTY)
      cmdline.mem_level_table |=
         text_to_bitset(optim_mem_level_presets[optim_level]);
   if (!load_defaults || cmdline.strategy_table == BITSET_EMPTY)
      cmdline.strategy_table |=
         text_to_bitset(optim_strategy_presets[optim_level]);
   if (!load_defaults || cmdline.filter_table == BITSET_EMPTY)
      cmdline.filter_table |=
         text_to_bitset(optim_filter_presets[optim_level]);
}


/** Command line parsing **/
static void
parse_args(int argc, char *argv[])
{
   char *arg;
   /* char */ int cmd;
   bitset interlace_table, optim_level_table;
   int stop_switch, optim_level, i, j;

   /* Initialize. */
   memset(&cmdline, 0, sizeof(cmdline));
   interlace_table = optim_level_table = BITSET_EMPTY;
   optim_level = -1;

   /* Parse the args. */
   stop_switch = 0;
   for (i = 1; i < argc; ++i)
   {
      arg = argv[i];
      if (arg[0] != '-' || stop_switch)
      {
         cmdline.has_files = 1;
         continue;
      }

      do ++arg;  /* multiple dashes are as good as one */
         while (arg[0] == '-');
      argv[i][0] = 0;  /* allow process_args() to skip it */
      if (argv[i][1] == '-' && arg[0] == 0)
      {
         stop_switch = 1;
         continue;
      }

      /* Options are case insensitive. */
      /* It's silly that tolower is ANSI C, but strlwr is not. */
      for (j = 0; arg[j] != 0; ++j)
         arg[j] = (char)tolower(arg[j]);

      if (strcmp(arg, "h") == 0 || strcmp(arg, "help") == 0)
      {
         cmdline.help = 1;
         continue;
      }
      if (strcmp(arg, "v") == 0)
      {
         cmdline.ver = 1;
         continue;
      }
      if (strcmp(arg, "k") == 0 || strcmp(arg, "keep") == 0)
      {
         cmdline.keep = 1;
         continue;
      }
      if (strcmp(arg, "log") == 0)
      {
         cmdline.log = 1;
         continue;
      }
      if (strcmp(arg, "q") == 0 || strcmp(arg, "quiet") == 0)
      {
         cmdline.quiet = 1;
         continue;
      }
      if (strcmp(arg, "fix") == 0)
      {
         cmdline.fix = 1;
         continue;
      }
      if (strcmp(arg, "force") == 0)
      {
         cmdline.force = 1;
         continue;
      }
      if (strcmp(arg, "full") == 0)
      {
         cmdline.full = 1;
         continue;
      }
      if (strcmp(arg, "preserve") == 0)
      {
         cmdline.preserve = 1;
         continue;
      }
      if (arg[0] == 'n' && arg[1] != 0 && arg[2] == 0)
      {
         switch (arg[1])
         {
            case 'b':
               cmdline.nb = 1;
               break;
            case 'c':
               cmdline.nc = 1;
               break;
            case 'o':
               cmdline.no = 1;
               break;
            case 'p':
               cmdline.np = 1;
               break;
            case 'z':
               cmdline.nz = 1;
               break;
            default:
               argv[i][0] = '-';  /* put back the '-' */
               Throw argv[i];
         }
         continue;
      }

      /* Parse the numeric or bitset parameters. */
      cmd = arg[0];
      if (cmd == 'z')
         cmd = toupper((++arg)[0]);
      if ((arg[1] < 'a' || arg[1] > 'z') && cmd != 0 &&
          strchr("ioCMSWf", cmd))
      {
         ++arg;
         if (arg[0] == 0)
            arg = argv[++i];
      }
      else
      {
         argv[i][0] = '-';  /* put back the '-' */
         Throw argv[i];
      }

      switch (cmd)
      {
#if 0  /* not implemented */
         case 'b':  /* bit depth */
            if (sscanf(arg, "%u", &cmdline.bit_depth) < 1 ||
                cmdline.bit_depth < 1 || cmdline.bit_depth > 16 ||
                bitset_count((bitset)cmdline.bit_depth) != 1)
               Throw /* invalid */ "bit depth";
            break;
         case 'c':  /* color type */
            /* ... */
            break;
#endif
         case 'i':  /* interlace type */
            if (sscanf(arg, "%u", &cmdline.interlace) < 1 ||
                (cmdline.interlace & ~1) != 0 ||
                !BITSET_IS_VALID(interlace_table |= text_to_bitset(arg)))
               Throw /* invalid */ "interlace type";
            if (bitset_count(interlace_table) > 1)
               Throw "multiple interlace types are not permitted";
            break;
         case 'o':  /* optimization level */
            if (sscanf(arg, "%u", &optim_level) < 1 ||
                !BITSET_IS_VALID(optim_level_table |= text_to_bitset(arg)))
               Throw /* invalid */ "optimization level";
            if (bitset_count(optim_level_table) > 1)
               Throw "multiple optimization levels are not permitted";
            break;
         case 'C':  /* zlib compression level */
            if (!BITSET_IS_VALID(cmdline.compr_level_table |=
                text_to_bitset(arg)))
               Throw /* invalid */ "compression level(s)";
            break;
         case 'M':  /* zlib memory level */
            if (!BITSET_IS_VALID(cmdline.mem_level_table |=
                text_to_bitset(arg)))
               Throw /* invalid */ "memory level(s)";
            break;
         case 'S':  /* zlib strategy */
            if (!BITSET_IS_VALID(cmdline.strategy_table |=
                text_to_bitset(arg)))
               Throw /* invalid */ "strategy";
            break;
         case 'W':  /* zlib window size */
         {
            unsigned int wsize;
            int wbits;
            char wk;
            int nscanf = sscanf(arg, "%u%c", &wsize, &wk);
            if (nscanf == 0)
               wsize = 0;
            else if (nscanf == 2)
            {
               if (tolower(wk) == 'k' && wsize <= 32)
                  wsize *= 1024;
               else
                  wsize = 0;
            }
            for (wbits = 15; wbits >= 8; --wbits)
               if ((1U << wbits) == wsize)
                  break;
            if (wbits < 8)
               Throw /* invalid */ "window size";
#ifndef WBITS_8_OK
            else if (wbits == 8)
               wbits = 9;
#endif
            if (cmdline.window_bits == 0)
               cmdline.window_bits = wbits;
            else
               Throw "multiple window sizes are not permitted";
            break;
         }
         case 'f':  /* PNG filter */
            if (!BITSET_IS_VALID(cmdline.filter_table |= text_to_bitset(arg)))
               Throw /* invalid */ "filter(s)";
            break;
         default:
            if (argv[i][0] == 0)
               argv[i][0] = '-';  /* put back the '-' */
            Throw argv[i];
      }
      arg[0] = 0;  /* allow process_args() to skip it */
   }

   /* Finalize the data. */
   if (interlace_table == BITSET_EMPTY)
      cmdline.interlace = -1;

   if (optim_level_table == BITSET_EMPTY)
      opng_load_level_presets(-1);
   else if (optim_level <= OPTIM_LEVEL_MIN)
      cmdline.nz = 1;
   else
   {
      if (optim_level > OPTIM_LEVEL_MAX)
         optim_level = OPTIM_LEVEL_MAX;
      opng_load_level_presets(optim_level);
   }

   if (cmdline.nz)
      cmdline.nb = cmdline.nc = cmdline.np = 1;
}


/** Command line processing **/
static void
process_args(int argc, char *argv[])
{
   const char *err_msg;
   volatile int i;  /* no need to be volatile, but it keeps compilers happy */

   for (i = 1; i < argc; ++i)
   {
      if (argv[i][0] == 0)
         continue;
      Try
      {
         opng_optimize_png(argv[i]);
      }
      Catch (err_msg)
      {
         opng_printf("!\nError: %s\n\n", err_msg);
         ++global.err_count;
      }
      opng_free_image_info();
   }
}


/** main **/
int
main(int argc, char *argv[])
{
   const char *option;
   int result;

   Try
   {
      parse_args(argc, argv);
   }
   Catch (option)
   {
      fprintf(stderr, "Invalid option: %s\n", option);
      return EXIT_FAILURE;
   }

   memset(&global, 0, sizeof(global));
   if (cmdline.log)
   {
      if ((global.logfile = fopen(LOG_FILE_NAME, "a")) == NULL)  /* append */
      {
         fprintf(stderr, "Error: Can't open the log file " LOG_FILE_NAME "\n");
         return EXIT_FAILURE;
      }
   }

   result = EXIT_SUCCESS;

   opng_printf(msg_intro);
   if (cmdline.ver)
   {
      opng_printf(msg_license);
      opng_printf("Compiled with libpng version %s and zlib version %s\n\n",
         png_get_libpng_ver(NULL), zlibVersion());
   }
   if (cmdline.help)
   {
      opng_printf(msg_help);
      if (cmdline.has_files)
      {
         opng_printf("!Warning: No files processed.\n");
         cmdline.has_files = 0;
         result = EXIT_FAILURE;
      }
   }
   else if (!cmdline.ver && !cmdline.has_files)
      opng_printf(msg_short_help);

   if (cmdline.has_files)
   {
      process_args(argc, argv);
      if (global.err_count > 0)
      {
         opng_printf("!%u error(s) encountered.\n", global.err_count);
         if (global.fix_count > 0)
            opng_printf("!%u error(s) have been fixed.\n", global.fix_count);
         result = EXIT_FAILURE;
      }
   }

   if (global.logfile != NULL)
      fclose(global.logfile);

   return result;
}
