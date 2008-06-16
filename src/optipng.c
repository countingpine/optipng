/**
 ** OptiPNG: Advanced PNG optimization program.
 ** http://optipng.sourceforge.net/
 **
 ** Copyright (C) 2001-2008 Cosmin Truta.
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
 **    libpng version 1.2.9 or newer (version 1.2.29 is bundled).
 **    pngxtern (version 0.6 is bundled).
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
#include "png.h"
#include "pngx.h"
#include "pngxtern.h"
#include "opngreduc.h"
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
#if 0  /* metadata optimization is not implemented */
   "    -nm\t\t\tno metadata optimization\n"
#endif
   "    -nz\t\t\tno IDAT recompression (also disable reductions)\n"
   "    -fix\t\tenable error recovery\n"
   "    -force\t\tenforce writing of a new output file\n"
   "    -full\t\tproduce a full report on IDAT (might reduce speed)\n"
   "    -preserve\t\tpreserve file attributes if possible\n"
   "    -simulate\t\trun in simulation mode, do not create output files\n"
   "    -snip\t\tcut one image out of multi-image or animation files\n"
#if 0  /* multi-image splitting is not implemented */
   "    -split\t\tsplit multi-image/animation files into separate images\n"
#endif
   "    -out <file>\t\twrite output file to <file>\n"
   "    -dir <directory>\twrite output file(s) to <directory>\n"
   "    -log <file>\t\tlog messages to <file>\n"
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


/** Status flags **/
#define INPUT_IS_PNG_FILE           0x0001
#define INPUT_HAS_PNG_DATASTREAM    0x0002
#define INPUT_HAS_PNG_SIGNATURE     0x0004
#define INPUT_HAS_DIGITAL_SIGNATURE 0x0008
#define INPUT_HAS_MULTIPLE_IMAGES   0x0010
#define INPUT_HAS_NONCONFORMING_PNG 0x0020
#define INPUT_HAS_JUNK              0x0040
#define INPUT_HAS_ERRORS            0x0080
#define OUTPUT_NEEDS_NEW_FILE       0x0100
#define OUTPUT_NEEDS_NEW_IDAT       0x0200
#define OUTPUT_RESERVED             0x7c00
#define OUTPUT_HAS_ERRORS           0x8000U


/** The critical chunks handled by OptiPNG **/
static const png_byte sig_IDAT[4] = { 0x49, 0x44, 0x41, 0x54 };
static const png_byte sig_IEND[4] = { 0x49, 0x45, 0x4e, 0x44 };
/** The ancillary chunks handled by OptiPNG **/
static const png_byte sig_bKGD[4] = { 0x62, 0x4b, 0x47, 0x44 };
static const png_byte sig_hIST[4] = { 0x68, 0x49, 0x53, 0x54 };
static const png_byte sig_sBIT[4] = { 0x73, 0x42, 0x49, 0x54 };
static const png_byte sig_tRNS[4] = { 0x74, 0x52, 0x4e, 0x53 };
static const png_byte sig_dSIG[4] = { 0x64, 0x53, 0x49, 0x47 };
/** The non-standard(!) chunks handled by OptiPNG **/
static const png_byte sig_fdAT[4] = { 0x66, 0x64, 0x41, 0x54 };


/** User exception setup -- see cexcept.h for more info **/
define_exception_type(const char *);
struct exception_context the_exception_context[1];


/** OptiPNG info **/
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
   unsigned int status;
   unsigned long in_file_size, out_file_size;
   long in_datastream_offset;
   png_uint_32 in_idat_size, out_idat_size;
   png_uint_32 best_idat_size, max_idat_size;
   png_uint_32 crt_row, last_row;
   int crt_ipass, last_ipass;
   png_uint_32 reductions;
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int best_compr_level, best_mem_level, best_strategy, best_filter;
   int num_iterations;
} opng_info;

static struct cmdline_struct
{
   unsigned int file_count;
   int help, ver;
   int optim_level;
   int interlace;
   int keep, quiet;
   int nb, nc, np, nz;
   int fix, force, full;
   int preserve, simulate, snip;
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int window_bits;
   char *out_name, *dir_name, *log_name;
} cmdline;

static struct global_struct
{
   FILE *logfile;
   unsigned int err_count, fix_count, snip_count;
} global;


/** Global variables, for quick access and bonus style points **/
static png_structp read_ptr, write_ptr;
static png_infop read_info_ptr, write_info_ptr;
static png_infop read_end_info_ptr, write_end_info_ptr;


/** Internal debugging tool **/
#define OPNG_ENSURE(cond, msg) \
   { if (!(cond)) opng_internal_error(msg); }  /* strong check, no #ifdef's */


/** Bitset utility (find minimum value) **/
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

   if (!cmdline.quiet)
   {
      va_start(arg_ptr, fmt);
      vfprintf(stdout, fmt, arg_ptr);
      va_end(arg_ptr);
   }
   if (global.logfile != NULL)
   {
      va_start(arg_ptr, fmt);
      vfprintf(global.logfile, fmt, arg_ptr);
      va_end(arg_ptr);
      fflush(global.logfile);
   }
}


/** Ratio display w/ logging **/
static void
opng_print_ratio(unsigned long num, unsigned long denom, int fancy)
{
   /* (1) num/denom = 0/0                  ==> print "??%"
    * (2) num/denom = INFINITY             ==> print "INFTY%"
    * (3) 0 <= num/denom < 99.995%         ==> use the percent format "99.99%"
    *     if fancy:
    * (4)    0.995 <= num/denom < 99.995   ==> use the factor format "9.99x"
    * (5)    99.5 <= num/denom < INFINITY  ==> use the factor format "999x"
    *     else:
    * (6)    0.995 <= num/denom < INFINITY ==> use the percent format "999%"
    *     end if
    */

   unsigned long integral, adj_num, adj_denom;

   /* (1,2): num/denom = 0/0 or num/denom = INFINITY */
   if (denom == 0)
   {
      opng_printf(num == 0 ? "??%%" : "INFTY%%");
      return;
   }

   /* (3): 0 <= num/denom < 99.995% */
   /* num/denom < 99.995% <==> denom/(denom-num) < 20000 */
   if (num < denom && denom / (denom - num) < 20000)
   {
      /* Round to nearest 0.01% and multiply the result by 10000%. */
      if (denom <= ULONG_MAX / 10000)
      {
         /* Use the best precision possible. */
         adj_num = num * 10000 + denom / 2;
         adj_denom = denom * 100;
         assert(adj_num / adj_denom < 100);
      }
      else
      {
         /* Reduce the precision to prevent overflow. */
         adj_num = num + denom / 20000;
         if (denom <= ULONG_MAX - 5000)
            denom = (denom + 5000) / 10000;
         else
            denom = ULONG_MAX / 10000;
         assert(denom > 0);
         adj_denom = denom * 100;
         if (adj_num / adj_denom >= 100)
            adj_num = denom * 9999;  /* 100.00% --> 99.99% */
      }
      opng_printf("%lu.%02lu%%",
         adj_num / adj_denom, adj_num % adj_denom / denom);
      return;
   }

   /* Extract the integral out of the fraction for the remaining cases. */
   integral = num / denom;
   num = num % denom;
   /* Round to nearest 0.01 and multiply the result by 100. */
   /* num/denom < 0.995 <==> denom/(denom-num) < 200 */
   if (denom / (denom - num) >= 200)
   {
      /* Round up, use the best precision possible. */
      ++integral;
      adj_num = 0;
      adj_denom = denom;
   }
   else if (denom <= ULONG_MAX / 100)
   {
      /* Also use the best precision possible. */
      adj_num = num * 100 + denom / 2;
      adj_denom = denom;
      assert(adj_num / adj_denom < 100);
   }
   else
   {
      /* Reduce the precision to prevent overflow. */
      adj_num = num + denom / 200;
      if (denom <= ULONG_MAX - 50)
         adj_denom = (denom + 50) / 100;
      else
         adj_denom = ULONG_MAX / 100;
      assert(adj_denom > 0);
      if (adj_num / adj_denom >= 100)
         adj_num = adj_denom * 99;  /* N + 100% --> N + 99% */
   }

   /* (6): 0.995 <= num/denom < INFINITY */
   if (!fancy)
   {
      opng_printf("%lu%02lu%%", integral, adj_num / adj_denom);
      return;
   }

   /* (4): 0.995 <= num/denom < 99.995 */
   if (integral < 100)
   {
      opng_printf("%lu.%02lux", integral, adj_num / adj_denom);
      return;
   }

   /* (5): 99.5 <= num/denom < INFINITY */
   /* Round to nearest integral value, use the best precision possible. */
   if (num % denom >= denom / 2)
      ++integral;
   opng_printf("%lux", integral);
}


