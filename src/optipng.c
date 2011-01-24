/*
 * OptiPNG: Advanced PNG optimization program.
 * http://optipng.sourceforge.net/
 *
 * Copyright (C) 2001-2010 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the attached LICENSE for more information.
 *
 * PNG optimization is described in detail in the PNG-Tech article
 * "A guide to PNG optimization"
 * http://optipng.sourceforge.net/pngtech/optipng.html
 *
 * The idea of running multiple compression trials with different
 * PNG filters and zlib parameters is inspired from the pngcrush
 * program by Glenn Randers-Pehrson.
 * The idea of performing lossless image reductions is inspired from
 * the pngrewrite program by Jason Summers.
 *
 * Requirements:
 *    ANSI/ISO C compiler and library.
 *    zlib version 1.2.1 or newer.
 *    libpng version 1.2.9 or newer, with pngxtern.
 *    cexcept version 2.0.1 or newer.
 *    POSIX or Windows API for enhanced functionality.
 */


#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "optipng.h"
#include "proginfo.h"

#include "cbitset.h"
#include "osys.h"
#include "png.h"
#include "pngx.h"
#include "zlib.h"


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
   "Synopsis:\n"
   "    optipng [options] files ...\n"
   "Files:\n"
   "    Image files of type: PNG, BMP, GIF, PNM or TIFF\n"
   "Basic options:\n"
   "    -?, -h, -help\tshow the extended help\n"
   "    -o <level>\t\toptimization level (0-7)\t\tdefault 2\n"
   "    -v\t\t\tverbose mode / show copyright and version info\n"
   "Examples:\n"
   "    optipng file.png\t\t\t(default speed)\n"
   "    optipng -o5 file.png\t\t(moderately slow)\n"
   "    optipng -o7 file.png\t\t(very slow)\n"
   "Type \"optipng -h\" for extended help.\n";

static const char *msg_help =
   "Synopsis:\n"
   "    optipng [options] files ...\n"
   "Files:\n"
   "    Image files of type: PNG, BMP, GIF, PNM or TIFF\n"
   "Basic options:\n"
   "    -?, -h, -help\tshow this help\n"
   "    -o <level>\t\toptimization level (0-7)\t\tdefault 2\n"
   "    -v\t\t\tverbose mode / show copyright and version info\n"
   "General options:\n"
   "    -fix\t\tenable error recovery\n"
   "    -force\t\tenforce writing of a new output file\n"
#if 0  /* not implemented */
   "    -jobs <number>\tallow parallel jobs\n"
#endif
   "    -keep\t\tkeep a backup of the modified files\n"
   "    -preserve\t\tpreserve file attributes if possible\n"
   "    -quiet\t\tquiet mode\n"
   "    -simulate\t\tsimulation mode\n"
   "    -snip\t\tcut one image out of multi-image or animation files\n"
   "    -out <file>\t\twrite output file to <file>\n"
   "    -dir <directory>\twrite output file(s) to <directory>\n"
   "    -log <file>\t\tlog messages to <file>\n"
   "    --\t\t\tstop option switch parsing\n"
   "Optimization options:\n"
#if 0  /* not implemented */
   "    -b  <depth>\t\tbit depth (1,2,4,8,16)\t\t\tdefault <min>\n"
   "    -c  <type>\t\tcolor type (0,2,3,4,6)\t\t\tdefault <input>\n"
#endif
   "    -f  <filters>\tPNG delta filters (0-5)\t\t\tdefault 0,5\n"
   "    -i  <type>\t\tPNG interlace type (0-1)\t\tdefault <input>\n"
   "    -zc <levels>\tzlib compression levels (1-9)\t\tdefault 9\n"
   "    -zm <levels>\tzlib memory levels (1-9)\t\tdefault 8\n"
   "    -zs <strategies>\tzlib compression strategies (0-3)\tdefault 0-3\n"
   "    -zw <window size>\tzlib window size (32k,16k,8k,4k,2k,1k,512,256)\n"
   "    -full\t\tproduce a full report on IDAT (might reduce speed)\n"
   "    -nb\t\t\tno bit depth reduction\n"
   "    -nc\t\t\tno color type reduction\n"
   "    -np\t\t\tno palette reduction\n"
#if 0  /* not implemented */
   "    -nm\t\t\tno metadata optimization\n"
