/*
 * opngoptim.c
 * The main OptiPNG optimization engine.
 *
 * Copyright (C) 2001-2010 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the attached LICENSE for more information.
 */

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proginfo.h"
#include "optipng.h"
#include "png.h"
#include "pngx.h"
#include "pngxtern.h"
#include "opngreduc.h"
#include "cexcept.h"
#include "cbitset.h"
#include "osys.h"


/*
 * User exception setup.
 * See cexcept.h for more info
 */
define_exception_type(const char *);
struct exception_context the_exception_context[1];


/*
 * Program tables, limits and presets
 */
#define OPTIM_LEVEL_MIN     0
#define OPTIM_LEVEL_MAX     7
#define OPTIM_LEVEL_DEFAULT 2

#define COMPR_LEVEL_MIN     1
#define COMPR_LEVEL_MAX     9
static const char *compr_level_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "9", "9", "9", "9", "1-9", "1-9" };
static const char *compr_level_mask = "1-9";

#define MEM_LEVEL_MIN       1
#define MEM_LEVEL_MAX       9
static const char *mem_level_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "8", "8-9", "8", "8-9", "8", "8-9" };
static const char *mem_level_mask = "1-9";

#define STRATEGY_MIN        0
#define STRATEGY_MAX        3
static const char *strategy_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "0-", "0-", "0-", "0-", "0-", "0-" };
static const char *strategy_mask = "0-3";

#define FILTER_MIN          0
#define FILTER_MAX          5
static const char *filter_presets[OPTIM_LEVEL_MAX + 1] =
   { "", "", "0,5", "0,5", "0-", "0-", "0-", "0-" };
static const char *filter_mask = "0-5";
static const int filter_table[FILTER_MAX + 1] =
{
   PNG_FILTER_NONE  /* 0 */,
   PNG_FILTER_SUB   /* 1 */,
   PNG_FILTER_UP    /* 2 */,
   PNG_FILTER_AVG   /* 3 */,
   PNG_FILTER_PAETH /* 4 */,
   PNG_ALL_FILTERS  /* 5 */
};


/*
 * Status flags
 */
#define INPUT_IS_PNG_FILE           0x0001
#define INPUT_HAS_PNG_DATASTREAM    0x0002
#define INPUT_HAS_PNG_SIGNATURE     0x0004
#define INPUT_HAS_DIGITAL_SIGNATURE 0x0008
#define INPUT_HAS_MULTIPLE_IMAGES   0x0010
#define INPUT_HAS_APNG              0x0020
#define INPUT_HAS_JUNK              0x0040
#define INPUT_HAS_ERRORS            0x0080
#define OUTPUT_NEEDS_NEW_FILE       0x0100
#define OUTPUT_NEEDS_NEW_IDAT       0x0200
#define OUTPUT_RESERVED             0x7c00
#define OUTPUT_HAS_ERRORS           0x8000U


/*
 * The chunks handled by OptiPNG
 */
static const png_byte sig_PLTE[4] = { 0x50, 0x4c, 0x54, 0x45 };
static const png_byte sig_tRNS[4] = { 0x74, 0x52, 0x4e, 0x53 };
static const png_byte sig_IDAT[4] = { 0x49, 0x44, 0x41, 0x54 };
static const png_byte sig_IEND[4] = { 0x49, 0x45, 0x4e, 0x44 };
static const png_byte sig_bKGD[4] = { 0x62, 0x4b, 0x47, 0x44 };
static const png_byte sig_hIST[4] = { 0x68, 0x49, 0x53, 0x54 };
static const png_byte sig_sBIT[4] = { 0x73, 0x42, 0x49, 0x54 };
static const png_byte sig_dSIG[4] = { 0x64, 0x53, 0x49, 0x47 };
static const png_byte sig_acTL[4] = { 0x61, 0x63, 0x54, 0x4c };
static const png_byte sig_fcTL[4] = { 0x66, 0x63, 0x54, 0x4c };
static const png_byte sig_fdAT[4] = { 0x66, 0x64, 0x41, 0x54 };


/*
 * The optimization engine.
 * Since the engine is not multithreaded, there isn't much to put in here...
 */
static struct opng_engine_struct
{
   int started;
} engine;


/*
 * The optimization process
 */
static struct opng_process_struct
{
   unsigned int status;
   long in_datastream_offset;
   unsigned long in_file_size, out_file_size;
   png_uint_32 in_plte_trns_size, out_plte_trns_size;
   png_uint_32 in_idat_size, out_idat_size;
   png_uint_32 best_idat_size, max_idat_size;
   png_uint_32 reductions;
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int best_compr_level, best_mem_level, best_strategy, best_filter;
   int num_iterations;
} process;


/*
 * The optimization process summary
 */
static struct opng_summary_struct
{
   unsigned int file_count;
   unsigned int err_count;
   unsigned int fix_count;
   unsigned int snip_count;
} summary;


/*
 * The optimized image
 */
static struct opng_image_struct
{
   png_uint_32 width;             /* IHDR */
   png_uint_32 height;
   int bit_depth;
   int color_type;
   int compression_type;
   int filter_type;
   int interlace_type;
   png_bytepp row_pointers;       /* IDAT */
   png_colorp palette;            /* PLTE */
   int num_palette;
   png_color_16p background_ptr;  /* bKGD */
   png_color_16 background;
   png_uint_16p hist;             /* hIST */
   png_color_8p sig_bit_ptr;      /* sBIT */
   png_color_8 sig_bit;
   png_bytep trans_alpha;         /* tRNS */
   int num_trans;
   png_color_16p trans_color_ptr;
   png_color_16 trans_color;
   png_unknown_chunkp unknowns;   /* everything else */
   int num_unknowns;
} image;


/*
 * The user options
 */
static struct opng_options options;


/*
 * The user interface
 */
static void (*usr_printf)(const char *fmt, ...);
static void (*usr_print_cntrl)(int cntrl_code);
static void (*usr_progress)(unsigned long num, unsigned long denom);
static void (*usr_panic)(const char *msg);


/*
 * More global variables, for quick access and bonus style points
 */
static png_structp read_ptr;
static png_infop read_info_ptr;
static png_infop read_end_info_ptr;
static png_structp write_ptr;
static png_infop write_info_ptr;
static png_infop write_end_info_ptr;


/*
 * Internal debugging tool
 */