/** Size change display w/ logging **/
static void
opng_print_size_difference(unsigned long init_size, unsigned long final_size,
   int show_ratio)
{
   unsigned long difference;
   int sign;

   if (init_size <= final_size)
   {
      sign = 0;
      difference = final_size - init_size;
   }
   else
   {
      sign = 1;
      difference = init_size - final_size;
   }

   if (difference == 0)
   {
      opng_printf("no change");
      return;
   }
   if (difference == 1)
      opng_printf("1 byte");
   else
      opng_printf("%lu bytes", difference);
   if (show_ratio && init_size > 0)
   {
      opng_printf(" = ");
      opng_print_ratio(difference, init_size, 1);
   }
   opng_printf(sign == 0 ? " increase" : " decrease");
}


/** Image info display w/ logging **/
static void
opng_print_image_info(int show_dim, int show_depth, int show_type,
   int show_interlaced)
{
   static const int type_channels[8] = {1, 0, 3, 1, 2, 0, 4, 0};
   int channels, printed;

   printed = 0;
   if (show_dim)
   {
      printed = 1;
      opng_printf("%lux%lu pixels",
         (unsigned long)opng_image.width, (unsigned long)opng_image.height);
   }
   if (show_depth)
   {
      if (printed)
         opng_printf(", ");
      printed = 1;
      channels = type_channels[opng_image.color_type & 7];
      if (channels != 1)
         opng_printf("%dx%d bits/pixel", channels, opng_image.bit_depth);
      else if (opng_image.bit_depth != 1)
         opng_printf("%d bits/pixel", opng_image.bit_depth);
      else
         opng_printf("1 bit/pixel");
   }
   if (show_type)
   {
      if (printed)
         opng_printf(", ");
      printed = 1;
      if (opng_image.color_type & PNG_COLOR_MASK_PALETTE)
      {
         if (opng_image.num_palette == 1)
            opng_printf("1 color");
         else
            opng_printf("%d colors", opng_image.num_palette);
         if (opng_image.num_trans > 0)
            opng_printf(" (%d transparent)", opng_image.num_trans);
         opng_printf(" in palette");
      }
      else
      {
         opng_printf((opng_image.color_type & PNG_COLOR_MASK_COLOR) ?
                     "RGB" : "grayscale");
         if (opng_image.color_type & PNG_COLOR_MASK_ALPHA)
            opng_printf("+alpha");
         else if (opng_image.trans_values_ptr != NULL)
            opng_printf("+transparency");
      }
   }
   if (show_interlaced)
   {
      if (opng_image.interlace_type != PNG_INTERLACE_NONE)
      {
         if (printed)
            opng_printf(", ");
         opng_printf("interlaced");
      }
      /* Displaying "non-interlaced" is not really necessary for PNG images,
       * and is almost meaningless for non-PNG images.
       */
   }
}


/** Progress calculation w/ printing and logging **/
static void
opng_print_progress(void)
{
   static const int progress_factor[7] = {1, 1, 2, 4, 8, 16, 32};
   png_uint_32 height, crt_row, progress;
   int i;

   if (opng_info.crt_row >= opng_info.last_row &&
       opng_info.crt_ipass >= opng_info.last_ipass)
   {
      opng_printf("100%%");
      return;  /* finished */
   }

   if (opng_image.interlace_type == PNG_INTERLACE_ADAM7)
   {
      assert(opng_info.crt_ipass < 7);

      /* This code is accurate only if opng_image.height >= 8 */
      height = opng_image.height;
      crt_row = opng_info.crt_row;
      if (height > PNG_UINT_31_MAX / 64)
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
         opng_print_ratio(progress, height, 0);
      else  /* this may happen only if precision was reduced */
         opng_printf("100%%");  /* ... but it isn't really finished */
   }
   else  /* PNG_INTERLACE_NONE */
   {
      assert(opng_info.crt_ipass == 0);
      opng_print_ratio(opng_info.crt_row, opng_image.height, 0);
   }
}


/** Warning display **/
static void
opng_print_warning(const char *msg)
{
   opng_printf("Warning: %s\n", msg);
}


/** Warning handler **/
static void
opng_warning(png_structp png_ptr, png_const_charp msg)
{
   /* Error in input or output file; processing may continue. */
   /* Recovery requires (re)compression of IDAT. */
   if (png_ptr == read_ptr)
      opng_info.status |= (INPUT_HAS_ERRORS | OUTPUT_NEEDS_NEW_IDAT);
   opng_print_warning(msg);
}


/** Error handler **/
static void
opng_error(png_structp png_ptr, png_const_charp msg)
{
   /* Error in input or output file; processing must stop. */
   /* Recovery requires (re)compression of IDAT. */
   if (png_ptr == read_ptr)
      opng_info.status |= (INPUT_HAS_ERRORS | OUTPUT_NEEDS_NEW_IDAT);
   Throw msg;
}


/** Internal error handler -- this should never get executed! **/
static void
opng_internal_error(png_const_charp msg)
{
   fprintf(stderr, "\n[internal error] %s\n", msg);
   fflush(stderr);
   osys_terminate();
}


