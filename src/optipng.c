/**
 ** OptiPNG: Advanced PNG optimization program.
 ** http://optipng.sourceforge.net/
 **
 ** Copyright (C) 2001-2006 Cosmin Truta.
 ** The program is distributed under the same licensing and warranty
 ** terms as libpng.
 **
 ** This program functions as follows:
 ** For each input image, it reduces the bit depth, color type and
 ** palette without losing any information; combines several methods
 ** and strategies of compression; and reencodes the IDAT data using
 ** the best method found.  If none of these methods yield a smaller
 ** IDAT, then the original IDAT is preserved.
 ** The output file will have all the IDAT data in a single chunk.
 **
 ** The idea of running multiple trials with different PNG filters
 ** and zlib parameters is inspired from the pngcrush program by
 ** Glenn Randers-Pehrson.
 **
 ** Requirements:
 **    ANSI C or ISO C compiler and library.
 **    POSIX library for enhanced functionality.
 **    zlib version 1.2.1 or newer (version 1.2.3 is bundled).
 **    libpng version 1.2.9 or newer (version 1.2.12 is bundled).
 **    pngxtern (version 0.3 is bundled).
 **    cexcept (version 2.0.0 is bundled).
 **/

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proginfo.h"
#include "opng.h"
#include "pngxtern.h"
#include "cexcept.h"
#include "cbitset.h"
#include "osys.h"
#include "strutil.h"


static const char *msg_intro =
   PROGRAM_NAME " " PROGRAM_VERSION ": " PROGRAM_DESCRIPTION ".\n"
   PROGRAM_COPYRIGHT ".\n\n";

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
   "    Image files of type: PNG, BMP, GIF, PNM or TIFF\n"
   "Basic options:\n"
   "    -h, -help\t\tshow the advanced help\n"
   "    -v\t\t\tverbose mode / show copyright, version and build info\n"
   "    -o  <level>\t\toptimization level (0-7)\t\tdefault 2\n"
   "    -i  <type>\t\tinterlace type (0-1)\t\t\tdefault <input>\n"
   "    -k, -keep\t\tkeep a backup of the modified files\n"
   "    -q, -quiet\t\tquiet mode\n"
   "Examples:\n"
   "    optipng file.png\t\t\t(default speed)\n"
   "    optipng -o5 file.png\t\t(moderately slow)\n"
   "    optipng -o7 file.png\t\t(very slow)\n";

static const char *msg_help =
   "Usage:\n"
   "    optipng [options] files ...\n"
   "Files:\n"
   "    Image files of type: PNG, BMP, GIF, PNM or TIFF\n"
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
   "    -np\t\t\tno palette reduction\n"
#if 0  /* text chunk optimization is not implemented */
   "    -nt\t\t\tno text chunk optimization\n"
#endif
   "    -nz\t\t\tno IDAT recompression (also disable reductions)\n"
   "    -fix\t\tenable error recovery\n"
   "    -force\t\twrite a new output, even if it is larger than the input\n"
   "    -full\t\tproduce a full report on IDAT (might reduce speed)\n"
   "    -log <file>\t\tlog messages to <file>\n"
   "    -preserve\t\tpreserve file attributes if possible\n"
   "    -simulate\t\trun in simulation mode, do not create output files\n"
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
   "Notes:\n"
   "  - The option names are case-insensitive and can be abbreviated.\n"
   "  - Range arguments are cumulative; e.g.\n"
   "    -f0 -f3-5  <=>  -f0,3-5\n"
   "    -zs0 -zs1 -zs2-3  <=>  -zs0,1,2,3  <=>  -zs0-3\n"
   "  - The libpng heuristics consist of:\n"
   "    -o1  <=>  -zc9 -zm8 -zs0 -f0\t\t(if PLTE is present)\n"
   "    -o1  <=>  -zc9 -zm8 -zs1 -f5\t\t(if PLTE is not present)\n"
   "  - The most exhaustive search  -zc1-9 -zm1-9 -zs0-3 -f0-5  (1080 trials)\n"
   "    is offered only as an advanced option, and it is not recomended.\n"
   "Examples:\n"
   "    optipng file.png\t\t\t\t(default speed)\n"
   "    optipng -o5 file.png\t\t\t(moderately slow)\n"
   "    optipng -o7 file.png\t\t\t(very slow)\n"
   "    optipng -i1 -o7 -v -full -sim experiment.png -log experiment.log\n";