#define OPNG_ENSURE(cond, msg) \
   { if (!(cond)) usr_panic(msg); }  /* strong check, no #ifdef's */


/*
 * Bitset utility:
 * Find first element in set
 */
static int
opng_bitset_get_first(bitset_t set)
{
   unsigned int i;

   for (i = 0; i < BITSET_SIZE; ++i)
      if (BITSET_GET(set, i))
         return i;
   return -1;  /* empty set */
}


/*
 * Ratio display
 */
static void
opng_print_ratio(unsigned long num, unsigned long denom, int force_percent)
{
   /* (1) num/denom = 0/0                  ==> print "??%"
    * (2) num/denom = INFINITY             ==> print "INFTY%"
    * (3) 0 <= num/denom < 99.995%         ==> use the percent format "99.99%"
    *     if force_percent:
    * (4)    0.995 <= num/denom < INFINITY ==> use the percent format "999%"
    *     else:
    * (5)    0.995 <= num/denom < 99.995   ==> use the factor format "9.99x"
    * (6)    99.5 <= num/denom < INFINITY  ==> use the factor format "999x"
    *     end if
    */

   unsigned long integral, adj_num, adj_denom;

   /* (1,2): num/denom = 0/0 or num/denom = INFINITY */
   if (denom == 0)
   {
      usr_printf(num == 0 ? "??%%" : "INFTY%%");
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
      usr_printf("%lu.%02lu%%",
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

   /* (4): 0.995 <= num/denom < INFINITY */
   if (force_percent)
   {
      usr_printf("%lu%02lu%%", integral, adj_num / adj_denom);
      return;
   }

   /* (5): 0.995 <= num/denom < 99.995 */
   if (integral < 100)
   {
      usr_printf("%lu.%02lux", integral, adj_num / adj_denom);
      return;
   }

   /* (6): 99.5 <= num/denom < INFINITY */
   /* Round to nearest integral value, use the best precision possible. */
   if (num % denom >= denom / 2)
      ++integral;
   usr_printf("%lux", integral);
}


/*
 * Size change display
 */
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
      usr_printf("no change");
      return;
   }
   if (difference == 1)
      usr_printf("1 byte");
   else
      usr_printf("%lu bytes", difference);
   if (show_ratio && init_size > 0)
   {
      usr_printf(" = ");
      opng_print_ratio(difference, init_size, 0);
   }
   usr_printf(sign == 0 ? " increase" : " decrease");
}


/*
 * Image info display
 */
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
      usr_printf("%lux%lu pixels",
         (unsigned long)image.width, (unsigned long)image.height);
   }
   if (show_depth)
   {
      if (printed)
         usr_printf(", ");
      printed = 1;
      channels = type_channels[image.color_type & 7];
      if (channels != 1)
         usr_printf("%dx%d bits/pixel", channels, image.bit_depth);
      else if (image.bit_depth != 1)
         usr_printf("%d bits/pixel", image.bit_depth);
      else
         usr_printf("1 bit/pixel");
   }
   if (show_type)
   {
      if (printed)
         usr_printf(", ");
      printed = 1;
      if (image.color_type & PNG_COLOR_MASK_PALETTE)
      {
         if (image.num_palette == 1)
            usr_printf("1 color");
         else
            usr_printf("%d colors", image.num_palette);
         if (image.num_trans > 0)
            usr_printf(" (%d transparent)", image.num_trans);
         usr_printf(" in palette");
      }
      else
      {
         usr_printf((image.color_type & PNG_COLOR_MASK_COLOR) ?
                    "RGB" : "grayscale");
         if (image.color_type & PNG_COLOR_MASK_ALPHA)
            usr_printf("+alpha");
         else if (image.trans_color_ptr != NULL)
            usr_printf("+transparency");
      }
   }
   if (show_interlaced)
   {
      if (image.interlace_type != PNG_INTERLACE_NONE)
      {
         if (printed)
            usr_printf(", ");
         usr_printf("interlaced");
      }
      /* Displaying "non-interlaced" is not really necessary for PNG images,
       * and is almost meaningless for non-PNG images.
       */
   }
}


/*
 * Warning display
 */
static void
opng_print_warning(const char *msg)
{
   usr_print_cntrl('\v');  /* VT: new paragraph */
   usr_printf("Warning: %s\n", msg);
}


/*
 * Error display
 */
static void
opng_print_error(const char *msg)
{
   usr_print_cntrl('\v');  /* VT: new paragraph */
   usr_printf("Error: %s\n", msg);
}


/*
 * Warning handler
 */
static void
opng_warning(png_structp png_ptr, png_const_charp msg)
{
   /* Error in input or output file; processing may continue. */
   /* Recovery requires (re)compression of IDAT. */
   if (png_ptr == read_ptr)
      process.status |= (INPUT_HAS_ERRORS | OUTPUT_NEEDS_NEW_IDAT);
   opng_print_warning(msg);
}


/*
 * Error handler
 */
static void
opng_error(png_structp png_ptr, png_const_charp msg)
{
   /* Error in input or output file; processing must stop. */
   /* Recovery requires (re)compression of IDAT. */
   if (png_ptr == read_ptr)
      process.status |= (INPUT_HAS_ERRORS | OUTPUT_NEEDS_NEW_IDAT);
   Throw msg;
}


/*
 * Memory deallocator
 */
static void
opng_free(void *ptr)
{
   /* This deallocator must be compatible with libpng's memory allocation
    * routines, png_malloc() and png_free().
    * If those routines change, this one must be changed accordingly.
    */
   free(ptr);
}


/*
 * Chunk categorization
 */
static int
opng_is_critical_chunk(png_bytep chunk_type)
{
   if ((chunk_type[0] & 0x20) == 0)
      return 1;
   /* In strict terms of the PNG specification, tRNS is ancillary.
    * However, the tRNS data defines the actual alpha samples, which
    * is critical information.  OptiPNG cannot operate losslessly
    * unless it treats tRNS as a critical chunk.
    * (Image animations constitute yet another area of applications
    * in which transparency is critical, and cannot be ignored unless
    * explicitly stated on a case-by-case basis.)
    */
   if (memcmp(chunk_type, sig_tRNS, 4) == 0)
      return 1;
   return 0;
}


/*
 * Chunk categorization
 */