/** Chunk handler **/
static void
opng_handle_chunk(png_structp png_ptr, png_bytep chunk_type)
{
   png_byte chunk_name[5];
   int keep;

   if ((chunk_type[0] & 0x20) == 0  /* critical chunk? */
       || memcmp(chunk_type, sig_bKGD, 4) == 0
       || memcmp(chunk_type, sig_hIST, 4) == 0
       || memcmp(chunk_type, sig_sBIT, 4) == 0
       || memcmp(chunk_type, sig_tRNS, 4) == 0)
      return;  /* let libpng handle it */

   /* Everything else is handled as unknown by libpng. */
   keep = PNG_HANDLE_CHUNK_ALWAYS;
   if (memcmp(chunk_type, sig_dSIG, 4) == 0)  /* digital signature? */
      opng_info.status |= INPUT_HAS_DIGITAL_SIGNATURE;
   else if ((chunk_type[1] & 0x20) != 0)  /* non-compliant extension? */
   {
      opng_info.status |= INPUT_HAS_NONCONFORMING_PNG;
      if (memcmp(chunk_type, sig_fdAT, 4) == 0)
         opng_info.status |= INPUT_HAS_MULTIPLE_IMAGES;
      if (cmdline.snip)
      {
         opng_info.status |= INPUT_HAS_JUNK;
         keep = PNG_HANDLE_CHUNK_NEVER;
      }
   }
   memcpy(chunk_name, chunk_type, 4);
   chunk_name[4] = 0;
   if (!png_handle_as_unknown(png_ptr, chunk_name))
      png_set_keep_unknown_chunks(png_ptr, keep, chunk_name, 1);
}


/** Chunk filter **/
static int
opng_allow_chunk(png_bytep chunk_type)
{
   if (memcmp(chunk_type, sig_dSIG, 4) == 0)  /* digital signature? */
      return 0;                               /* ... never allow    */
   if ((chunk_type[1] & 0x20) != 0)  /* non-compliant extension?  */
      return cmdline.snip ? 0 : 1;   /* ... allow if not snipping */
   return 1;  /* allow everything else */
}


/** Initialization for input handler **/
static void
opng_init_read_data(void)
{
   /* Everything inside opng_info is set to zero,
    * and nothing else needs to be done at this moment.
    */
}


/** Initialization for output handler **/
static void
opng_init_write_data(void)
{
   opng_info.out_file_size = 0;
   opng_info.out_idat_size = 0;
   if (opng_info.max_idat_size == 0)
      opng_info.max_idat_size = PNG_UINT_31_MAX;
}


/** Input handler **/
static void
opng_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   FILE *stream = (FILE *)png_get_io_ptr(png_ptr);
   int io_state = pngx_get_io_state(png_ptr);
   int io_state_loc = io_state & PNGX_IO_MASK_LOC;
   png_bytep chunk_sig;

   /* Read the data. */
   if (fread(data, 1, length, stream) != length)
      png_error(png_ptr,
         "Can't read the input file or unexpected end of file");

   if (opng_info.in_file_size == 0)  /* first piece of PNG data */
   {
      OPNG_ENSURE(length == 8, "PNG I/O must start with the first 8 bytes");
      opng_info.in_datastream_offset = ftell(stream) - 8;
      opng_info.status |= INPUT_HAS_PNG_DATASTREAM;
      if (io_state_loc == PNGX_IO_SIGNATURE)
         opng_info.status |= INPUT_HAS_PNG_SIGNATURE;
      if (opng_info.in_datastream_offset == 0)
         opng_info.status |= INPUT_IS_PNG_FILE;
      else if (opng_info.in_datastream_offset < 0)
         png_error(png_ptr,
            "Can't get the file-position indicator in input file");
      opng_info.in_file_size = (unsigned long)opng_info.in_datastream_offset;
   }
   opng_info.in_file_size += length;

   /* Handle the OptiPNG-specific events. */
   OPNG_ENSURE((io_state & PNGX_IO_READING) && (io_state_loc != 0),
      "Incorrect info in png_ptr->io_state");
   if (io_state_loc == PNGX_IO_CHUNK_HDR)
   {
      /* In libpng 1.4.x and later, the chunk length and the chunk name
       * are serialized in a single operation.  This is also ensured by
       * the opngio add-on for libpng 1.2.x and earlier.
       */
      OPNG_ENSURE(length == 8, "Reading chunk header, expecting 8 bytes");
      chunk_sig = data + 4;

      if (memcmp(chunk_sig, sig_IDAT, 4) == 0)
      {
         if (opng_info.in_idat_size == 0)  /* first IDAT */
         {
            /* Allocate the rows here, bypassing libpng.
             * This allows to initialize the contents and perform recovery
             * in case of a premature EOF.
             */
            OPNG_ENSURE(png_ptr == read_ptr, "Incorrect I/O handler setup");
            if (png_get_image_height(read_ptr, read_info_ptr) == 0)
               return;  /* premature IDAT; an error will be triggered later */
            OPNG_ENSURE(png_get_rows(read_ptr, read_info_ptr) == NULL,
               "Image rows have been allocated too early");
            OPNG_ENSURE(pngx_malloc_rows(read_ptr, read_info_ptr, 0) != NULL,
               "Failed allocation of image rows; check the safe allocator");
            png_data_freer(read_ptr, read_info_ptr,
               PNG_USER_WILL_FREE_DATA, PNG_FREE_ROWS);
         }
         else
            opng_info.status |= INPUT_HAS_JUNK;  /* collapse multiple IDAT's */
         opng_info.in_idat_size += png_get_uint_32(data);
      }
      else  /* not IDAT */
         opng_handle_chunk(png_ptr, chunk_sig);
   }
}