#endif
   "    -nx\t\t\tno reductions\n"
   "    -nz\t\t\tno IDAT recoding\n"
   "Optimization details:\n"
   "    The optimization level presets\n"
   "        -o0  <=>  -o1 -nx -nz\n"
   "        -o1  <=>  [use the libpng heuristics]\t(1 trial)\n"
   "        -o2  <=>  -zc9 -zm8 -zs0-3 -f0,5\t(8 trials)\n"
   "        -o3  <=>  -zc9 -zm8-9 -zs0-3 -f0,5\t(16 trials)\n"
   "        -o4  <=>  -zc9 -zm8 -zs0-3 -f0-5\t(24 trials)\n"
   "        -o5  <=>  -zc9 -zm8-9 -zs0-3 -f0-5\t(48 trials)\n"
   "        -o6  <=>  -zc1-9 -zm8 -zs0-3 -f0-5\t(120 trials)\n"
   "        -o7  <=>  -zc1-9 -zm8-9 -zs0-3 -f0-5\t(240 trials)\n"
   "    The libpng heuristics\n"
   "        -o1  <=>  -zc9 -zm8 -zs0 -f0\t\t(if PLTE is present)\n"
   "        -o1  <=>  -zc9 -zm8 -zs1 -f5\t\t(if PLTE is not present)\n"
   "    The most exhaustive search (not generally recommended)\n"
   "      [no preset] -zc1-9 -zm1-9 -zs0-3 -f0-5\t(1080 trials)\n"
   "Examples:\n"
   "    optipng file.png\t\t\t\t(default speed)\n"
   "    optipng -o5 file.png\t\t\t(moderately slow)\n"
   "    optipng -o7 file.png\t\t\t(very slow)\n"
   "    optipng -i1 -o7 -v -full -sim experiment.png\n";


static enum { OP_NONE, OP_HELP, OP_RUN } operation;
static struct opng_options options;
static FILE *con_file;
static FILE *log_file;
int start_of_line;


/*
 * Error handling
 */