/** Program tables, limits and presets **/
#define OPTIM_LEVEL_MIN     0
#define OPTIM_LEVEL_MAX     7
#define OPTIM_LEVEL_DEFAULT 2

/*  "-"  <=>  "MIN-MAX"  */

#define COMPR_LEVEL_MIN     1
#define COMPR_LEVEL_MAX     9
static const char *compr_level_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "9", "9", "9", "9", "-", "-" };
static const char *compr_level_mask = "1-9";

#define MEM_LEVEL_MIN       1
#define MEM_LEVEL_MAX       9
static const char *mem_level_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "8", "8-", "8", "8-", "8", "8-" };
static const char *mem_level_mask = "1-9";

#define STRATEGY_MIN        0
#define STRATEGY_MAX        3
static const char *strategy_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "-", "-", "-", "-", "-", "-" };
static const char *strategy_mask = "0-3";

#define FILTER_MIN          0
#define FILTER_MAX          5
static const char *filter_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "0,5", "0,5", "-", "-", "-", "-" };
static const char *filter_mask = "0-5";


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
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int best_compr_level, best_mem_level, best_strategy, best_filter;
   int num_iterations;
} opng_info;

static struct cmdline_struct
{
   int has_files;
   int help, ver;
   int optim_level;
   int interlace;
   int keep, quiet;
   int nb, nc, np, nz;
   int fix, force, full, log, preserve, simulate;
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
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


/** Internal debugging tool **/
#define OPNG_ENSURE(cond, msg) \
   { if (!(cond)) opng_internal_error(msg); }


/** This should never execute **/
static void
opng_internal_error(const char *msg)
{
   const char *fmt = "[internal error] %s\n";

   fprintf(stderr, fmt, msg);
   if (global.logfile != NULL)
   {
      fprintf(global.logfile, fmt, msg);
      fflush(global.logfile);
   }
   abort();
}


/** Safe memory deallocation **/
static void
opng_free(void *ptr)
{
   /* NOT happy about the standard behavior of free()... */
   if (ptr != NULL)
      free(ptr);
}


/** Bitset utility - find minimum value **/
static int
opng_bitset_min(bitset_t set)
{
   unsigned int i;

   for (i = 0; i < BITSET_SIZE; ++i)
      if (BITSET_GET(set, i))
         return i;
   return -1;  /* empty set */
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

      /* Accumulate the previous passes and the current one */
      progress = 0;
      for (i = 0; i < opng_info.crt_ipass; ++i)
         progress += progress_factor[i] * height;
      progress += progress_factor[i] * crt_row;
      /* Compute the percentage and make sure it's not beyond 100% */
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
   static int crt_chunk_is_idat;
   static long idat_offset;
   static png_uint_32 crt_idat_crc;
   FILE *fp = (FILE *)png_get_io_ptr(png_ptr);
   int io_state = opng_get_io_state(png_ptr);
   int io_state_loc = io_state & OPNG_IO_MASK_LOC;
   png_bytep chunk_sig;

   if ((io_state & OPNG_IO_MASK_OP) == 0 || (io_state & OPNG_IO_MASK_LOC) == 0)
      png_error(png_ptr, "[libpng error] No info in png_ptr->io_state");

   if (length == 0)
      return;
   assert(data != NULL);

   if (io_state & OPNG_IO_READING)
   {
      assert(fp != NULL);
      if (fread(data, length, 1, fp) != 1)
         png_error(png_ptr,
            "Can't read the input file or unexpected end of file");
   }

   /* Update file_size, idat_size, etc. */
   opng_info.file_size += length;
   if (io_state_loc == OPNG_IO_CHUNK_HDR)
   {
      if (length != 8)
         png_error(png_ptr, "[libpng error] Incorrect amount of data for I/O");
      chunk_sig = data + 4;
      if (memcmp(chunk_sig, sig_IDAT, 4) == 0)
      {
         crt_chunk_is_idat = 1;
         ++opng_info.num_idat_chunks;
         opng_info.idat_size += png_get_uint_32(data);
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
      else  /* not IDAT */
      {
         crt_chunk_is_idat = 0;
         if (opng_handle_as_unknown(chunk_sig))
            opng_set_keep_unknown_chunk(png_ptr, chunk_sig);
      }
   }

   /* Don't write anything during trials. */
   if (fp == NULL || !(io_state & OPNG_IO_WRITING))
      return;

   /* Here comes an elaborate way of writing the data, in which
    * multiple IDATs are collapsed in a single chunk.
    * Normally, the user-supplied I/O routines are not so complicated.
    */
   switch (io_state_loc)
   {
      case OPNG_IO_SIGNATURE:
      {
         /* Initialize the static local variables here. */
         idat_offset = 0;
         break;
      }
      case OPNG_IO_CHUNK_HDR:
      {
         if (crt_chunk_is_idat)
         {
            OPNG_ENSURE(opng_info.total_idat_size <= PNG_MAX_UINT,
               "Exceedingly large IDAT size - not handled");
            if (opng_info.num_idat_chunks == 1)  /* the first */
            {
               /* Save the current file position. */
               idat_offset = ftell(fp);
               /* Write the concatenated IDAT's length. */
               png_save_uint_32(data, opng_info.total_idat_size);
               /* Start computing the concatenated IDAT's CRC. */
               crt_idat_crc = crc32(0, sig_IDAT, 4);
            }
            else
            {
               /* No IDAT header unless it's the first IDAT. */
               opng_info.file_size -= 8;
               return;
            }
         }
         else
         {
            /* Sanity check: */
            /* Make sure IDAT is written completely, or not at all. */
            OPNG_ENSURE(opng_info.num_idat_chunks == 0
               || opng_info.idat_size == opng_info.total_idat_size
               || opng_info.total_idat_size == 0,
               "Inconsistent IDAT");
            if (idat_offset != 0)
            {
               /* In some instances, it is necessary to write
                * the IDAT chunk size in a non-streamable way.
                */
               png_byte buf[4];
               size_t num;
               /* Write the IDAT CRC. */
               png_save_uint_32(buf, crt_idat_crc);
               num = fwrite(buf, 1, 4, fp);
               opng_info.file_size += 4;
               /* Seek to the IDAT header and update the length. */
               OPNG_ENSURE(opng_info.total_idat_size == 0, "Got lost...");
               opng_info.total_idat_size = opng_info.idat_size;
               png_save_uint_32(buf, opng_info.idat_size);
               num += osys_fwrite_at(fp, idat_offset, SEEK_SET, buf, 4);
               if (num != 8)
                  png_error(png_ptr, "Can't finalize IDAT");
               idat_offset = 0;  /* not needed anymore */
            }
         }
         break;
      }
      case OPNG_IO_CHUNK_DATA:
      {
         if (crt_chunk_is_idat)
            crt_idat_crc = crc32(crt_idat_crc, data, length);
         break;
      }
      case OPNG_IO_CHUNK_CRC:
      {
         if (crt_chunk_is_idat)
         {
            if (opng_info.idat_size < opng_info.total_idat_size ||
                opng_info.total_idat_size == 0)
            {
               /* No IDAT CRC unless it's the last IDAT. */
               opng_info.file_size -= 4;
               return;
            }
            png_save_uint_32(data, crt_idat_crc);
            idat_offset = 0;  /* not needed anymore */
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
#ifdef PNG_INTERNAL
      png_write_sig(write_ptr);
#else
      /* Some systems that do not allow PNG_INTERNAL
       * require a replacement for png_write_sig().
       */
      {
         static const png_byte png_signature[8] =
            {137, 80, 78, 71, 13, 10, 26, 10};
         extern void /* PRIVATE */
         opng_priv_read_write(png_structp png_ptr,
            png_bytep data, png_size_t length)
         opng_priv_read_write(write_ptr, png_signature, 8);
      }
#endif

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


/** Iteration initialization **/
static void
opng_init_iteration(int cmdline_set, const char *preset, const char *mask,
   int *output_set)
{
   bitset_t set;

   *output_set = cmdline_set;
   if (*output_set == BITSET_EMPTY || cmdline.optim_level >= 0)
   {
      OPNG_ENSURE(bitset_parse(preset, &set) == 0, "Invalid iteration preset");
      *output_set |= set;
   }
   OPNG_ENSURE(bitset_parse(mask, &set) == 0, "Invalid iteration mask");
   *output_set &= set;
}


/** Iteration initialization **/
static void
opng_init_iterations(void)
{
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int preset_index;
   int t1, t2;

   /* Calculate preset_index here, but leave cmdline.optim_level intact,
    * because the effect of "optipng -o2 -z... -f..." is slightly different
    * than the effect of "optipng -z... -f..." (without "-o").
    */
   preset_index = cmdline.optim_level;
   if (preset_index < 0)
      preset_index = OPTIM_LEVEL_DEFAULT;
   else if (preset_index > OPTIM_LEVEL_MAX)
      preset_index = OPTIM_LEVEL_MAX;

   /* Load the iteration sets from the implicit (preset) values,
    * and also from the explicit (user-specified) values.
    */
   opng_init_iteration(cmdline.compr_level_set,
      compr_level_presets[preset_index], compr_level_mask, &compr_level_set);
   opng_init_iteration(cmdline.mem_level_set,
      mem_level_presets[preset_index], mem_level_mask, &mem_level_set);
   opng_init_iteration(cmdline.strategy_set,
      strategy_presets[preset_index], strategy_mask, &strategy_set);
   opng_init_iteration(cmdline.filter_set,
      filter_presets[preset_index], filter_mask, &filter_set);

   /* Replace the empty sets with the libpng's "best guess" heuristics. */
   if (compr_level_set == BITSET_EMPTY)
      BITSET_SET(compr_level_set, Z_BEST_COMPRESSION);  /* -zc9 */
   if (mem_level_set == BITSET_EMPTY)
      BITSET_SET(mem_level_set, 8);
   if (opng_image.bit_depth < 8 || opng_image.palette != NULL)
   {
      if (strategy_set == BITSET_EMPTY)
         BITSET_SET(strategy_set, Z_DEFAULT_STRATEGY);  /* -zs0 */
      if (filter_set == BITSET_EMPTY)
         BITSET_SET(filter_set, 0);  /* -f0 */
   }
   else
   {
      if (strategy_set == BITSET_EMPTY)
         BITSET_SET(strategy_set, Z_FILTERED);  /* -zs1 */
      if (filter_set == BITSET_EMPTY)
         BITSET_SET(filter_set, 5);  /* -f0 */
   }

   /* Store the results into opng_info. */
   opng_info.compr_level_set = compr_level_set;
   opng_info.mem_level_set   = mem_level_set;
   opng_info.strategy_set    = strategy_set;
   opng_info.filter_set      = filter_set;
   t1 = bitset_count(compr_level_set) *
        bitset_count(strategy_set & ~(Z_HUFFMAN_ONLY | Z_RLE));
   t2 = bitset_count(strategy_set & (Z_HUFFMAN_ONLY | Z_RLE));
   opng_info.num_iterations = (t1 + t2) *
        bitset_count(mem_level_set) * bitset_count(filter_set);

   if (opng_info.num_iterations <= 0)
      Throw "Invalid iteration parameters (-zc, -zm, -zs, -f)";
}


/** Iteration **/
static void
opng_iterate(void)
{
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int compr_level, mem_level, strategy, filter;
   int counter;

   OPNG_ENSURE(opng_info.num_iterations > 0, "Iteration not initialized");
   if (opng_info.num_iterations == 1 && !cmdline.simulate
       && (cmdline.force || !opng_info.input_is_png))
   {
      /* If there is one single trial, it is unnecessary to run it. */
      opng_info.best_file_size   = opng_info.best_idat_size = 0;
      opng_info.best_compr_level = opng_bitset_min(opng_info.compr_level_set);
      opng_info.best_mem_level   = opng_bitset_min(opng_info.mem_level_set);
      opng_info.best_strategy    = opng_bitset_min(opng_info.strategy_set);
      opng_info.best_filter      = opng_bitset_min(opng_info.filter_set);
      return;
   }

   /* Prepare for the big iteration. */
   compr_level_set = opng_info.compr_level_set;
   mem_level_set   = opng_info.mem_level_set;
   strategy_set    = opng_info.strategy_set;
   filter_set      = opng_info.filter_set;
   opng_info.best_file_size   = opng_info.best_idat_size = PNG_MAX_UINT + 1;
   opng_info.best_compr_level = opng_info.best_mem_level =
      opng_info.best_strategy = opng_info.best_filter = -1;

   /* Iterate through the "hyper-rectangle" (zc, zm, zs, f). */
   opng_printf("Trying...\n");
   counter = 0;
   for (filter = FILTER_MIN; filter <= FILTER_MAX; ++filter)
      if (BITSET_GET(filter_set, filter))
         for (strategy = STRATEGY_MIN; strategy <= STRATEGY_MAX; ++strategy)
            if (BITSET_GET(strategy_set, strategy))
            {
               /* The compression level has no significance under
                  Z_HUFFMAN_ONLY or Z_RLE. */
               bitset_t saved_level_set = compr_level_set;
               if (strategy == Z_HUFFMAN_ONLY)
               {
                  compr_level_set = BITSET_EMPTY;
                  BITSET_SET(compr_level_set, 1);
               }
               else if (strategy == Z_RLE)
               {
                  compr_level_set = BITSET_EMPTY;
                  BITSET_SET(compr_level_set, 9);  /* use deflate_slow */
               }
               for (compr_level = COMPR_LEVEL_MAX;
                    compr_level >= COMPR_LEVEL_MIN; --compr_level)
                  if (BITSET_GET(compr_level_set, compr_level))
                  {
                     for (mem_level = MEM_LEVEL_MAX;
                          mem_level >= MEM_LEVEL_MIN; --mem_level)
                        if (BITSET_GET(mem_level_set, mem_level))
                        {
                           ++counter;
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
               compr_level_set = saved_level_set;
            }

   OPNG_ENSURE(counter == opng_info.num_iterations,
      "Inconsistent iteration counter");

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
   long out_file_size;
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
      if (string_suffix_case_cmp(filename, ".png") != 0)
      {
         action = create;
         /* Also make sure it's possible to write the output. */
         if (osys_fname_chext(out_filename, sizeof(out_filename), filename,
             ".png") == NULL)
            Throw "Can't create the output file (name too long)";
      }
      else  /* special case: non-PNG file with ".png" extension */
         action = recompress;
   }

   /* Initialize the backup file name. */
   if (action == create)
   {
      if (!cmdline.simulate && (outfile = fopen(out_filename, "rb")) != NULL)
      {
         fclose(outfile);
         if (!cmdline.keep)
            Throw "The output file exists, try backing it up (use -keep)";
      }
      if (osys_fname_mkbak(bak_filename, sizeof(bak_filename),
                           out_filename) == NULL)
         bak_filename[0] = '\0';
   }
   else
   {
      if (osys_fname_mkbak(bak_filename, sizeof(bak_filename),
                           filename) == NULL)
         bak_filename[0] = '\0';
   }
   /* Check the name even in simulation mode, to ensure a uniform behavior. */
   if (bak_filename[0] == '\0')
      Throw "Can't create backup file (name too long)";
   /* Check the backup file before engaging into lengthy trials. */
   if (!cmdline.simulate && (outfile = fopen(bak_filename, "rb")) != NULL)
   {
      fclose(outfile);
      Throw "The backup file name exists, can't prepare the output file";
   }

   /* If the input is invalid but recoverable, enforce full compression. */
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

   /* Find the best parameters and see if it's worth recompressing. */
   if (action == create || action == recompress || !cmdline.nz)
   {
      opng_init_iterations();
      opng_iterate();
      OPNG_ENSURE(opng_info.best_compr_level >= 0 &&
                  opng_info.best_mem_level >= 0   &&
                  opng_info.best_strategy >= 0    &&
                  opng_info.best_filter >= 0,
                  "Incorrect iteration results");
      opng_printf("\nSelecting parameters:\n"
         "  zc = %d  zm = %d  zs = %d  f = %d",
         opng_info.best_compr_level, opng_info.best_mem_level,
         opng_info.best_strategy, opng_info.best_filter);
      if (opng_info.best_idat_size != 0)  /* trials have been run */
         opng_printf("\t\tIDAT size = %lu",
            (unsigned long)opng_info.best_idat_size);
      opng_printf("\n");
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

   out_file_size = 0;
   if (action == join || action == recompress)
   {
      if (cmdline.simulate)
      {
         opng_printf("\nSimulation mode: %s not changed.\n\n", filename);
         return;
      }

      /* Rename the input to a backup name and write the output. */
      if (rename(filename, bak_filename) != 0)
         Throw "Can't back up the input file";
      Try
      {
         if ((outfile = fopen(filename, "wb")) == NULL)
            Throw "Can't open the output file";

         if (action == join)  /* copy input to output, collapsing IDAT */
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
         out_file_size = ftell(outfile);
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

      if (cmdline.simulate)
      {
         opng_printf("\nSimulation mode: %s not created.\n\n", out_filename);
         return;
      }

      /* Create a new output file whose name is in out_filename. */
      assert(out_filename[0] == filename[0]);
      opng_info.total_idat_size = opng_info.best_idat_size;
      if ((outfile = fopen(out_filename, "rb")) != NULL)
      {
         fclose(outfile);
         assert(cmdline.keep);
         /* Rename the input to a backup name and write the output. */
         if (rename(out_filename, bak_filename) != 0)
            Throw "Can't back up the output file";
      }
      if ((outfile = fopen(out_filename, "wb")) == NULL)
         Throw "Can't open the output file";
      Try
      {
         opng_write_png(outfile,
            opng_info.best_compr_level, opng_info.best_mem_level,
            opng_info.best_strategy, opng_info.best_filter);
         out_file_size = ftell(outfile);
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

   opng_printf("\nNew IDAT size = %lu bytes",
      (unsigned long)opng_info.idat_size);
   if (opng_info.input_is_png)
   {
      opng_printf(" (");
      opng_print_size_difference(init_idat_size, opng_info.idat_size, 0);
      opng_printf(")");
   }
   opng_printf("\nNew file size = %lu bytes (",
      (unsigned long)opng_info.file_size);
   opng_print_size_difference(init_file_size, opng_info.file_size, 1);
   opng_printf(")\n\n");
   OPNG_ENSURE(out_file_size == (long)opng_info.file_size,
      "Inconsistent file size");
}


/** Command line parsing **/
static void
parse_args(int argc, char *argv[])
{
   char *arg;
   /* char */ int cmd;
   int stop_switch, i;
   bitset_t set, interlace_set, optim_level_set;

   /* Initialize. */
   memset(&cmdline, 0, sizeof(cmdline));
   cmdline.optim_level = cmdline.interlace = -1;
   interlace_set = optim_level_set = BITSET_EMPTY;

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

      string_lower(arg);  /* options are case-insensitive */
      cmd = 0;

      if (strcmp(arg, "?") == 0 ||
          string_prefix_min_cmp("help", arg, 1) == 0)
      {
         cmdline.help = 1;
      }
      else if (strcmp(arg, "v") == 0)
      {
         cmdline.ver = 1;
      }
      else if (string_prefix_min_cmp("keep", arg, 1) == 0)
      {
         cmdline.keep = 1;
      }
      else if (string_prefix_min_cmp("quiet", arg, 1) == 0)
      {
         cmdline.quiet = 1;
      }
      else if (string_prefix_min_cmp("fix", arg, 2) == 0)
      {
         cmdline.fix = 1;
      }
      else if (string_prefix_min_cmp("force", arg, 2) == 0)
      {
         cmdline.force = 1;
      }
      else if (string_prefix_min_cmp("full", arg, 2) == 0)
      {
         cmdline.full = 1;
      }
      else if (string_prefix_min_cmp("log", arg, 2) == 0)
      {
         cmdline.log = 1;
         if (++i < argc && argv[i][0] != '-')
         {
            if (string_suffix_case_cmp(argv[i], ".log") != 0)
               Throw "To prevent accidental data corruption,"
                     " the log file name must end with \".log\"";
            if ((global.logfile = fopen(argv[i], "a")) == NULL)  /* append */
               Throw "Can't open log file";
            argv[i][0] = 0;  /* allow process_args() to skip it */
         }
         else
            Throw "Missing log file name";
      }
      else if (string_prefix_min_cmp("preserve", arg, 2) == 0)
      {
         cmdline.preserve = 1;
      }
      else if (string_prefix_min_cmp("simulate", arg, 2) == 0)
      {
         cmdline.simulate = 1;
      }
      else if (strcmp(arg, "no") == 0)
      {
         opng_printf("!Warning: Option -no is deprecated. Use -simulate.\n\n");
         cmdline.simulate = 1;
      }
      else if (strcmp(arg, "nb") == 0)
      {
         cmdline.nb = 1;
      }
      else if (strcmp(arg, "nc") == 0)
      {
         cmdline.nc = 1;
      }
      else if (strcmp(arg, "np") == 0)
      {
         cmdline.np = 1;
      }
      else if (strcmp(arg, "nz") == 0)
      {
         cmdline.nz = 1;
      }
      else  /* -i, -o, -zX, -f, or unrecognized option */
      {
         /* Parse the numeric or bitset parameters. */
         cmd = arg[0];
         if (cmd == 'z')
            cmd = toupper((++arg)[0]);
         if ((arg[1] < 'a' || arg[1] > 'z') && cmd != 0 &&
             strchr("fioCMSW", cmd))
         {
            ++arg;
            if (arg[0] == 0)
            {
               if (++i < argc)
                  arg = argv[i];
               else
                  arg = "[NULL]";  /* trigger an error later */
            }
         }
         else  /* unrecognized option */
         {
            argv[i][0] = '-';  /* put back the '-' */
            Throw argv[i];
         }
      }

      /* The numeric/bitset parameter is now in arg. */
      switch (cmd)
      {
         case 0:
            continue;
         case 'f':  /* -f: PNG filter */
         {
            if (bitset_parse(arg, &set) != 0)
               Throw /* invalid */ "filter(s)";
            cmdline.filter_set |= set;
            break;
         }
         case 'i':  /* -i: PNG interlace type */
         {
            if (bitset_parse(arg, &set) != 0 ||
                sscanf(arg, "%d", &cmdline.interlace) < 1 ||
                (cmdline.interlace & ~1) != 0)
               Throw /* invalid */ "interlace type";
            if (bitset_count(interlace_set |= set) != 1)
               Throw "multiple interlace types are not permitted";
            break;
         }
         case 'o':  /* -o: optimization level */
         {
            if (bitset_parse(arg, &set) != 0 ||
                sscanf(arg, "%d", &cmdline.optim_level) < 1)
               Throw /* invalid */ "optimization level";
            if (bitset_count(optim_level_set |= set) != 1)
               Throw "multiple optimization levels are not permitted";
            break;
         }
#if 0 /* not implemented */
         case 'b':  /* -b: bit depth */
         {
            /* cmdline.bit_depth ... */
            break;
         }
         case 'c':  /* -c: color type */
         {
            /* cmdline.color_type ... */
            break;
         }
#endif
         case 'C':  /* -zc: zlib compression level */
         {
            if (bitset_parse(arg, &set) != 0)
               Throw /* invalid */ "compression level(s)";
            cmdline.compr_level_set |= set;
            break;
         }
         case 'M':  /* -zm: zlib memory level */
         {
            if (bitset_parse(arg, &set) != 0)
               Throw /* invalid */ "memory level(s)";
            cmdline.mem_level_set |= set;
            break;
         }
         case 'S':  /* -zs: zlib strategy */
         {
            if (bitset_parse(arg, &set) != 0)
               Throw /* invalid */ "strategy";
            cmdline.strategy_set |= set;
            break;
         }
         case 'W':  /* -zw: zlib window size */
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
            if (cmdline.window_bits > 0 && cmdline.window_bits != wbits)
               Throw "multiple window sizes are not permitted";
            else
               cmdline.window_bits = wbits;
            break;
         }
         default:  /* never get here */
         {
            /* If cmd is none of the above, it should be 0. */
            OPNG_ENSURE(cmd == 0, "Error in command-line parsing");
         }
      }

      arg[0] = 0;  /* allow process_args() to skip it */
   }

   /* Finalize. */
   if (cmdline.optim_level == OPTIM_LEVEL_MIN)
      cmdline.nz = 1;
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
   const char *err_msg;
   int result;

   memset(&global, 0, sizeof(global));
   Try
   {
      parse_args(argc, argv);
   }
   Catch (err_msg)
   {
      if (err_msg[0] < 'A' || err_msg[0] > 'Z')  /* special exception */
         opng_printf("!Invalid option: %s\n", err_msg);
      else
         opng_printf("!%s\n", err_msg);
      return EXIT_FAILURE;
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