/** Output handler **/
static void
opng_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   static int allow_crt_chunk;
   static int crt_chunk_is_idat;
   static long crt_idat_offset;
   static png_uint_32 crt_idat_size, crt_idat_crc;
   FILE *stream = (FILE *)png_get_io_ptr(png_ptr);
   int io_state = pngx_get_io_state(png_ptr);
   int io_state_loc = io_state & PNGX_IO_MASK_LOC;
   png_bytep chunk_sig;
   png_byte buf[4];

   OPNG_ENSURE((io_state & PNGX_IO_WRITING) && (io_state_loc != 0),
      "Incorrect info in png_ptr->io_state");

   /* Handle the OptiPNG-specific events. */
   if (io_state_loc == PNGX_IO_CHUNK_HDR)
   {
      OPNG_ENSURE(length == 8, "Writing chunk header, expecting 8 bytes");
      chunk_sig = data + 4;
      allow_crt_chunk = opng_allow_chunk(chunk_sig);
      if (memcmp(chunk_sig, sig_IDAT, 4) == 0)
      {
         crt_chunk_is_idat = 1;
         opng_info.out_idat_size += png_get_uint_32(data);
         /* Abandon the trial if IDAT is bigger than the maximum allowed. */
         if (stream == NULL)
         {
            if (opng_info.out_idat_size > opng_info.max_idat_size)
               Throw NULL;  /* early interruption, not an error */
         }
      }
      else  /* not IDAT */
         crt_chunk_is_idat = 0;
   }

   /* Exit early if this is only a trial. */
   if (stream == NULL)
      return;

   /* Continue only if the current chunk type is allowed. */
   if (io_state_loc != PNGX_IO_SIGNATURE && !allow_crt_chunk)
      return;

   /* Here comes an elaborate way of writing the data, in which
    * multiple IDATs are collapsed in a single chunk.
    * Normally, the user-supplied I/O routines are not so complicated.
    */
   switch (io_state_loc)
   {
      case PNGX_IO_CHUNK_HDR:
      {
         if (crt_chunk_is_idat)
         {
            if (crt_idat_offset == 0)  /* this is the first IDAT */
            {
               crt_idat_offset = ftell(stream);
               /* Try guessing the concatenated IDAT's length. */
               if (opng_info.best_idat_size > 0)
                  crt_idat_size = opng_info.best_idat_size;
               else
                  crt_idat_size = length;
               png_save_uint_32(data, crt_idat_size);
               /* Start computing the concatenated IDAT's CRC. */
               crt_idat_crc = crc32(0, sig_IDAT, 4);
            }
            else  /* this is not the first IDAT, so do not write its header */
               return;
         }
         else
         {
            if (crt_idat_offset != 0)
            {
               /* This is the header of the first chunk after IDAT. */
               /* IDAT must be finalized. */
               png_save_uint_32(buf, crt_idat_crc);
               if (fwrite(buf, 1, 4, stream) != 4)
                  io_state = 0;  /* error */
               opng_info.out_file_size += 4;
               if (opng_info.out_idat_size != crt_idat_size)
               {
                  /* The IDAT chunk size has not been correctly anticipated.
                   * It must be corrected in a non-streamable way.
                   */
                  OPNG_ENSURE(opng_info.best_idat_size == 0,
                     "Incorrect calculation of IDAT size");
                  OPNG_ENSURE(opng_info.out_idat_size <= PNG_UINT_31_MAX,
                     "Exceedingly large IDAT in output");
                  png_save_uint_32(buf, opng_info.out_idat_size);
                  if (osys_fwrite_at(stream, crt_idat_offset, SEEK_SET,
                      buf, 4) != 4)
                     io_state = 0;  /* error */
               }
               if (io_state == 0)
                  png_error(png_ptr, "Can't finalize IDAT");
               crt_idat_offset = 0;
            }
         }
         break;
      }
      case PNGX_IO_CHUNK_DATA:
      {
         if (crt_chunk_is_idat)
            crt_idat_crc = crc32(crt_idat_crc, data, length);
         break;
      }
      case PNGX_IO_CHUNK_CRC:
      {
         if (crt_chunk_is_idat)
            return;  /* defer writing until the first non-IDAT occurs */
         break;
      }
   }

   /* Write the data. */
   if (fwrite(data, 1, length, stream) != length)
      png_error(png_ptr, "Can't write the output file");
   opng_info.out_file_size += length;
}


/** Progress meter **/
static void
opng_read_write_status(png_structp png_ptr, png_uint_32 row_num, int pass)
{
   if (&png_ptr)  /* dummy, keep compilers happy */
   {
      opng_info.crt_row = row_num;
      opng_info.crt_ipass = pass;
   }
}


/** Image info initialization **/
static void
opng_clear_image_info(void)
{
   png_debug(0, "Clearing opng_image");
   memset(&opng_image, 0, sizeof(opng_image));
}


/** Image info transfer **/
static void
opng_load_image_info(png_structp png_ptr, png_infop info_ptr,
   png_infop end_info_ptr, int load_metadata)
{
   png_debug(0, "Loading opng_image from info struct\n");
   memset(&opng_image, 0, sizeof(opng_image));

   png_get_IHDR(png_ptr, info_ptr,
      &opng_image.width, &opng_image.height, &opng_image.bit_depth,
      &opng_image.color_type, &opng_image.interlace_type,
      &opng_image.compression_type, &opng_image.filter_type);
   opng_image.row_pointers = png_get_rows(png_ptr, info_ptr);
   png_get_PLTE(png_ptr, info_ptr,
      &opng_image.palette, &opng_image.num_palette);
   /* Transparency is not considered metadata, although tRNS is ancillary. */
   if (png_get_tRNS(png_ptr, info_ptr,
      &opng_image.trans, &opng_image.num_trans,
      &opng_image.trans_values_ptr))
   {
      /* Double copying (pointer + value) is necessary here
       * due to an inconsistency in the libpng design.
       */
      if (opng_image.trans_values_ptr != NULL)
      {
         opng_image.trans_values = *opng_image.trans_values_ptr;
         opng_image.trans_values_ptr = &opng_image.trans_values;
      }
   }

   if (!load_metadata)
      return;

   if (png_get_bKGD(png_ptr, info_ptr, &opng_image.background_ptr))
   {
      /* Same problem as in tRNS. */
      opng_image.background = *opng_image.background_ptr;
      opng_image.background_ptr = &opng_image.background;
   }
   png_get_hIST(png_ptr, info_ptr, &opng_image.hist);
   if (png_get_sBIT(png_ptr, info_ptr, &opng_image.sig_bit_ptr))
   {
      /* Same problem as in tRNS. */
      opng_image.sig_bit = *opng_image.sig_bit_ptr;
      opng_image.sig_bit_ptr = &opng_image.sig_bit;
   }
   opng_image.num_unknowns =
      png_get_unknown_chunks(png_ptr, info_ptr, &opng_image.unknowns);

   if (&end_info_ptr == NULL)  /* dummy, end_info_ptr is ignored */
      return;
}


/** Image info transfer **/
static void
opng_store_image_info(png_structp png_ptr, png_infop info_ptr,
   png_infop end_info_ptr, int store_metadata)
{
   png_debug(0, "Storing opng_image to info struct\n");
   OPNG_ENSURE(opng_image.row_pointers != NULL, "No info in opng_image");

   png_set_IHDR(png_ptr, info_ptr,
      opng_image.width, opng_image.height, opng_image.bit_depth,
      opng_image.color_type, opng_image.interlace_type,
      opng_image.compression_type, opng_image.filter_type);
   png_set_rows(write_ptr, write_info_ptr, opng_image.row_pointers);
   if (opng_image.palette != NULL)
      png_set_PLTE(png_ptr, info_ptr,
         opng_image.palette, opng_image.num_palette);
   /* Transparency is not considered metadata, although tRNS is ancillary. */
   if (opng_image.trans != NULL || opng_image.trans_values_ptr != NULL)
      png_set_tRNS(png_ptr, info_ptr,
         opng_image.trans, opng_image.num_trans,
         opng_image.trans_values_ptr);

   if (!store_metadata)
      return;

   if (opng_image.background_ptr != NULL)
      png_set_bKGD(png_ptr, info_ptr, opng_image.background_ptr);
   if (opng_image.hist != NULL)
      png_set_hIST(png_ptr, info_ptr, opng_image.hist);
   if (opng_image.sig_bit_ptr != NULL)
      png_set_sBIT(png_ptr, info_ptr, opng_image.sig_bit_ptr);
   if (opng_image.num_unknowns != 0)
   {
      int i;
      png_set_unknown_chunks(png_ptr, info_ptr,
         opng_image.unknowns, opng_image.num_unknowns);
      /* Is this really necessary? Should it not be implemented in libpng? */
      for (i = 0; i < opng_image.num_unknowns; ++i)
         png_set_unknown_chunk_location(png_ptr, info_ptr,
            i, opng_image.unknowns[i].location);
   }

   if (&end_info_ptr == NULL)  /* dummy, end_info_ptr is ignored */
      return;
}