static void
error(const char *fmt, ...)
{
    va_list arg_ptr;

    /* Print the error message to stderr and exit. */
    fprintf(stderr, "** Error: ");
    va_start(arg_ptr, fmt);
    vfprintf(stderr, fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}


/*
 * Panic handling
 */
static void
panic(const char *msg)
{
    /* Print the panic message to stderr and terminate abnormally. */
    fprintf(stderr, "\n** INTERNAL ERROR: %s\n", msg);
    fprintf(stderr, "Please submit a defect report.\n" PROGRAM_URI "\n\n");
    fflush(stderr);
    osys_terminate();
}


/*
 * String utility
 */
static int
opng_strcasecmp(const char *str1, const char *str2)
{
    int ch1, ch2;

    /* Perform a case-insensitive string comparison. */
    for ( ; ; )
    {
        ch1 = tolower(*str1++);
        ch2 = tolower(*str2++);
        if (ch1 != ch2)
            return ch1 - ch2;
        if (ch1 == 0)
            return 0;
    }
    /* FIXME: This function is not MBCS-aware. */
}


/*
 * String utility
 */
static char *
opng_strltrim(const char *str)
{
    /* Skip the leading whitespace characters. */
    while (isspace(*str))
        ++str;
    return (char *)str;
}


/*
 * String utility
 */
static char *
opng_strtail(const char *str, size_t num)
{
    size_t len;

    /* Return up to num rightmost characters. */
    len = strlen(str);
    if (len <= num)
        return (char *)str;
    return (char *)str + len - num;
}


/*
 * String conversion utility
 */
static int
opng_str2ulong_base10(unsigned long *out_val, const char *in_str,
                      int allow_multiplier)
{
    const char *begin_ptr;
    char *end_ptr;
    unsigned long multiplier;

    /* Extract the value from the string. */
    /* Do not allow the minus sign, not even for -0. */
    begin_ptr = end_ptr = opng_strltrim(in_str);
    if (*begin_ptr >= '0' && *begin_ptr <= '9')
        *out_val = strtoul(begin_ptr, &end_ptr, 10);
    if (begin_ptr == end_ptr)
    {
        errno = EINVAL;  /* matching failure */
        *out_val = 0;
        return -1;
    }

    if (allow_multiplier)
    {
        /* Check for the following SI suffixes:
         *   'K' or 'k': kibi (1024)
         *   'M':        mebi (1024 * 1024)
         *   'G':        gibi (1024 * 1024 * 1024)
         */
        if (*end_ptr == 'k' || *end_ptr == 'K')
        {
            ++end_ptr;
            multiplier = 1024UL;
        }
        else if (*end_ptr == 'M')
        {
            ++end_ptr;
            multiplier = 1024UL * 1024UL;
        }
        else if (*end_ptr == 'G')
        {
            ++end_ptr;
            multiplier = 1024UL * 1024UL * 1024UL;
        }
        else
            multiplier = 1;
        if (multiplier > 1)
        {
            if (*out_val > ULONG_MAX / multiplier)
            {
                errno = ERANGE;  /* overflow */
                *out_val = ULONG_MAX;
            }
            else
                *out_val *= multiplier;
        }
    }

    /* Check for trailing garbage. */
    if (*opng_strltrim(end_ptr) != 0)
    {
        errno = EINVAL;  /* garbage in input */
        return -1;
    }
    return 0;
}


/*
 * String conversion utility
 */
static int
opng_rangeset2bitset(bitset_t *out_val, const char *in_str)
{
    size_t end_idx;

    /* Extract the bitset value from the rangeset string. */
    *out_val = rangeset_string_to_bitset(in_str, &end_idx);
    if (end_idx == 0 || *opng_strltrim(in_str + end_idx) != 0)
    {
        errno = EINVAL;
        return -1;
    }
    return 0;
}


/*
 * Command line utility
 */
static void
err_option_arg(const char *opt, const char *opt_arg)
{
    /* Issue an error regarding the incorrect value of the option argument. */
    if (opt_arg == NULL || *opng_strltrim(opt_arg) == 0)
        error("Missing argument for option %s", opt);
    else
        error("Invalid argument for option %s: %s", opt, opt_arg);
}


/*
 * Command line utility
 */
static int
check_num_option(const char *opt, const char *opt_arg,
                 int lowest, int highest)
{
    unsigned long value;

    /* Extract the numeric value from the option argument. */
    if (opng_str2ulong_base10(&value, opt_arg, 0) != 0 ||
            value > INT_MAX || (int)value < lowest || (int)value > highest)
        err_option_arg(opt, opt_arg);
    return (int)value;
}


/*
 * Command line utility
 */
static int
check_power2_option(const char *opt, const char *opt_arg,
                    int lowest, int highest)
{
    unsigned long value;
    int result;

    /* Extract the exact log2 of the numeric value from the option argument. */
    /* Allow the 'k', 'M', 'G' suffixes. */
    if (opng_str2ulong_base10(&value, opt_arg, 1) == 0)
    {
        if (lowest < 0)
            lowest = 0;
        if (highest > (int)(CHAR_BIT * sizeof(long) - 2))
            highest = (int)(CHAR_BIT * sizeof(long) - 2);
        for (result = lowest; result <= highest; ++result)
        {
            if ((1UL << result) == value)
                return result;
        }
    }
    err_option_arg(opt, opt_arg);
    return -1;
}


/*
 * Command line utility
 */
static bitset_t
check_rangeset_option(const char *opt, const char *opt_arg,
                      bitset_t result_mask)
{
    bitset_t result;

    /* Extract the rangeset from the option argument. */
    if (opng_rangeset2bitset(&result, opt_arg) == 0)
        result &= result_mask;
    else
        result = BITSET_EMPTY;
    if (result == BITSET_EMPTY)
        err_option_arg(opt, opt_arg);
    return result;
}


/*
 * Command line parsing
 */
static int
scan_option(const char *str,
            char opt_buf[], size_t opt_buf_size, char **opt_arg_ptr)
{
    const char *ptr;
    unsigned int opt_len;

    /* Check if arg is an "-option". */
    if (str[0] != '-' || str[1] == 0)  /* no "-option", or just "-" */
        return 0;

    /* Extract the normalized option, and possibly the option argument. */
    opt_len = 0;
    ptr = str + 1;
    while (*ptr == '-')  /* "--option", "---option", etc. */
        ++ptr;
    if (*ptr == 0)  /* "--" */
        --ptr;
    if (isalpha(*ptr))  /* "-option" */
    {
        do
        {
            if (opt_buf_size > opt_len)  /* truncate "-verylongoption" */
                opt_buf[opt_len] = (char)tolower(*ptr);
            ++opt_len;
            ++ptr;
        } while (isalpha(*ptr) || (*ptr == '-'));  /* "-option", "-option-x" */
        while (*(ptr - 1) == '-')  /* put back trailing '-' in "-option-" */
        {
            --opt_len;
            --ptr;
        }
    }
    else  /* "--", "-@", etc. */
    {
        if (opt_buf_size > 1)
            opt_buf[0] = *ptr;
        opt_len = 1;
        ++ptr;
    }

    /* Finalize the normalized option. */
    if (opt_buf_size > 0)
    {
        if (opt_len < opt_buf_size)
            opt_buf[opt_len] = '\0';
        else
            opt_buf[opt_buf_size - 1] = '\0';
    }
    if (*ptr == '=')  /* "-option=arg" */
        ++ptr;
    else while (isspace(*ptr))  /* "-option arg" */
        ++ptr;
    *opt_arg_ptr = (*ptr != 0) ? (char *)ptr : NULL;
    return 1;
}


/*
 * Command line parsing
 */
static void
parse_args(int argc, char *argv[])
{
    char opt[16];
    size_t opt_len;
    char *arg, *xopt;
    unsigned int file_count;
    int stop_switch;
    bitset_t set;
    int val, i;

    /* Initialize. */
    memset(&options, 0, sizeof(options));
    options.optim_level = -1;
    options.interlace = -1;
    file_count = 0;

    /* Iterate over args. */
    stop_switch = 0;
    for (i = 1; i < argc; ++i)
    {
        arg = argv[i];
        if (stop_switch || scan_option(arg, opt, sizeof(opt), &xopt) < 1)
        {
            ++file_count;
            continue;  /* leave file names for process_files() */
        }
        opt_len = strlen(opt);

        /* Prevent process_files() from seeing this arg. */
        argv[i] = NULL;

        /* Check the simple options (without option arguments). */
        if (strcmp("-", opt) == 0)
        {
            /* -- */
            stop_switch = 1;
        }
        else if (strcmp("?", opt) == 0 ||
                 strncmp("help", opt, opt_len) == 0)
        {
            /* -? | -h | ... | -help */
            options.help = 1;
        }
        else if (strncmp("fix", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -fi | -fix */
            options.fix = 1;
        }
        else if (strncmp("force", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -fo | ... | -force */
            options.force = 1;
        }
        else if (strncmp("full", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -fu | ... | -full */
            options.full = 1;
        }
        else if (strncmp("keep", opt, opt_len) == 0)
        {
            /* -k | ... | -keep */
            options.keep = 1;
        }
        else if (strcmp("nb", opt) == 0)
        {
            /* -nb */
            options.nb = 1;
        }
        else if (strcmp("nc", opt) == 0)
        {
            /* -nc */
            options.nc = 1;
        }
        else if (strcmp("nm", opt) == 0)
        {
            /* -nm */
            /* options.nm = 1; */
            error("Metadata optimization is not implemented");
        }
        else if (strcmp("np", opt) == 0)
        {
            /* -np */
            options.np = 1;
        }
        else if (strcmp("nx", opt) == 0)
        {
            /* -nx */
            options.nb = options.nc = options.np = 1;
            /* options.nm = 1; */
        }
        else if (strcmp("nz", opt) == 0)
        {
            /* -nz */
            options.nz = 1;
        }
        else if (strncmp("preserve", opt, opt_len) == 0)
        {
            /* -p | ... | -preserve */
            options.preserve = 1;
        }
        else if (strncmp("quiet", opt, opt_len) == 0)
        {
            /* -q | ... | -quiet */
            options.quiet = 1;
        }
        else if (strncmp("simulate", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -si | ... | -simulate */
            options.simulate = 1;
        }
        else if (strncmp("snip", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -sn | ... | -snip */
            options.snip = 1;
        }
        else if (strcmp("v", opt) == 0)
        {
            /* -v */
            options.verbose = 1;
            options.version = 1;
        }
        else if (strncmp("verbose", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -verb | ... | -verbose */
            options.verbose = 1;
        }
        else if (strncmp("version", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -vers | ... | -version */
            options.version = 1;
        }
        else  /* possibly an option with an argument */
        {
            if (xopt == NULL)
            {
                if (++i < argc)
                {
                    xopt = argv[i];
                    /* Prevent process_files() from seeing this xopt. */
                    argv[i] = NULL;
                }
                else
                    xopt = "";
            }
        }

        /* Check the options that have option arguments. */
        if (xopt == NULL)
        {
            /* An option without argument has already been recognized. */
        }
        else if (strcmp("o", opt) == 0)
        {
            /* -o NUM */
            val = check_num_option("-o", xopt, 0, INT_MAX);
            if (options.optim_level < 0)
                options.optim_level = val;
            else if (options.optim_level != val)
                error("Multiple optimization levels are not permitted");
        }
        else if (strcmp("i", opt) == 0)
        {
            /* -i NUM */
            val = check_num_option("-i", xopt, 0, 1);
            if (options.interlace < 0)
                options.interlace = val;
            else if (options.interlace != val)
                error("Multiple interlace types are not permitted");
        }
        else if (strcmp("b", opt) == 0)
        {
            /* -b NUM */
            /* options.bit_depth = ... */
            error("Selection of bit depth is not implemented");
        }
        else if (strcmp("c", opt) == 0)
        {
            /* -c NUM */
            /* options.color_type = ... */
            error("Selection of color type is not implemented");
        }
        else if (strcmp("f", opt) == 0)
        {
            /* -f SET */
            set = check_rangeset_option("-f", xopt, OPNG_FILTER_SET_MASK);
            options.filter_set |= set;
        }
        else if (strcmp("zc", opt) == 0)
        {
            /* -zc SET */
            set = check_rangeset_option("-zc", xopt, OPNG_COMPR_LEVEL_SET_MASK);
            options.compr_level_set |= set;
        }
        else if (strcmp("zm", opt) == 0)
        {
            /* -zm SET */
            set = check_rangeset_option("-zm", xopt, OPNG_MEM_LEVEL_SET_MASK);
            options.mem_level_set |= set;
        }
        else if (strcmp("zs", opt) == 0)
        {
            /* -zs SET */
            set = check_rangeset_option("-zs", xopt, OPNG_STRATEGY_SET_MASK);
            options.strategy_set |= set;
        }
        else if (strcmp("zw", opt) == 0)
        {
            /* -zw NUM */
            val = check_power2_option("-zw", xopt, 8, 15);
            if (options.window_bits == 0)
                options.window_bits = val;
            else if (options.window_bits != val)
                error("Multiple window sizes are not permitted");
        }
        else if (strncmp("out", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -ou PATH | -out PATH */
            if (options.out_name != NULL)
                error("Multiple output file names are not permitted");
            if (xopt[0] == 0)
                err_option_arg("-out", NULL);
            options.out_name = xopt;
        }
        else if (strncmp("dir", opt, opt_len) == 0)
        {
            /* -d PATH | ... | -dir PATH */
            if (options.dir_name != NULL)
                error("Multiple output dir names are not permitted");
            if (xopt[0] == 0)
                err_option_arg("-dir", NULL);
            options.dir_name = xopt;
        }
        else if (strncmp("log", opt, opt_len) == 0)
        {
            /* -l PATH | ... | -log PATH */
            if (options.log_name != NULL)
                error("Multiple log file names are not permitted");
            if (xopt[0] == 0)
                err_option_arg("-log", NULL);
            options.log_name = xopt;
        }
        else if (strncmp("jobs", opt, opt_len) == 0)
        {
            /* -j NUM | ... | -jobs NUM */
            error("Parallel processing is not implemented");
        }
        else if (strcmp("erase", opt) == 0 ||
                 strcmp("strip", opt) == 0 ||
                 strcmp("protect", opt) == 0)
        {
            /* -erase DATA | -strip DATA | -protect DATA */
            error("Lossy operations are not currently supported");
        }
        else
        {
            error("Unrecognized option: %s", arg);
        }
    }

    /* Finalize. */
    if (options.out_name != NULL)
    {
        if (file_count > 1)
            error("The option -out requires one input file");
        if (options.dir_name != NULL)
            error("The options -out and -dir are mutually exclusive");
    }
    if (options.log_name != NULL)
    {
        if (opng_strcasecmp(".log", opng_strtail(options.log_name, 4)) != 0)
            error("To prevent accidental data corruption, "
                  "the log file name must end with \".log\"");
    }
    operation = (options.help || file_count == 0) ? OP_HELP : OP_RUN;
}


/*
 * Initialization
 */
static void
app_init(void)
{
    /* Initialize the console output. */
    con_file = (!options.quiet || options.help) ? stdout : NULL;

    /* Open the log file, line-buffered, if requested. */
    if (options.log_name != NULL)
    {
        if ((log_file = fopen(options.log_name, "a")) == NULL)
            error("Can't open log file: %s\n", options.log_name);
        setvbuf(log_file, NULL, _IOLBF, BUFSIZ);
    }

    /* Initialize the internal printing routines. */
    start_of_line = 1;
}


/*
 * Finalization
 */
static void
app_finish(void)
{
    if (log_file != NULL)
    {
        /* Close the log file. */
        fclose(log_file);
    }
}


/*
 * Application-defined printf callback
 */
static void
app_printf(const char *fmt, ...)
{
    va_list arg_ptr;

    if (fmt[0] == 0)
        return;
    start_of_line = (fmt[strlen(fmt) - 1] == '\n') ? 1 : 0;

    if (con_file != NULL)
    {
        va_start(arg_ptr, fmt);
        vfprintf(con_file, fmt, arg_ptr);
        va_end(arg_ptr);
    }
    if (log_file != NULL)
    {
        va_start(arg_ptr, fmt);
        vfprintf(log_file, fmt, arg_ptr);
        va_end(arg_ptr);
    }
}


/*
 * Application-defined control print callback
 */
static void
app_print_cntrl(int cntrl_code)
{
    const char *con_str, *log_str;
    int i;

    if (cntrl_code == '\r')
    {
        /* CR: reset line in console, new line in log file. */
        con_str = "\r";
        log_str = "\n";
        start_of_line = 1;
    }
    else if (cntrl_code == '\v')
    {
        /* VT: new line if current line is not empty, nothing otherwise. */
        if (!start_of_line)
        {
            con_str = log_str = "\n";
            start_of_line = 1;
        }
        else
            con_str = log_str = "";
    }
    else if (cntrl_code < 0 && cntrl_code > -80 && start_of_line)
    {
        /* Minus N: erase first N characters from line, in console only. */
        if (con_file != NULL)
        {
            for (i = 0; i > cntrl_code; --i)
                fputc(' ', con_file);
        }
        con_str = "\r";
        log_str = "";
    }
    else
    {
        /* Unhandled control code (due to internal error): show err marker. */
        con_str = log_str = "<?>";
    }

    if (con_file != NULL)
        fputs(con_str, con_file);
    if (log_file != NULL)
        fputs(log_str, log_file);
}


/*
 * Application-defined progress update callback
 */
static void
app_progress(unsigned long current_step, unsigned long total_steps)
{
    /* There will be a potentially long wait, so flush the console output. */
    if (con_file != NULL)
        fflush(con_file);
    /* An eager flush of the line-buffered log file is not very important. */

    /* A GUI application would normally update a progress bar. */
    /* Here we ignore the progress info. */
    if (current_step && total_steps)
        return;
}


/*
 * File list processing
 */
static int
process_files(int argc, char *argv[])
{
    int result;
    struct opng_ui ui;
    int i;

    /* Initialize the optimization engine. */
    ui.printf_fn      = app_printf;
    ui.print_cntrl_fn = app_print_cntrl;
    ui.progress_fn    = app_progress;
    ui.panic_fn       = panic;
    if (opng_initialize(&options, &ui) != 0)
        panic("Can't initialize optimization engine");

    /* Iterate over file names. */
    result = EXIT_SUCCESS;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i] == NULL || argv[i][0] == 0)
            continue;  /* this was an "-option" */
        if (opng_optimize(argv[i]) != 0)
            result = EXIT_FAILURE;
    }

    /* Finalize the optimization engine. */
    if (opng_finalize() != 0)
        panic("Can't finalize optimization engine");

    return result;
}


/*
 * main
 */
int
main(int argc, char *argv[])
{
    int result;

    /* Parse the user options and initialize the application. */
    parse_args(argc, argv);
    app_init();

    /* Print the copyright and version info. */
    app_printf("%s", msg_intro);
    if (options.version)
    {
        /* Print the licensing and extended version info. */
        app_printf(msg_license);
        app_printf("Using libpng version %s and zlib version %s\n\n",
            png_get_libpng_ver(NULL), zlibVersion());
        /* Print the help text only if explicitly requested. */
        if (operation == OP_HELP && !options.help)
            operation = OP_NONE;
    }

    /* Print the help text or run the application. */
    switch (operation)
    {
    case OP_RUN:
        result = process_files(argc, argv);
        break;
    case OP_HELP:
        app_printf("%s", options.help ? msg_help : msg_short_help);
        /* Fall through. */
    default:
        result = EXIT_SUCCESS;
    }

    /* Finalize the application. */
    app_finish();
    return result;
}