static int
opng_is_apng_chunk(png_bytep chunk_type)
{
   if (memcmp(chunk_type, sig_acTL, 4) == 0 ||
       memcmp(chunk_type, sig_fcTL, 4) == 0 ||
       memcmp(chunk_type, sig_fdAT, 4) == 0)
      return 1;
   return 0;
}


/*
 * Chunk filter
 */
static int
opng_allow_chunk(png_bytep chunk_type)
{
   /* Always block the digital signature chunks. */
   if (memcmp(chunk_type, sig_dSIG, 4) == 0)
      return 0;
   /* Block the APNG chunks when snipping. */
   if (options.snip && opng_is_apng_chunk(chunk_type))
      return 0;
   /* Allow everything else. */
   return 1;
}


/*
 * Chunk handler
 */
static void
opng_handle_chunk(png_structp png_ptr, png_bytep chunk_type)
{
   png_byte chunk_name[5];
   int keep;

   if (opng_is_critical_chunk(chunk_type) ||
       memcmp(chunk_type, sig_bKGD, 4) == 0 ||
       memcmp(chunk_type, sig_hIST, 4) == 0 ||
       memcmp(chunk_type, sig_sBIT, 4) == 0)
      return;  /* let libpng handle it */

   /* Everything else is handled as unknown by libpng. */
   keep = PNG_HANDLE_CHUNK_ALWAYS;
   if (memcmp(chunk_type, sig_dSIG, 4) == 0)  /* digital signature? */
      process.status |= INPUT_HAS_DIGITAL_SIGNATURE;
   else if (opng_is_apng_chunk(chunk_type))  /* APNG? */
   {
      process.status |= INPUT_HAS_APNG;
      if (memcmp(chunk_type, sig_fdAT, 4) == 0)
         process.status |= INPUT_HAS_MULTIPLE_IMAGES;
      if (options.snip)
      {
         process.status |= INPUT_HAS_JUNK;
         keep = PNG_HANDLE_CHUNK_NEVER;
      }
   }
   memcpy(chunk_name, chunk_type, 4);
   chunk_name[4] = 0;
   if (!png_handle_as_unknown(png_ptr, chunk_name))
      png_set_keep_unknown_chunks(png_ptr, keep, chunk_name, 1);
}


/*
 * Initialization for input handler
 */
static void
opng_init_read_data(void)
{
   /* The relevant process data members are set to zero,
    * and nothing else needs to be done at this moment.
    */
}


/*
 * Initialization for output handler
 */
static void
opng_init_write_data(void)
{
   process.out_file_size = 0;
   process.out_plte_trns_size = 0;
   process.out_idat_size = 0;
}


/*
 * Input handler
 */
static void
opng_read_data(png_structp png_ptr, png_bytep data, size_t length)
{
   FILE *stream = (FILE *)png_get_io_ptr(png_ptr);
   int io_state = pngx_get_io_state(png_ptr);
   int io_state_loc = io_state & PNGX_IO_MASK_LOC;
   png_bytep chunk_sig;

   /* Read the data. */
   if (fread(data, 1, length, stream) != length)
      png_error(png_ptr,
         "Can't read the input file or unexpected end of file");

   if (process.in_file_size == 0)  /* first piece of PNG data */
   {
      OPNG_ENSURE(length == 8, "PNG I/O must start with the first 8 bytes");
      process.in_datastream_offset = ftell(stream) - 8;
      process.status |= INPUT_HAS_PNG_DATASTREAM;
      if (io_state_loc == PNGX_IO_SIGNATURE)
         process.status |= INPUT_HAS_PNG_SIGNATURE;
      if (process.in_datastream_offset == 0)
         process.status |= INPUT_IS_PNG_FILE;
      else if (process.in_datastream_offset < 0)
         png_error(png_ptr,
            "Can't get the file-position indicator in input file");
      process.in_file_size = (unsigned long)process.in_datastream_offset;
   }
   process.in_file_size += length;

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
         if (process.in_idat_size == 0)  /* first IDAT */
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
         {
            /* The split IDAT overhead is removed by joining IDATs. */
            process.status |= INPUT_HAS_JUNK;
         }
         process.in_idat_size += png_get_uint_32(data);
      }
      else if (memcmp(chunk_sig, sig_PLTE, 4) == 0 ||
               memcmp(chunk_sig, sig_tRNS, 4) == 0)
      {
         /* Add the chunk overhead (header + CRC) besides the data size. */
         process.in_plte_trns_size += png_get_uint_32(data) + 12;
      }
      else
         opng_handle_chunk(png_ptr, chunk_sig);
   }
}


/*
 * Output handler
 */