/** Image info destruction **/
static void
opng_destroy_image_info(void)
{
   png_uint_32 i;
   int j;

   png_debug(0, "Destroying opng_image\n");
   if (opng_image.row_pointers == NULL)
      return;  /* nothing to clean up */

   for (i = 0; i < opng_image.height; ++i)
      osys_free(opng_image.row_pointers[i]);
   osys_free(opng_image.row_pointers);
   osys_free(opng_image.palette);
   osys_free(opng_image.trans);
   osys_free(opng_image.hist);
   for (j = 0; j < opng_image.num_unknowns; ++j)
      osys_free(opng_image.unknowns[j].data);
   osys_free(opng_image.unknowns);
   /* DO NOT deallocate background_ptr, sig_bit_ptr, trans_values_ptr.
    * See the comments regarding double copying inside opng_load_image_info().
    */

   /* Clear the space here and do not worry about double-deallocation issues
    * that might arise later on.
    */
   memset(&opng_image, 0, sizeof(opng_image));
}


/** Image file reading **/
static void
opng_read_file(FILE *infile)
{
   char fmt_name[16];
   int num_img;
   png_uint_32 reductions;
   const char * volatile err_msg;  /* volatile is required by cexcept */

   png_debug(0, "Reading image file\n");
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

      png_debug(0, "Reading input file\n");
      opng_init_read_data();
      pngx_set_read_fn(read_ptr, infile, opng_read_data);
      fmt_name[0] = '\0';
      num_img = pngx_read_image(read_ptr, read_info_ptr,
         fmt_name, sizeof(fmt_name), NULL, 0);
      if (num_img > 1)
         opng_info.status |= INPUT_HAS_MULTIPLE_IMAGES;
      if ((opng_info.status & INPUT_IS_PNG_FILE)
          && (opng_info.status & INPUT_HAS_MULTIPLE_IMAGES))
      {
         /* pngxtern can't distinguish between APNG and proper PNG. */
         strcpy(fmt_name, (opng_info.status & INPUT_HAS_PNG_SIGNATURE) ?
                "APNG" : "APNG datastream");
      }
      OPNG_ENSURE(num_img >= 0, "Format name buffer too small for pngxtern");
      OPNG_ENSURE(fmt_name[0] != 0, "No format name from pngxtern");

      if (opng_info.in_file_size == 0)
      {
         if (fseek(infile, 0, SEEK_END) == 0)
         {
            opng_info.in_file_size = (unsigned long)ftell(infile);
            if (opng_info.in_file_size > LONG_MAX)
               opng_info.in_file_size = 0;
         }
         if (opng_info.in_file_size == 0)
            opng_print_warning("Unable to get the correct file size");
      }

      err_msg = NULL;  /* everything is ok */
   }
   Catch (err_msg)
   {
      /* If the critical info has been loaded, treat all errors as warnings.
       * This enables a more advanced data recovery.
       */
      if (opng_validate_image(read_ptr, read_info_ptr))
      {
         png_warning(read_ptr, err_msg);
         err_msg = NULL;
      }
   }

   Try
   {
      if (err_msg != NULL)
         Throw err_msg;

      /* Display format and image information. */
      if (strcmp(fmt_name, "PNG") != 0)
      {
         opng_printf("Importing %s", fmt_name);
         if (opng_info.status & INPUT_HAS_MULTIPLE_IMAGES)
         {
            if (!(opng_info.status & INPUT_IS_PNG_FILE))
               opng_printf(" (multi-image or animation)");
            if (cmdline.snip)
               opng_printf("; snipping...");
         }
         opng_printf("\n");
      }
      opng_load_image_info(read_ptr, read_info_ptr, read_end_info_ptr, 1);
      opng_print_image_info(1, 1, 1, 1);
      opng_printf("\n");

      /* Choose the applicable image reductions. */
      reductions = OPNG_REDUCE_ALL;
      if (cmdline.nb)
         reductions &= ~OPNG_REDUCE_BIT_DEPTH;
      if (cmdline.nc)
         reductions &= ~OPNG_REDUCE_COLOR_TYPE;
      if (cmdline.np)
         reductions &= ~OPNG_REDUCE_PALETTE_ALL;
      if (opng_info.status & INPUT_HAS_DIGITAL_SIGNATURE)
      {
         /* Do not reduce signed files. */
         reductions = OPNG_REDUCE_NONE;
      }
      if ((opng_info.status & INPUT_IS_PNG_FILE) &&
          (opng_info.status & INPUT_HAS_MULTIPLE_IMAGES) &&
          (reductions != OPNG_REDUCE_NONE) && !cmdline.snip)
      {
         opng_printf(
            "Can't reliably reduce APNG file; disabling reductions.\n"
            "(Rerun " PROGRAM_NAME " with the -snip option "
            "to convert APNG to optimized PNG.)\n");
         reductions = OPNG_REDUCE_NONE;
      }

      /* Try to reduce the image. */
      opng_info.reductions =
         opng_reduce_image(read_ptr, read_info_ptr, reductions);

      /* If the image is reduced, enforce full compression. */
      if (opng_info.reductions != OPNG_REDUCE_NONE)
      {
         opng_info.status |= OUTPUT_NEEDS_NEW_IDAT;
         opng_load_image_info(read_ptr, read_info_ptr, read_end_info_ptr, 1);
         opng_printf("Reducing image to ");
         opng_print_image_info(0, 1, 1, 0);
         opng_printf("\n");
      }

      /* Change the interlace type if required. */
      if (cmdline.interlace >= 0
          && opng_image.interlace_type != cmdline.interlace)
      {
         opng_image.interlace_type = cmdline.interlace;
         /* A change in interlacing requires IDAT recompression. */
         opng_info.status |= OUTPUT_NEEDS_NEW_IDAT;
      }
   }
   Catch (err_msg)
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

   png_debug(0, "Destroying data structs\n");
   /* Leave the data for upcoming processing. */
   png_data_freer(read_ptr, read_info_ptr, PNG_USER_WILL_FREE_DATA,
      PNG_FREE_ALL);
   png_data_freer(read_ptr, read_end_info_ptr, PNG_USER_WILL_FREE_DATA,
      PNG_FREE_ALL);
   png_destroy_read_struct(&read_ptr, &read_info_ptr, &read_end_info_ptr);
}


/** PNG file writing **/
/** If the output file is NULL, PNG encoding is still done,
    but no file is written. **/
static void
opng_write_file(FILE *outfile,
   int compression_level, int memory_level,
   int compression_strategy, int filter)
{
   const char * volatile err_msg;  /* volatile is required by cexcept */

   static int filter_table[FILTER_MAX + 1] =
   {
      PNG_FILTER_NONE, PNG_FILTER_SUB, PNG_FILTER_UP,
      PNG_FILTER_AVG, PNG_FILTER_PAETH, PNG_ALL_FILTERS
   };

   /*png_debug(0, "Writing PNG file\n");*/
   OPNG_ENSURE(
      compression_level >= COMPR_LEVEL_MIN &&
      compression_level <= COMPR_LEVEL_MAX &&
      memory_level >= MEM_LEVEL_MIN &&
      memory_level <= MEM_LEVEL_MAX &&
      compression_strategy >= STRATEGY_MIN &&
      compression_strategy <= STRATEGY_MAX &&
      filter >= FILTER_MIN &&
      filter <= FILTER_MAX,
      "Invalid encoding parameters");

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
      opng_store_image_info(write_ptr, write_info_ptr, write_end_info_ptr,
         (outfile != NULL ? 1 : 0));

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
      opng_init_write_data();
      pngx_set_write_fn(write_ptr, outfile, opng_write_data, NULL);
      png_write_png(write_ptr, write_info_ptr, 0, NULL);

      err_msg = NULL;  /* everything is ok */
   }
   Catch (err_msg)
   {
      /* Set IDAT size to invalid. */
      opng_info.out_idat_size = PNG_UINT_31_MAX + 1;
   }

   png_debug(0, "Destroying data structs\n");
   png_destroy_info_struct(write_ptr, &write_end_info_ptr);
   png_destroy_write_struct(&write_ptr, &write_info_ptr);

   if (err_msg != NULL)
      Throw err_msg;
}


/** PNG file copying **/
static void
opng_copy_file(FILE *infile, FILE *outfile)
{
   volatile png_bytep buf;  /* volatile is required by cexcept */
   const png_uint_32 buf_size_incr = 0x1000;
   png_uint_32 buf_size, length;
   png_byte chunk_hdr[8];
   const char * volatile err_msg;

   png_debug(0, "Copying PNG stream\n");
   assert(infile != NULL && outfile != NULL);

   write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      NULL, opng_error, opng_warning);
   if (write_ptr == NULL)
      Throw "Out of memory";
   opng_init_write_data();
   pngx_set_write_fn(write_ptr, outfile, opng_write_data, NULL);

   Try
   {
      buf = NULL;
      buf_size = 0;

      /* Write the signature in the output file. */
      pngx_write_sig(write_ptr);

      /* Copy all chunks until IEND. */
      /* Error checking is done only at a very basic level. */
      do
      {
         if (fread(chunk_hdr, 8, 1, infile) != 1)  /* length + name */
            Throw "Read error";
         length = png_get_uint_32(chunk_hdr);
         if (length > PNG_UINT_31_MAX)
         {
            if (buf == NULL && length == 0x89504e47)  /* "\x89PNG" */
               continue;  /* skip the signature */
            Throw "Data error";
         }
         if (length + 4 > buf_size)
         {
            png_free(write_ptr, buf);
            buf_size = (((length + 4) + (buf_size_incr - 1))
                        / buf_size_incr) * buf_size_incr;
            buf = (png_bytep)png_malloc(write_ptr, buf_size);
            /* Do not use realloc() here, it's slower. */
         }
         if (fread(buf, length + 4, 1, infile) != 1)  /* data + crc */
            Throw "Read error";
         png_write_chunk(write_ptr, chunk_hdr + 4, buf, length);
      } while (memcmp(chunk_hdr + 4, sig_IEND, 4) != 0);

      err_msg = NULL;  /* everything is ok */
   }
   Catch (err_msg)
   {
   }

   png_free(write_ptr, buf);
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

   /* If the input IDAT exists and is in good condition,
    * the output IDAT should be no larger than the input IDAT.
    */
   if (!(opng_info.status & OUTPUT_NEEDS_NEW_IDAT))
       opng_info.max_idat_size = opng_info.in_idat_size;

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
        bitset_count(strategy_set & ~((1 << Z_HUFFMAN_ONLY) | (1 << Z_RLE)));
   t2 = bitset_count(strategy_set &  ((1 << Z_HUFFMAN_ONLY) | (1 << Z_RLE)));
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

   OPNG_ENSURE(opng_info.num_iterations > 0, "Iterations not initialized");
   if (opng_info.num_iterations == 1
       && (opng_info.status & OUTPUT_NEEDS_NEW_FILE)
       && (opng_info.status & OUTPUT_NEEDS_NEW_IDAT))
   {
      /* If there is one single trial, it is unnecessary to run it. */
      opng_info.best_idat_size   = 0;
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
   opng_info.best_idat_size   = PNG_UINT_31_MAX + 1;
   opng_info.best_compr_level = -1;
   opng_info.best_mem_level   = -1;
   opng_info.best_strategy    = -1;
   opng_info.best_filter      = -1;

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
                           opng_write_file(NULL,
                              compr_level, mem_level, strategy, filter);
                           if (opng_info.out_idat_size > PNG_UINT_31_MAX)
                           {
                              opng_printf("IDAT too big");
                              if (cmdline.ver)  /* verbose */
                              {
                                 opng_printf(" ... abandoned at ");
                                 opng_print_progress();
                              }
                              opng_printf("\n");
                              continue;
                           }
                           opng_printf("IDAT size = %lu\n",
                              (unsigned long)opng_info.out_idat_size);
                           if (opng_info.best_idat_size < opng_info.out_idat_size)
                              continue;
                           if (opng_info.best_idat_size == opng_info.out_idat_size
                               && opng_info.best_strategy >= Z_HUFFMAN_ONLY)
                              continue;  /* it's neither smaller nor faster */
                           opng_info.best_compr_level = compr_level;
                           opng_info.best_mem_level   = mem_level;
                           opng_info.best_strategy    = strategy;
                           opng_info.best_filter      = filter;
                           opng_info.best_idat_size   = opng_info.out_idat_size;
                           opng_info.max_idat_size    = opng_info.out_idat_size;
                        }
                  }
               compr_level_set = saved_level_set;
            }

   OPNG_ENSURE(counter == opng_info.num_iterations,
      "Inconsistent iteration counter");
}


/** Iteration finalization **/
static void
opng_finish_iterations(void)
{
   if (opng_info.best_idat_size < opng_info.in_idat_size)
      opng_info.status |= OUTPUT_NEEDS_NEW_IDAT;
   if (opng_info.status & OUTPUT_NEEDS_NEW_IDAT)
   {
      opng_printf("\nSelecting parameters:\n"
                  "  zc = %d  zm = %d  zs = %d  f = %d",
                  opng_info.best_compr_level, opng_info.best_mem_level,
                  opng_info.best_strategy, opng_info.best_filter);
      if (opng_info.best_idat_size != 0)  /* trials have been run */
         opng_printf("\t\tIDAT size = %lu",
                     (unsigned long)opng_info.best_idat_size);
      opng_printf("\n");
   }
}