static void
opng_write_data(png_structp png_ptr, png_bytep data, size_t length)
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
         process.out_idat_size += png_get_uint_32(data);
         /* Abandon the trial if IDAT is bigger than the maximum allowed. */
         if (stream == NULL)
         {
            if (process.out_idat_size > process.max_idat_size)
               Throw NULL;  /* early interruption, not an error */
         }
      }
      else  /* not IDAT */
      {
         crt_chunk_is_idat = 0;
         if (memcmp(chunk_sig, sig_PLTE, 4) == 0 ||
             memcmp(chunk_sig, sig_tRNS, 4) == 0)
         {
            /* Add the chunk overhead (header + CRC) besides the data size. */
            process.out_plte_trns_size += png_get_uint_32(data) + 12;
         }
      }
   }

   /* Exit early if this is only a trial. */
   if (stream == NULL)
      return;

   /* Continue only if the current chunk type is allowed. */
   if (io_state_loc != PNGX_IO_SIGNATURE && !allow_crt_chunk)
      return;

   /* Here comes an elaborate way of writing the data, in which all IDATs
    * are joined into a single chunk.
    * Normally, the user-supplied I/O routines are not so complicated.
    */
   switch (io_state_loc)
   {
      case PNGX_IO_CHUNK_HDR:
      {
         if (crt_chunk_is_idat)
         {
            if (crt_idat_offset == 0)
            {
               /* This is the header of the first IDAT. */
               crt_idat_offset = ftell(stream);
               /* Try guessing the size of the final (joined) IDAT. */
               if (process.best_idat_size > 0)
               {
                  /* The guess is expected to be right. */
                  crt_idat_size = process.best_idat_size;
               }
               else
               {
                  /* The guess could be wrong.
                   * The size of the final IDAT will be revised if necessary.
                   */
                  crt_idat_size = length;
               }
               png_save_uint_32(data, crt_idat_size);
               /* Start computing the CRC of the final IDAT. */
               crt_idat_crc = crc32(0, sig_IDAT, 4);
            }
            else
            {
               /* This is not the first IDAT.  Do not write its header. */
               return;
            }
         }
         else
         {
            if (crt_idat_offset != 0)
            {
               /* This is the header of the first chunk after IDAT.
                * Finalize IDAT before resuming the normal operation.
                */
               png_save_uint_32(buf, crt_idat_crc);
               if (fwrite(buf, 1, 4, stream) != 4)
                  io_state = 0;  /* error */
               process.out_file_size += 4;
               if (process.out_idat_size != crt_idat_size)
               {
                  /* The IDAT size has not been guessed correctly.
                   * It must be updated in a non-streamable way.
                   */
                  OPNG_ENSURE(process.best_idat_size == 0,
                     "Wrong guess of the output IDAT size");
                  OPNG_ENSURE(process.out_idat_size <= PNG_UINT_31_MAX,
                     "Exceedingly large IDAT in output");
                  png_save_uint_32(buf, process.out_idat_size);
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
   process.out_file_size += length;
}


/*
 * Image info initialization
 */
static void
opng_clear_image_info(void)
{
   memset(&image, 0, sizeof(image));
}


/*
 * Image info transfer
 */
static void
opng_load_image_info(png_structp png_ptr, png_infop info_ptr,
   png_infop end_info_ptr, int load_metadata)
{
   memset(&image, 0, sizeof(image));

   png_get_IHDR(png_ptr, info_ptr,
      &image.width, &image.height, &image.bit_depth, &image.color_type,
      &image.interlace_type, &image.compression_type, &image.filter_type);
   image.row_pointers = png_get_rows(png_ptr, info_ptr);
   png_get_PLTE(png_ptr, info_ptr, &image.palette, &image.num_palette);
   /* Transparency is not considered metadata, although tRNS is ancillary.
    * See the comment in opng_is_critical_chunk() above.
    */
   if (png_get_tRNS(png_ptr, info_ptr,
      &image.trans_alpha, &image.num_trans, &image.trans_color_ptr))
   {
      /* Double copying (pointer + value) is necessary here
       * due to an inconsistency in the libpng design.
       */
      if (image.trans_color_ptr != NULL)
      {
         image.trans_color = *image.trans_color_ptr;
         image.trans_color_ptr = &image.trans_color;
      }
   }

   if (!load_metadata)
      return;

   if (png_get_bKGD(png_ptr, info_ptr, &image.background_ptr))
   {
      /* Same problem as in tRNS. */
      image.background = *image.background_ptr;
      image.background_ptr = &image.background;
   }
   png_get_hIST(png_ptr, info_ptr, &image.hist);
   if (png_get_sBIT(png_ptr, info_ptr, &image.sig_bit_ptr))
   {
      /* Same problem as in tRNS. */
      image.sig_bit = *image.sig_bit_ptr;
      image.sig_bit_ptr = &image.sig_bit;
   }
   image.num_unknowns =
      png_get_unknown_chunks(png_ptr, info_ptr, &image.unknowns);

   if (end_info_ptr == NULL)  /* dummy, keep compilers happy */
      return;
}


/*
 * Image info transfer
 */
static void
opng_store_image_info(png_structp png_ptr, png_infop info_ptr,
   png_infop end_info_ptr, int store_metadata)
{
   OPNG_ENSURE(image.row_pointers != NULL, "No info in image");

   png_set_IHDR(png_ptr, info_ptr,
      image.width, image.height, image.bit_depth, image.color_type,
      image.interlace_type, image.compression_type, image.filter_type);
   png_set_rows(write_ptr, write_info_ptr, image.row_pointers);
   if (image.palette != NULL)
      png_set_PLTE(png_ptr, info_ptr, image.palette, image.num_palette);
   /* Transparency is not considered metadata, although tRNS is ancillary.
    * See the comment in opng_is_critical_chunk() above.
    */
   if (image.trans_alpha != NULL || image.trans_color_ptr != NULL)
      png_set_tRNS(png_ptr, info_ptr,
         image.trans_alpha, image.num_trans, image.trans_color_ptr);

   if (!store_metadata)
      return;

   if (image.background_ptr != NULL)
      png_set_bKGD(png_ptr, info_ptr, image.background_ptr);
   if (image.hist != NULL)
      png_set_hIST(png_ptr, info_ptr, image.hist);
   if (image.sig_bit_ptr != NULL)
      png_set_sBIT(png_ptr, info_ptr, image.sig_bit_ptr);
   if (image.num_unknowns != 0)
   {
      int i;
      png_set_unknown_chunks(png_ptr, info_ptr,
         image.unknowns, image.num_unknowns);
      /* Is this really necessary? Should it not be implemented in libpng? */
      for (i = 0; i < image.num_unknowns; ++i)
         png_set_unknown_chunk_location(png_ptr, info_ptr,
            i, image.unknowns[i].location);
   }

   if (end_info_ptr == NULL)  /* dummy, keep compilers happy */
      return;
}


/*
 * Image info destruction
 */
static void
opng_destroy_image_info(void)
{
   png_uint_32 i;
   int j;

   if (image.row_pointers == NULL)
      return;  /* nothing to clean up */

   for (i = 0; i < image.height; ++i)
      opng_free(image.row_pointers[i]);
   opng_free(image.row_pointers);
   opng_free(image.palette);
   opng_free(image.trans_alpha);
   opng_free(image.hist);
   for (j = 0; j < image.num_unknowns; ++j)
      opng_free(image.unknowns[j].data);
   opng_free(image.unknowns);
   /* DO NOT deallocate background_ptr, sig_bit_ptr, trans_color_ptr.
    * See the comments regarding double copying inside opng_load_image_info().
    */

   /* Clear the space here and do not worry about double-deallocation issues
    * that might arise later on.
    */
   memset(&image, 0, sizeof(image));
}


/*
 * Image file reading
 */
static void
opng_read_file(FILE *infile)
{
   const char *fmt_name;
   int num_img;
   png_uint_32 reductions;
   const char * volatile err_msg;  /* volatile is required by cexcept */

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
      if (read_end_info_ptr == NULL)
         Throw "Out of memory";

      png_set_keep_unknown_chunks(read_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);

      /* Read the input image file. */
      opng_init_read_data();
      pngx_set_read_fn(read_ptr, infile, opng_read_data);
      fmt_name = NULL;
      num_img = pngx_read_image(read_ptr, read_info_ptr, &fmt_name, NULL);
      if (num_img <= 0)
         Throw "Unrecognized image file format";
      if (num_img > 1)
         process.status |= INPUT_HAS_MULTIPLE_IMAGES;
      if ((process.status & INPUT_IS_PNG_FILE) &&
          (process.status & INPUT_HAS_MULTIPLE_IMAGES))
      {
         /* pngxtern can't distinguish between APNG and proper PNG. */
         fmt_name = (process.status & INPUT_HAS_PNG_SIGNATURE) ?
                    "APNG" : "APNG datastream";
      }
      OPNG_ENSURE(fmt_name != NULL, "No format name from pngxtern");

      if (process.in_file_size == 0)
      {
         if (fseek(infile, 0, SEEK_END) == 0)
         {
            process.in_file_size = (unsigned long)ftell(infile);
            if (process.in_file_size > LONG_MAX)
               process.in_file_size = 0;
         }
         if (process.in_file_size == 0)
            opng_print_warning("Can't get the correct file size");
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
         usr_printf("Importing %s", fmt_name);
         if (process.status & INPUT_HAS_MULTIPLE_IMAGES)
         {
            if (!(process.status & INPUT_IS_PNG_FILE))
               usr_printf(" (multi-image or animation)");
            if (options.snip)
               usr_printf("; snipping...");
         }
         usr_printf("\n");
      }
      opng_load_image_info(read_ptr, read_info_ptr, read_end_info_ptr, 1);
      opng_print_image_info(1, 1, 1, 1);
      usr_printf("\n");

      /* Choose the applicable image reductions. */
      reductions = OPNG_REDUCE_ALL;
      if (options.nb)
         reductions &= ~OPNG_REDUCE_BIT_DEPTH;
      if (options.nc)
         reductions &= ~OPNG_REDUCE_COLOR_TYPE;
      if (options.np)
         reductions &= ~OPNG_REDUCE_PALETTE_ALL;
      if (options.nz && (process.status & INPUT_HAS_PNG_DATASTREAM))
      {
         /* Do not reduce files with PNG datastreams under -nz. */
         reductions = OPNG_REDUCE_NONE;
      }
      if (process.status & INPUT_HAS_DIGITAL_SIGNATURE)
      {
         /* Do not reduce signed files. */
         reductions = OPNG_REDUCE_NONE;
      }
      if ((process.status & INPUT_IS_PNG_FILE) &&
          (process.status & INPUT_HAS_MULTIPLE_IMAGES) &&
          (reductions != OPNG_REDUCE_NONE) && !options.snip)
      {
         usr_printf(
            "Can't reliably reduce APNG file; disabling reductions.\n"
            "(Did you want to -snip and optimize the first frame?)\n");
         reductions = OPNG_REDUCE_NONE;
      }

      /* Try to reduce the image. */
      process.reductions =
         opng_reduce_image(read_ptr, read_info_ptr, reductions);

      /* If the image is reduced, enforce full compression. */
      if (process.reductions != OPNG_REDUCE_NONE)
      {
         opng_load_image_info(read_ptr, read_info_ptr, read_end_info_ptr, 1);
         usr_printf("Reducing image to ");
         opng_print_image_info(0, 1, 1, 0);
         usr_printf("\n");
      }

      /* Change the interlace type if required. */
      if (options.interlace >= 0 &&
          image.interlace_type != options.interlace)
      {
         image.interlace_type = options.interlace;
         /* A change in interlacing requires IDAT recoding. */
         process.status |= OUTPUT_NEEDS_NEW_IDAT;
      }
   }
   Catch (err_msg)
   {
      /* Do the cleanup, then rethrow the exception. */
      png_data_freer(read_ptr, read_info_ptr,
         PNG_DESTROY_WILL_FREE_DATA, PNG_FREE_ALL);
      png_data_freer(read_ptr, read_end_info_ptr,
         PNG_DESTROY_WILL_FREE_DATA, PNG_FREE_ALL);
      png_destroy_read_struct(&read_ptr,
         &read_info_ptr, &read_end_info_ptr);
      Throw err_msg;
   }

   /* Destroy the libpng structures, but leave the enclosed data intact
    * to allow further processing.
    */
   png_data_freer(read_ptr, read_info_ptr,
      PNG_USER_WILL_FREE_DATA, PNG_FREE_ALL);
   png_data_freer(read_ptr, read_end_info_ptr,
      PNG_USER_WILL_FREE_DATA, PNG_FREE_ALL);
   png_destroy_read_struct(&read_ptr, &read_info_ptr, &read_end_info_ptr);
}


/*
 * PNG file writing
 *
 * If the output file is NULL, PNG encoding is still done,
 * but no file is written.
 */
static void
opng_write_file(FILE *outfile,
   int compression_level, int memory_level,
   int compression_strategy, int filter)
{
   const char * volatile err_msg;  /* volatile is required by cexcept */

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
      if (write_end_info_ptr == NULL)
         Throw "Out of memory";

      png_set_compression_level(write_ptr, compression_level);
      png_set_compression_mem_level(write_ptr, memory_level);
      png_set_compression_strategy(write_ptr, compression_strategy);
      png_set_filter(write_ptr, PNG_FILTER_TYPE_BASE, filter_table[filter]);
      if (compression_strategy != Z_HUFFMAN_ONLY &&
          compression_strategy != Z_RLE)
      {
         if (options.window_bits > 0)
            png_set_compression_window_bits(write_ptr, options.window_bits);
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

      /* Write the PNG stream. */
      opng_init_write_data();
      pngx_set_write_fn(write_ptr, outfile, opng_write_data, NULL);
      png_write_png(write_ptr, write_info_ptr, 0, NULL);

      err_msg = NULL;  /* everything is ok */
   }
   Catch (err_msg)
   {
      /* Set IDAT size to invalid. */
      process.out_idat_size = PNG_UINT_31_MAX + 1;
   }

   /* Destroy the libpng structures. */
   png_destroy_info_struct(write_ptr, &write_end_info_ptr);
   png_destroy_write_struct(&write_ptr, &write_info_ptr);

   if (err_msg != NULL)
      Throw err_msg;
}


/*
 * PNG file copying
 */
static void
opng_copy_file(FILE *infile, FILE *outfile)
{
   volatile png_bytep buf;  /* volatile is required by cexcept */
   const png_uint_32 buf_size_incr = 0x1000;
   png_uint_32 buf_size, length;
   png_byte chunk_hdr[8];
   const char * volatile err_msg;

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
            if (buf == NULL && length == 0x89504e47UL)  /* "\x89PNG" */
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


/*
 * Iteration initialization
 */
static void
opng_init_iteration(bitset_t cmdline_set, const char *preset, const char *mask,
   bitset_t *output_set)
{
   bitset_t tmp_set, mask_set;

   OPNG_ENSURE(bitset_parse(mask, &mask_set) == 0, "Invalid iteration mask");
   tmp_set = cmdline_set & mask_set;
   if (cmdline_set != BITSET_EMPTY && tmp_set == BITSET_EMPTY)
      Throw "Iteration parameters (-zc, -zm, -zs, -f) out of range";
   *output_set = tmp_set;

   if (*output_set == BITSET_EMPTY || options.optim_level >= 0)
   {
      OPNG_ENSURE(bitset_parse(preset, &tmp_set) == 0, "Invalid iteration preset");
      *output_set |= tmp_set & mask_set;
   }
}


/*
 * Iteration initialization
 */
static void
opng_init_iterations(void)
{
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int preset_index;
   int t1, t2;

   /* Set the IDAT size limit.  The trials that pass this limit will be
    * abandoned, as there will be no need to wait until their completion.
    * This limit may further decrease as iterations go on.
    */
   if ((process.status & OUTPUT_NEEDS_NEW_IDAT) || options.full)
      process.max_idat_size = PNG_UINT_31_MAX;
   else
   {
      OPNG_ENSURE(process.in_idat_size > 0, "No IDAT in input");
      /* Add the input PLTE and tRNS sizes to the initial max IDAT size,
       * to account for the changes that may occur during reduction.
       * This incurs a negligible overhead on processing only: the final
       * IDAT size will not be affected, because a precise check will be
       * performed at the end, inside opng_finish_iterations().
       */
      process.max_idat_size = process.in_idat_size + process.in_plte_trns_size;
   }

   /* Get preset_index from options.optim_level, but leave the latter intact,
    * because the effect of "optipng -o2 -z... -f..." is slightly different
    * than the effect of "optipng -z... -f..." (without "-o").
    */
   preset_index = options.optim_level;
   if (preset_index < 0)
      preset_index = OPTIM_LEVEL_DEFAULT;
   else if (preset_index > OPTIM_LEVEL_MAX)
      preset_index = OPTIM_LEVEL_MAX;

   /* Load the iteration sets from the implicit (preset) values,
    * and also from the explicit (user-specified) values.
    */
   opng_init_iteration(options.compr_level_set,
      compr_level_presets[preset_index], compr_level_mask, &compr_level_set);
   opng_init_iteration(options.mem_level_set,
      mem_level_presets[preset_index], mem_level_mask, &mem_level_set);
   opng_init_iteration(options.strategy_set,
      strategy_presets[preset_index], strategy_mask, &strategy_set);
   opng_init_iteration(options.filter_set,
      filter_presets[preset_index], filter_mask, &filter_set);

   /* Replace the empty sets with the libpng's "best guess" heuristics. */
   if (compr_level_set == BITSET_EMPTY)
      BITSET_SET(compr_level_set, Z_BEST_COMPRESSION);  /* -zc9 */
   if (mem_level_set == BITSET_EMPTY)
      BITSET_SET(mem_level_set, 8);
   if (image.bit_depth < 8 || image.palette != NULL)
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

   /* Store the results into process. */
   process.compr_level_set = compr_level_set;
   process.mem_level_set   = mem_level_set;
   process.strategy_set    = strategy_set;
   process.filter_set      = filter_set;
   t1 = bitset_count(compr_level_set) *
        bitset_count(strategy_set & ~((1 << Z_HUFFMAN_ONLY) | (1 << Z_RLE)));
   t2 = bitset_count(strategy_set &  ((1 << Z_HUFFMAN_ONLY) | (1 << Z_RLE)));
   process.num_iterations = (t1 + t2) *
        bitset_count(mem_level_set) * bitset_count(filter_set);
   OPNG_ENSURE(process.num_iterations > 0, "Invalid iteration parameters");
}


/*
 * Iteration
 */
static void
opng_iterate(void)
{
   bitset_t compr_level_set, mem_level_set, strategy_set, filter_set;
   int compr_level, mem_level, strategy, filter;
   int counter;
   int line_reused;

   OPNG_ENSURE(process.num_iterations > 0, "Iterations not initialized");
   if ((process.num_iterations == 1) &&
       (process.status & OUTPUT_NEEDS_NEW_IDAT))
   {
      /* We already know this combination will be selected.
       * Do not waste time running it twice.
       */
      process.best_idat_size = 0;
      process.best_compr_level = opng_bitset_get_first(process.compr_level_set);
      process.best_mem_level   = opng_bitset_get_first(process.mem_level_set);
      process.best_strategy    = opng_bitset_get_first(process.strategy_set);
      process.best_filter      = opng_bitset_get_first(process.filter_set);
      return;
   }

   /* Prepare for the big iteration. */
   compr_level_set = process.compr_level_set;
   mem_level_set   = process.mem_level_set;
   strategy_set    = process.strategy_set;
   filter_set      = process.filter_set;
   process.best_idat_size   = PNG_UINT_31_MAX + 1;
   process.best_compr_level = -1;
   process.best_mem_level   = -1;
   process.best_strategy    = -1;
   process.best_filter      = -1;

   /* Iterate through the "hyper-rectangle" (zc, zm, zs, f). */
   usr_printf("\nTrying:\n");
   line_reused = 0;
   counter = 0;
   for (filter = FILTER_MIN;
        filter <= FILTER_MAX; ++filter)
      if (BITSET_GET(filter_set, filter))
         for (strategy = STRATEGY_MIN;
              strategy <= STRATEGY_MAX; ++strategy)
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
                           usr_printf(
                              "  zc = %d  zm = %d  zs = %d  f = %d",
                              compr_level, mem_level, strategy, filter);
                           usr_progress(counter, process.num_iterations);
                           ++counter;
                           opng_write_file(NULL,
                              compr_level, mem_level, strategy, filter);
                           if (process.out_idat_size > PNG_UINT_31_MAX)
                           {
                              if (options.verbose)
                              {
                                 usr_printf("\t\tIDAT too big\n");
                                 line_reused = 0;
                              }
                              else
                              {
                                 usr_print_cntrl('\r');  /* CR: reset line */
                                 line_reused = 1;
                              }
                              continue;
                           }
                           usr_printf("\t\tIDAT size = %lu\n",
                              (unsigned long)process.out_idat_size);
                           line_reused = 0;
                           if (process.best_idat_size < process.out_idat_size)
                              continue;
                           if (process.best_idat_size == process.out_idat_size
                               && process.best_strategy >= Z_HUFFMAN_ONLY)
                              continue;  /* it's neither smaller nor faster */
                           process.best_compr_level = compr_level;
                           process.best_mem_level   = mem_level;
                           process.best_strategy    = strategy;
                           process.best_filter      = filter;
                           process.best_idat_size   = process.out_idat_size;
                           if (!options.full)
                              process.max_idat_size = process.out_idat_size;
                        }
                  }
               compr_level_set = saved_level_set;
            }
   if (line_reused)
      usr_print_cntrl(-31);  /* Minus N: erase N chars from start of line */

   OPNG_ENSURE(counter == process.num_iterations,
      "Inconsistent iteration counter");
   usr_progress(counter, process.num_iterations);
}


/*
 * Iteration finalization
 */
static void
opng_finish_iterations(void)
{
   if (process.best_idat_size + process.out_plte_trns_size
       < process.in_idat_size + process.in_plte_trns_size)
      process.status |= OUTPUT_NEEDS_NEW_IDAT;
   if (process.status & OUTPUT_NEEDS_NEW_IDAT)
   {
      usr_printf("\nSelecting parameters:\n"
                 "  zc = %d  zm = %d  zs = %d  f = %d",
                 process.best_compr_level, process.best_mem_level,
                 process.best_strategy, process.best_filter);
      if (process.best_idat_size != 0)  /* trials have been run */
         usr_printf("\t\tIDAT size = %lu",
                    (unsigned long)process.best_idat_size);
      usr_printf("\n");
   }
}


/*
 * Image file optimization
 */
static void
opng_optimize_impl(const char *infile_name)
{
   static FILE *infile, *outfile;         /* static or volatile is required */
   static const char *outfile_name, *bakfile_name;            /* by cexcept */
   static int new_outfile;
   char name_buf[FILENAME_MAX], tmp_buf[FILENAME_MAX];
   const char * volatile err_msg;

   memset(&process, 0, sizeof(process));
   if (options.force)
      process.status |= OUTPUT_NEEDS_NEW_IDAT;

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

   /* Check the error flag. This must be the first check. */
   if (process.status & INPUT_HAS_ERRORS)
   {
      usr_printf("Recoverable errors found in input.");
      if (options.fix)
      {
         usr_printf(" Fixing...\n");
         process.status |= OUTPUT_NEEDS_NEW_FILE;
      }
      else
      {
         usr_printf(" Rerun " PROGRAM_NAME " with -fix enabled.\n");
         Throw "Previous error(s) not fixed";
      }
   }

   /* Check the junk flag. */
   if (process.status & INPUT_HAS_JUNK)
      process.status |= OUTPUT_NEEDS_NEW_FILE;

   /* Check the PNG signature and datastream flags. */
   if (!(process.status & INPUT_HAS_PNG_SIGNATURE))
      process.status |= OUTPUT_NEEDS_NEW_FILE;
   if (process.status & INPUT_HAS_PNG_DATASTREAM)
   {
      if (options.nz && (process.status & OUTPUT_NEEDS_NEW_IDAT))
      {
         usr_printf(
            "IDAT recoding is necessary, but is disabled by the user.\n");
         Throw "Can't continue";
      }
   }
   else
      process.status |= OUTPUT_NEEDS_NEW_IDAT;

   /* Check the digital signature flag. */
   if (process.status & INPUT_HAS_DIGITAL_SIGNATURE)
   {
      usr_printf("Digital signature found in input.");
      if (options.force)
      {
         usr_printf(" Erasing...\n");
         process.status |= OUTPUT_NEEDS_NEW_FILE;
      }
      else
      {
         usr_printf(" Rerun " PROGRAM_NAME " with -force enabled.\n");
         Throw "Can't optimize digitally-signed files";
      }
   }

   /* Check the multi-image flag. */
   if (process.status & INPUT_HAS_MULTIPLE_IMAGES)
   {
      if (!options.snip && !(process.status & INPUT_IS_PNG_FILE))
      {
         usr_printf("Conversion to PNG requires snipping. "
                    "Rerun " PROGRAM_NAME " with -snip enabled.\n");
         Throw "Incompatible input format";
      }
   }
   if ((process.status & INPUT_HAS_APNG) && options.snip)
      process.status |= OUTPUT_NEEDS_NEW_FILE;

   /* Initialize the output file name. */
   outfile_name = NULL;
   if (!(process.status & INPUT_IS_PNG_FILE))
   {
      if (osys_path_chext(name_buf, sizeof(name_buf), infile_name,
                          ".png") == NULL)
         Throw "Can't create the output file (name too long)";
      outfile_name = name_buf;
   }
   if (options.out_name != NULL)
      outfile_name = options.out_name;  /* override the old name */
   if (options.dir_name != NULL)
   {
      const char *tmp_name;
      if (outfile_name != NULL)
      {
         strcpy(tmp_buf, outfile_name);
         tmp_name = tmp_buf;
      }
      else
         tmp_name = infile_name;
      if (osys_path_chdir(name_buf, sizeof(name_buf), tmp_name,
                          options.dir_name) == NULL)
         Throw "Can't create the output file (name too long)";
      outfile_name = name_buf;
   }
   if (outfile_name == NULL)
   {
      outfile_name = infile_name;
      new_outfile = 0;
   }
   else
   {
      int test_eq = osys_ftest_eq(infile_name, outfile_name);
      if (test_eq >= 0)
         new_outfile = (test_eq == 0);
      else
         new_outfile = (osys_path_cmp(infile_name, outfile_name) != 0);
   }

   /* Initialize the backup file name. */
   bakfile_name = tmp_buf;
   if (new_outfile)
   {
      if (osys_path_mkbak(tmp_buf, sizeof(tmp_buf), outfile_name) == NULL)
         bakfile_name = NULL;
   }
   else
   {
      if (osys_path_mkbak(tmp_buf, sizeof(tmp_buf), infile_name) == NULL)
         bakfile_name = NULL;
   }
   /* Check the name even in simulation mode, to ensure a uniform behavior. */
   if (bakfile_name == NULL)
      Throw "Can't create backup file (name too long)";
   /* Check the backup file before engaging into lengthy trials. */
   if (!options.simulate && osys_ftest(outfile_name, "e") == 0)
   {
      if (new_outfile && !options.keep)
         Throw "The output file exists, try backing it up (use -keep)";
      if (osys_ftest(outfile_name, "fw") != 0 ||
          osys_ftest(bakfile_name, "e") == 0)
         Throw "Can't back up the existing output file";
   }

   /* Display the input IDAT/file sizes. */
   if (process.status & INPUT_HAS_PNG_DATASTREAM)
      usr_printf("Input IDAT size = %lu bytes\n",
                 (unsigned long)process.in_idat_size);
   usr_printf("Input file size = %lu bytes\n", process.in_file_size);

   /* Find the best parameters and see if it's worth recompressing. */
   if (!options.nz || (process.status & OUTPUT_NEEDS_NEW_IDAT))
   {
      opng_init_iterations();
      opng_iterate();
      opng_finish_iterations();
   }
   if (process.status & OUTPUT_NEEDS_NEW_IDAT)
      process.status |= OUTPUT_NEEDS_NEW_FILE;

   /* Stop here? */
   if (!(process.status & OUTPUT_NEEDS_NEW_FILE))
   {
      usr_printf("\n%s is already optimized.\n", infile_name);
      if (!new_outfile)
         return;
   }
   if (options.simulate)
   {
      usr_printf("\nNo output: simulation mode.\n");
      return;
   }

   /* Make room for the output file. */
   if (new_outfile)
   {
      usr_printf("\nOutput file: %s\n", outfile_name);
      if (options.dir_name != NULL)
         osys_dir_make(options.dir_name);
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
      if (process.status & OUTPUT_NEEDS_NEW_IDAT)
      {
         /* Write a brand new PNG datastream to the output. */
         opng_write_file(outfile,
            process.best_compr_level, process.best_mem_level,
            process.best_strategy, process.best_filter);
      }
      else
      {
         /* Copy the input PNG datastream to the output. */
         infile = osys_fopen_at((new_outfile ? infile_name : bakfile_name),
            "rb", process.in_datastream_offset, SEEK_SET);
         if (infile == NULL)
            Throw "Can't reopen the input file";
         Try
         {
            process.best_idat_size = process.in_idat_size;
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
   if (options.preserve)
      osys_fattr_copy(outfile_name, (new_outfile ? infile_name : bakfile_name));

   /* Remove the backup file if it is not needed. */
   if (!new_outfile && !options.keep)
   {
      if (remove(bakfile_name) != 0)
         Throw "Can't remove the backup file";
   }

   /* Display the output IDAT/file sizes. */
   usr_printf("\nOutput IDAT size = %lu bytes",
              (unsigned long)process.out_idat_size);
   if (process.status & INPUT_HAS_PNG_DATASTREAM)
   {
      usr_printf(" (");
      opng_print_size_difference(process.in_idat_size,
                                 process.out_idat_size, 0);
      usr_printf(")");
   }
   usr_printf("\nOutput file size = %lu bytes (", process.out_file_size);
   opng_print_size_difference(process.in_file_size,
                              process.out_file_size, 1);
   usr_printf(")\n");
}


/*
 * Engine initialization
 */
int
opng_initialize(const struct opng_options *init_options,
                const struct opng_ui *init_ui)
{
   /* Initialize and check the validity of the user interface. */
   usr_printf      = init_ui->printf_fn;
   usr_print_cntrl = init_ui->print_cntrl_fn;
   usr_progress    = init_ui->progress_fn;
   usr_panic       = init_ui->panic_fn;
   if (usr_printf == NULL      ||
       usr_print_cntrl == NULL ||
       usr_progress == NULL    ||
       usr_panic == NULL)
      return -1;

   /* Initialize and adjust the user options. */
   options = *init_options;
   if (options.optim_level == 0)
   {
      options.nb = options.nc = options.np = 1;
      options.nz = 1;
   }

   /* Start the engine. */
   memset(&summary, 0, sizeof(summary));
   engine.started = 1;
   return 0;
}


/*
 * Engine execution
 */
int
opng_optimize(const char *infile_name)
{
   const char *err_msg;
   volatile int result;  /* needs not be volatile, but keeps compilers happy */

   OPNG_ENSURE(engine.started, "The OptiPNG engine is not running");

   usr_printf("** Processing: %s\n", infile_name);
   ++summary.file_count;
   opng_clear_image_info();
   Try
   {
      opng_optimize_impl(infile_name);
      if (process.status & INPUT_HAS_ERRORS)
      {
         ++summary.err_count;
         ++summary.fix_count;
      }
      if (process.status & INPUT_HAS_MULTIPLE_IMAGES)
      {
         if (options.snip)
            ++summary.snip_count;
      }
      result = 0;
   }
   Catch (err_msg)
   {
      ++summary.err_count;
      opng_print_error(err_msg);
      result = -1;
   }
   opng_destroy_image_info();
   usr_printf("\n");
   return result;
}


/*
 * Engine finalization
 */
int
opng_finalize(void)
{
   /* Print the status report. */
   if (options.verbose || summary.snip_count > 0 || summary.err_count > 0)
   {
      usr_printf("** Status report\n");
      usr_printf("%u file(s) have been processed.\n", summary.file_count);
      if (summary.snip_count > 0)
      {
         usr_printf("%u multi-image file(s) have been snipped.\n",
                    summary.snip_count);
      }
      if (summary.err_count > 0)
      {
         usr_printf("%u error(s) have been encountered.\n",
                    summary.err_count);
         if (summary.fix_count > 0)
            usr_printf("%u erroneous file(s) have been fixed.\n",
                       summary.fix_count);
      }
   }

   /* Stop the engine. */
   engine.started = 0;
   return 0;
}