/** Image file optimization **/
static void
opng_optimize(const char *infile_name)
{
   static FILE *infile, *outfile;         /* static or volatile is required */
   static const char *outfile_name, *bakfile_name;            /* by cexcept */
   static int new_outfile;
   char name_buf[FILENAME_MAX], tmp_buf[FILENAME_MAX];
   const char * volatile err_msg;

   png_debug1(0, "Optimizing file: %s\n", infile_name);
   memset(&opng_info, 0, sizeof(opng_info));
   if (cmdline.force)
      opng_info.status |= OUTPUT_NEEDS_NEW_IDAT;

   err_msg = NULL;  /* prepare for error handling */

   if ((infile = fopen(infile_name, "rb")) == NULL)
      Throw "Can't open the input file";
   Try
   {
      opng_read_file(infile);
   }
   Catch (err_msg)
   {
      /* assert(err_msg != NULL); */
   }
   fclose(infile);  /* finally */
   if (err_msg != NULL)
      Throw err_msg;  /* rethrow */

   /* Check the digital signature flag. */
   if (opng_info.status & INPUT_HAS_DIGITAL_SIGNATURE)
   {
      opng_printf("Digital signature found in input.");
      if (cmdline.force)
      {
         opng_printf(" Erasing...\n");
         opng_info.status |= OUTPUT_NEEDS_NEW_FILE;
      }
      else
      {
         opng_printf(" Rerun " PROGRAM_NAME " with the -force option.\n");
         Throw "Can't optimize digitally-signed files";
      }
   }

   /* Check the multi-image flag. */
   if (opng_info.status & INPUT_HAS_MULTIPLE_IMAGES)
   {
      if (cmdline.snip)
         ++global.snip_count;
      else if (!(opng_info.status & INPUT_IS_PNG_FILE))
      {
         opng_printf("Conversion to PNG requires snipping. "
                     "Rerun " PROGRAM_NAME " with the -snip option.\n");
         Throw "Incompatible input format";
      }
   }
   if ((opng_info.status & INPUT_HAS_NONCONFORMING_PNG) && cmdline.snip)
      opng_info.status |= OUTPUT_NEEDS_NEW_FILE;

   /* Check the junk flag. */
   if (opng_info.status & INPUT_HAS_JUNK)
      opng_info.status |= OUTPUT_NEEDS_NEW_FILE;

   /* Check the error flag. */
   if (opng_info.status & INPUT_HAS_ERRORS)
   {
      opng_printf("Recoverable errors found in input.");
      if (cmdline.fix)
      {
         opng_printf(" Fixing...\n");
         opng_info.status |= OUTPUT_NEEDS_NEW_FILE;
         ++global.err_count;
         ++global.fix_count;
      }
      else
      {
         opng_printf(" Rerun " PROGRAM_NAME " with the -fix option.\n");
         Throw "Previous error(s) not fixed";
      }
   }

   /* Initialize the output file name. */
   outfile_name = NULL;
   if (!(opng_info.status & INPUT_IS_PNG_FILE))
   {
      if (osys_fname_chext(name_buf, sizeof(name_buf), infile_name,
                           ".png") == NULL)
         Throw "Can't create the output file (name too long)";
      outfile_name = name_buf;
   }
   if (cmdline.out_name != NULL)
      outfile_name = cmdline.out_name;  /* override the old name */
   if (cmdline.dir_name != NULL)
   {
      const char *tmp_name;
      if (outfile_name != NULL)
      {
         strcpy(tmp_buf, outfile_name);
         tmp_name = tmp_buf;
      }
      else
         tmp_name = infile_name;
      if (osys_fname_chdir(name_buf, sizeof(name_buf), tmp_name,
                           cmdline.dir_name) == NULL)
         Throw "Can't create the output file (name too long)";
      outfile_name = name_buf;
   }
   if (outfile_name == NULL)
   {
      outfile_name = infile_name;
      new_outfile = 0;
   }
   else
      new_outfile = (osys_fname_cmp(infile_name, outfile_name) != 0) ? 1 : 0;

   /* Initialize the backup file name. */
   bakfile_name = tmp_buf;
   if (new_outfile)
   {
      if (osys_fname_mkbak(tmp_buf, sizeof(tmp_buf), outfile_name) == NULL)
         bakfile_name = NULL;
   }
   else
   {
      if (osys_fname_mkbak(tmp_buf, sizeof(tmp_buf), infile_name) == NULL)
         bakfile_name = NULL;
   }
   /* Check the name even in simulation mode, to ensure a uniform behavior. */
   if (bakfile_name == NULL)
      Throw "Can't create backup file (name too long)";
   /* Check the backup file before engaging into lengthy trials. */
   if (!cmdline.simulate && osys_ftest(outfile_name, "e") == 0)
   {
      if (new_outfile && !cmdline.keep)
         Throw "The output file exists, try backing it up (use -keep)";
      if (osys_ftest(outfile_name, "fw") != 0 ||
          osys_ftest(bakfile_name, "e") == 0)
         Throw "Can't back up the existing output file";
   }

   /* Display the input IDAT/file sizes. */
   if (opng_info.status & INPUT_HAS_PNG_DATASTREAM)
      opng_printf("Input IDAT size = %lu bytes\n",
                  (unsigned long)opng_info.in_idat_size);
   else
      opng_info.status |= OUTPUT_NEEDS_NEW_IDAT;
   opng_printf("Input file size = %lu bytes\n", opng_info.in_file_size);

   if (cmdline.nz
       && (opng_info.status & INPUT_HAS_PNG_DATASTREAM)
       && (opng_info.status & OUTPUT_NEEDS_NEW_IDAT))
      opng_print_warning(
         "IDAT recompression is necessary; ignoring the -nz option");

   /* Find the best parameters and see if it's worth recompressing. */
   if (!cmdline.nz || (opng_info.status & OUTPUT_NEEDS_NEW_IDAT))
   {
      opng_init_iterations();
      opng_iterate();
      opng_finish_iterations();
   }
   if (opng_info.status & OUTPUT_NEEDS_NEW_IDAT)
      opng_info.status |= OUTPUT_NEEDS_NEW_FILE;
   if (!(opng_info.status & OUTPUT_NEEDS_NEW_FILE))
   {
      opng_printf("\n%s is already optimized.\n", infile_name);
      if (!new_outfile)
         return;
   }

   if (cmdline.simulate)
   {
      if (new_outfile)
         opng_printf("\nSimulation mode: %s not created.\n", outfile_name);
      else
         opng_printf("\nSimulation mode: %s not changed.\n", infile_name);
      return;
   }

   /* Make room for the output file. */
   if (new_outfile)
   {
      opng_printf("\nOutput file: %s\n", outfile_name);
      if (cmdline.dir_name != NULL)
         osys_dir_make(cmdline.dir_name);
      if (osys_ftest(outfile_name, "e") == 0)
         if (rename(outfile_name, bakfile_name) != 0)
            Throw "Can't back up the output file";
   }
   else
   {
      if (rename(infile_name, bakfile_name) != 0)
         Throw "Can't back up the input file";
   }

   outfile = fopen(outfile_name, "wb");
   Try
   {
      if (outfile == NULL)
         Throw "Can't open the output file";
      if (opng_info.status & OUTPUT_NEEDS_NEW_IDAT)
      {
         /* Write a brand new PNG datastream to the output. */
         opng_write_file(outfile,
            opng_info.best_compr_level, opng_info.best_mem_level,
            opng_info.best_strategy, opng_info.best_filter);
      }
      else
      {
         /* Copy the input PNG datastream to the output. */
         infile = osys_fopen_at((new_outfile ? infile_name : bakfile_name),
            "rb", opng_info.in_datastream_offset, SEEK_SET);
         if (infile == NULL)
            Throw "Can't reopen the input file";
         Try
         {
            opng_info.best_idat_size = opng_info.in_idat_size;
            opng_copy_file(infile, outfile);
         }
         Catch (err_msg)
         {
            /* assert(err_msg != NULL); */
         }
         fclose(infile);  /* finally */
         if (err_msg != NULL)
            Throw err_msg;  /* rethrow */
      }
   }
   Catch (err_msg)
   {
      if (outfile != NULL)
         fclose(outfile);
      /* Restore the original input file and rethrow the exception. */
      if (remove(outfile_name) != 0 ||
          rename(bakfile_name, (new_outfile ? outfile_name : infile_name)) != 0)
         opng_print_warning(
            "The original file could not be recovered from the backup");
      Throw err_msg;  /* rethrow */
   }
   /* assert(err_msg == NULL); */
   fclose(outfile);

   /* Preserve file attributes (e.g. ownership, access rights, time stamps)
    * on request, if possible.
    */
   if (cmdline.preserve)
      osys_fattr_copy(outfile_name, infile_name);

   /* Remove the backup file if it is not needed. */
   if (!new_outfile && !cmdline.keep)
   {
      if (remove(bakfile_name) != 0)
         Throw "Can't remove the backup file";
   }

   /* Display the output IDAT/file sizes. */
   opng_printf("\nOutput IDAT size = %lu bytes",
               (unsigned long)opng_info.out_idat_size);
   if (opng_info.status & INPUT_HAS_PNG_DATASTREAM)
   {
      opng_printf(" (");
      opng_print_size_difference(opng_info.in_idat_size,
                                 opng_info.out_idat_size, 0);
      opng_printf(")");
   }
   opng_printf("\nOutput file size = %lu bytes (", opng_info.out_file_size);
   opng_print_size_difference(opng_info.in_file_size,
                              opng_info.out_file_size, 1);
   opng_printf(")\n");
}


/** Command line parsing **/
static void
parse_args(int argc, char *argv[])
{
   char *arg, *dash_arg;
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
      arg = dash_arg = argv[i];
      if (arg[0] != '-' || stop_switch)
      {
         ++cmdline.file_count;
         continue;
      }

      argv[i] = NULL;  /* allow process_args() to skip it */
      do ++arg;  /* multiple dashes are as good as one */
         while (arg[0] == '-');
      if (arg[0] == 0 && dash_arg[0] == '-')  /* -- */
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
      else if (string_prefix_min_cmp("preserve", arg, 2) == 0)
      {
         cmdline.preserve = 1;
      }
      else if (string_prefix_min_cmp("simulate", arg, 2) == 0)
      {
         cmdline.simulate = 1;
      }
      else if (string_prefix_min_cmp("snip", arg, 2) == 0)
      {
         cmdline.snip = 1;
      }
      else if (string_prefix_min_cmp("out", arg, 2) == 0)
      {
         if (cmdline.out_name != NULL)
            Throw "duplicate output file name";
         if (++i >= argc)
            Throw "missing output file name";
         cmdline.out_name = argv[i];
         argv[i] = NULL;  /* allow process_args() to skip it */
      }
      else if (string_prefix_min_cmp("dir", arg, 2) == 0)
      {
         if (cmdline.dir_name != NULL)
            Throw "duplicate output dir name";
         if (++i >= argc)
            Throw "missing output dir name";
         cmdline.dir_name = argv[i];
         argv[i] = NULL;  /* allow process_args() to skip it */
      }
      else if (string_prefix_min_cmp("log", arg, 2) == 0)
      {
         if (cmdline.log_name != NULL)
            Throw "duplicate log file name";
         if (++i >= argc)
            Throw "missing log file name";
         cmdline.log_name = argv[i];
         argv[i] = NULL;  /* allow process_args() to skip it */
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
               {
                  arg = argv[i];
                  argv[i] = NULL;  /* allow process_args() to skip it */
               }
               else
                  arg = "[NULL]";  /* trigger an error later */
            }
         }
         else  /* unrecognized option */
            Throw dash_arg;
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
   }

   /* Finalize. */
   if (cmdline.out_name != NULL)
   {
      if (cmdline.file_count > 1)
         Throw "-out requires a single input file";
      if (cmdline.dir_name != NULL)
         Throw "-out and -dir are mutually exclusive";
   }
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
      if (argv[i] == NULL || argv[i][0] == 0)
         continue;
      opng_clear_image_info();
      Try
      {
         opng_printf("** Processing: %s\n", argv[i]);
         opng_optimize(argv[i]);
         opng_printf("\n");
      }
      Catch (err_msg)
      {
         opng_printf("\nError: %s\n\n", err_msg);
         ++global.err_count;
      }
      opng_destroy_image_info();
   }

   if (cmdline.ver || global.snip_count > 0 || global.err_count > 0)
   {
      opng_printf("** Status report\n");
      opng_printf("%u file(s) have been processed.\n", cmdline.file_count);
      if (global.snip_count > 0)
      {
         opng_printf("%u multi-image file(s) have been snipped.\n",
                     global.snip_count);
      }
      if (global.err_count > 0)
      {
         opng_printf("%u error(s) have been encountered.\n",
                     global.err_count);
         if (global.fix_count > 0)
            opng_printf("%u erroneous file(s) have been fixed.\n",
                        global.fix_count);
      }
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
      fprintf(stderr, "Invalid option: %s\n", err_msg);
      return EXIT_FAILURE;
   }

   if (cmdline.log_name != NULL)
   {
      if (string_suffix_case_cmp(cmdline.log_name, ".log") != 0)
      {
         fprintf(stderr, "To prevent accidental data corruption,"
                         " the log file name must end with \".log\"\n");
         return EXIT_FAILURE;
      }
      if ((global.logfile = fopen(cmdline.log_name, "a")) == NULL)
      {
         fprintf(stderr, "Can't open log file: %s\n", cmdline.log_name);
         return EXIT_FAILURE;
      }
      /* logfile is open, so use opng_printf() from now on. */
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
      if (cmdline.file_count > 0)
      {
         opng_print_warning("No files processed");
         cmdline.file_count = 0;
         result = EXIT_FAILURE;
      }
   }
   else if (!cmdline.ver && cmdline.file_count == 0)
      opng_printf(msg_short_help);

   if (cmdline.file_count > 0)
   {
      process_args(argc, argv);
      if (global.err_count != global.fix_count)
         result = EXIT_FAILURE;
   }

   if (global.logfile != NULL)
      fclose(global.logfile);

   return result;
}
