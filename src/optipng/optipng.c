/*
 * OptiPNG: Advanced PNG optimization program.
 * http://optipng.sourceforge.net/
 *
 * Copyright (C) 2001-2012 Cosmin Truta and the Contributing Authors.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 *
 * PNG optimization is described in detail in the PNG-Tech article
 * "A guide to PNG optimization"
 * http://optipng.sourceforge.net/pngtech/png_optimization.html
 *
 * The idea of running multiple compression trials with different
 * PNG filters and zlib parameters is inspired from the pngcrush
 * program by Glenn Randers-Pehrson.
 * The idea of performing lossless image reductions is inspired
 * from the pngrewrite program by Jason Summers.
 */

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "optk/bits.h"
#include "optk/io.h"
#include "sysexits.h"

#include "proginfo.h"
#include "strconv.h"

#include "opnglib/opnglib.h"


static const char *program_name = PROGRAM_NAME;

static const char *msg_intro =
    PRODUCT_NAME " version " PRODUCT_VERSION "\n"
    PRODUCT_COPYRIGHT ".\n";

static const char *msg_license =
    "This program is open-source software, "
    "distributed under the zlib license.\n"
    "See LICENSE.txt for more details.\n";

static const char *msg_help_synopsis =
    "Synopsis:\n"
    "    optipng [options] files ...\n";

static const char *msg_help_basic_options =
    "Basic options:\n"
    "    -?, -h, -help\tshow the extended help\n"
    "    -o <level>\t\toptimization level (1-6)\t\t[default 2]\n";

static const char *msg_help_options =
    "Basic options:\n"
    "    -?, -h, -help\tshow this help\n"
    "    -o <level>\t\toptimization level (1-6)\t\t[default 2]\n"
    "General options:\n"
    "    -backup\t\tback up modified files\n"
#if 0  /* internal */
    "    -debug\t\tenable debug features\n"
#endif
    "    -dir <directory>\twrite output file(s) to <directory>\n"
    "    -fix\t\tenable error recovery\n"
    "    -force\t\tenforce writing of a new output file\n"
    "    -no-clobber\t\tdo not overwrite existing files\n"
    "    -no-create\t\tdo not create any files\n"
    "    -out <file>\t\twrite output file to <file>\n"
    "    -preserve\t\tpreserve file attributes if possible\n"
    "    -quiet, -silent\trun in quiet mode\n"
    "    -stdout\t\twrite to the standard console output\n"
    "    -verbose\t\trun in verbose mode\n"
    "    -version\t\tshow copyright and version info\n"
    "    --\t\t\tstop option switch parsing\n"
    "Optimization options:\n"
    "    -f <filters>\tPNG delta filters (0-5)\t\t\t[default GUESS]\n"
    "    -i <type>\t\tPNG interlace type (0-1)\n"
    "    -zc <levels>\tzlib compression levels (1-9)\t\t[default 9]\n"
    "    -zm <levels>\tzlib memory levels (1-9)\t\t[default GUESS]\n"
    "    -zs <strategies>\tzlib compression strategies (0-3)\t[default GUESS]\n"
    "    -zw <size>\t\tzlib window size (256,512,1k,2k,4k,8k,16k,32k)\n"
    "    -nb\t\t\tno bit depth reduction\n"
    "    -nc\t\t\tno color type reduction\n"
    "    -np\t\t\tno palette reduction\n"
    "    -nx\t\t\tno reductions\n"
    "    -nz\t\t\tno IDAT recoding\n"
    "    -paranoid\t\tencode IDAT fully and show its size in report\n"
    "Editing options:\n"
    "    -set <object>=<val>\tset object (e.g. \"image.alpha.precision=1\")\n"
    "    -reset <objects>\treset image data objects (e.g. \"image.alpha\")\n"
    "    -strip <objects>\tstrip metadata objects (e.g. \"all\")\n"
    "    -protect <objects>\tprotect metadata objects (e.g. \"sRGB,iCCP\")\n"
    "    -snip\t\tcut one image out of multi-image or animation files\n"
    "Optimization levels:\n"
    "    -fastest\t<=>\t-zc3 -nz\t\t\t\t(0 or 1 trials)\n"
    "    -o1, -fast\t<=>\t-zc9\t\t\t\t\t(1 trial)\n"
    "    -o2\t\t<=>\t-zc9 -zs0-3 -f0,5\t\t\t(8 trials)\n"
    "    -o3\t\t<=>\t-zc9 -zm8-9 -zs0-3 -f0,5\t\t(16 trials)\n"
    "    -o4\t\t<=>\t-zc9 -zm8-9 -zs0-3 -f0-5\t\t(48 trials)\n"
    "    -o5\t\t<=>\t-zc3-9 -zm8-9 -zs0-3 -f0-5\t\t(192 trials)\n"
    "    -o6\t\t<=>\t-zc1-9 -zm7-9 -zs0-3 -f0-5\t\t(360 trials)\n"
    "    -o6 -zm1-9\t<=>\t-zc1-9 -zm1-9 -zs0-3 -f0-5\t\t(1080 trials)\n"
    "    -o7, -o8...\t<=>\t-o6\n"
    "Notes:\n"
    "    The default \"GUESS\" values for -f, -zm and -zs are chosen heuristically.\n"
    "    Exhaustive combinations such as \"-o6 -zm1-9\" are not generally recommended.\n";

static const char *msg_help_examples =
    "Examples:\n"
    "    optipng file.png\t\t\t\t\t\t(default speed)\n"
    "    optipng -o4 file.png\t\t\t\t\t(slow)\n"
    "    optipng -o6 file.png\t\t\t\t\t(very slow)\n";

static const char *msg_help_more =
    "Type \"optipng -h\" for extended help.\n";


/*
 * Local structures
 */
struct opng_local_options
{
    /* options handled outside of the opnglib engine */
    int debug;
    int fast;
    int help;
    int quiet;
    int version;
    const char *dir_name;
    const char *out_name;
};


/*
 * Local variables
 */
static char **fnames;
static int fname_count;
static struct opng_options options;
static struct opng_local_options local_options;
static opng_optimizer_t *the_optimizer;
static opng_transformer_t *the_transformer;
static int printed_to_stderr;


/*
 * Initialization
 */
static int
initialize(void)
{
    memset(&options, 0, sizeof(options));
    memset(&local_options, 0, sizeof(local_options));
    the_optimizer = opng_create_optimizer();
    if (the_optimizer == NULL)
        return -1;
    the_transformer = opng_create_transformer();
    if (the_transformer == NULL)
        return -1;
    printed_to_stderr = 0;
    return 0;
}

/*
 * Finalization
 */
static void
finalize(void)
{
    opng_destroy_optimizer(the_optimizer);
    opng_destroy_transformer(the_transformer);
}

/*
 * Warning handling
 */
static void
warning(const char *message)
{
    fprintf(stderr, "%s: warning: %s\n", program_name, message);
    printed_to_stderr = 1;
}

/*
 * Error handling
 */
static void
error(int exit_code, const char *fmt, ...)
{
    va_list arg_ptr;

    /* Print the error message to stderr and exit. */
    fprintf(stderr, "%s: error: ", program_name);
    va_start(arg_ptr, fmt);
    vfprintf(stderr, fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "\n");
    exit(exit_code);
}

/*
 * Error handling
 */
static void
error_xinfo(int exit_code, const char *xinfo, const char *fmt, ...)
{
    va_list arg_ptr;

    /* Print the error message to stderr and exit. */
    fprintf(stderr, "%s: error: ", program_name);
    va_start(arg_ptr, fmt);
    vfprintf(stderr, fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "\n");
    if (xinfo != NULL)
        fprintf(stderr, "%s: info: %s\n", program_name, xinfo);
    exit(exit_code);
}

/*
 * Error handling
 */
static void
error_option_arg_xinfo(const char *opt, const char *opt_arg, const char *xinfo)
{
    if (opt_arg == NULL || opt_arg[0] == 0)
        error_xinfo(EX_USAGE, xinfo,
                    "Missing argument for option %s", opt);
    else
        error_xinfo(EX_USAGE, xinfo,
                    "Invalid argument for option %s: %s", opt, opt_arg);
}

/*
 * Error handling
 */
static void
error_option_arg(const char *opt, const char *opt_arg)
{
    error_option_arg_xinfo(opt, opt_arg, NULL);
}

/*
 * Command line utility
 */
static int
check_num_option(const char *opt, const char *opt_arg,
                 int lowest, int highest)
{
    unsigned long value;
    int result;

    /* Extract the numeric value from the option argument. */
    if (numeric_string_to_ulong(&value, opt_arg, 0) >= 0 &&
        value <= INT_MAX)
    {
        result = (int)value;
        if (result >= lowest && result <= highest)
            return result;
    }
    error_option_arg(opt, opt_arg);
    return -1;
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
    if (numeric_string_to_ulong(&value, opt_arg, 1) >= 0)
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
    error_option_arg(opt, opt_arg);
    return -1;
}

/*
 * Command line utility
 */
static optk_bits_t
check_rangeset_option(const char *opt, const char *opt_arg,
                      optk_bits_t result_mask)
{
    optk_bits_t result;

    /* Extract the rangeset from the option argument. */
    if (rangeset_string_to_bits(&result, opt_arg) >= 0)
        result &= result_mask;
    else
        result = 0;
    if (result == 0)
        error_option_arg(opt, opt_arg);
    return (optk_bits_t)result;
}

/*
 * Command line utility
 */
static void
check_transform_option(const char *opt, /* not const */ char *opt_arg)
{
    int (*transform_fn)(opng_transformer_t *, const char *,
                        size_t *, size_t *, const char **);
    char *err_objname;
    size_t err_objname_offset, err_objname_length;
    const char *err_message;

    if (opt[1] == 'r')
        transform_fn = opng_transform_reset_objects;
    else if (opt[1] == 's' && opt[2] == 'e')
        transform_fn = opng_transform_set_object;
    else if (opt[1] == 's')
        transform_fn = opng_transform_strip_objects;
    else if (opt[1] == 'p')
        transform_fn = opng_transform_protect_objects;
    else
    {
        error(EX_SOFTWARE, "[BUG] check_transform_option: opt=\"%s\"\n", opt);
        return;
    }

    if (transform_fn(the_transformer, opt_arg,
                     &err_objname_offset, &err_objname_length,
                     &err_message) >= 0)
        return;

    if (err_objname_length > 0)
    {
        err_objname = opt_arg + err_objname_offset;
        err_objname[err_objname_length] = '\0';
        error_option_arg_xinfo(opt, err_objname, err_message);
    }
    else
        error_option_arg_xinfo(opt, opt_arg, err_message);
}

/*
 * Command line parsing
 */
static int
scan_option(const char *str,
            char **opt_ptr, size_t *opt_len_ptr, char **opt_arg_ptr)
{
    char *ptr;

    /* Check if arg is an "-option". */
    if (str[0] != '-' || str[1] == 0)  /* no "-option", or just "-" */
        return 0;

    /* Extract the normalized option, and possibly the option argument. */
    ptr = (char *)str + 1;
    if (*ptr == '-')  /* "--option" */
        ++ptr;
    if (*ptr == 0)  /* "--" */
        --ptr;
    *opt_ptr = ptr;
    if (isalnum(*ptr))  /* "-option", "-opti8n", "-8ption", "-option-x" */
    {
        do
        {
            ++ptr;
        } while (isalnum(*ptr) || (*ptr == '-'));
        *opt_len_ptr = ptr - *opt_ptr;
        if (*ptr == '=')  /* "-option=arg" */
        {
            *opt_arg_ptr = ptr + 1;
            return 1;
        }
        else if (*ptr == 0)  /* "-option" */
        {
            *opt_arg_ptr = NULL;
            return 1;
        }
        else if (!isspace(*ptr))  /* "-option!@#" */
        {
            /* Don't deal with ill-formed options here. */
            *opt_len_ptr += strlen(ptr);
            *opt_arg_ptr = NULL;
            return 1;
        }
        /* else if "-option arg" fall through */
    }
    else  /* "--", "-?", "-@", etc. */
        *opt_len_ptr = 1;
    do
    {
        ++ptr;
    } while (isspace(*ptr));
    *opt_arg_ptr = (*ptr != 0) ? ptr : NULL;
    return 1;
}

/*
 * Command line parsing
 */
static void
postprocess_options(void)
{
    /* Set up stderr for logging. */
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
    opng_set_logging_name(program_name);
    if (local_options.quiet)
    {
        /* Display warnings and errors (but no other informational messages)
         * in brief Unix style.
         */
        opng_set_logging_level(OPNG_MSG_WARNING);
        opng_set_logging_format(OPNG_MSGFMT_UNIX);
    }
    else
    {
        /* Display informational messages in fancy style. */
        opng_set_logging_level(OPNG_MSG_INFO);
        opng_set_logging_format(OPNG_MSGFMT_FANCY);
        if (printed_to_stderr)
            fprintf(stderr, "\n");
    }

    /* Set up the debug display. */
    if (local_options.debug)
        opng_set_logging_level(OPNG_MSG_DEBUG);

    /* Set up the optimization level. */
    if (local_options.fast != 0)
    {
        if (options.optim_level == 0 &&
            local_options.fast >= OPNG_OPTIM_LEVEL_MIN)
            options.optim_level = local_options.fast;
        else
            error(EX_USAGE, "Can't combine -fast, -fastest and -o");
    }
}

/*
 * Command line parsing
 */
static void
parse_args(int argc, char *argv[])
{
    char *arg, *opt, *xopt;
    size_t opt_len;
    int simple_opt;
    unsigned int out_name_count;
    optk_bits_t set;
    int val, i;

    memset(&options, 0, sizeof(options));
    options.interlace = -1;
    out_name_count = 0;

    for (i = 1; i < argc; ++i)
    {
        arg = argv[i];
        if (scan_option(arg, &opt, &opt_len, &xopt) < 1)
            break;
        if (strcmp("--", arg) == 0)
        {
            ++i;
            break;
        }

        /* Normalize the options that allow juxtaposed arguments. */
        if (strchr("fio", opt[0]) != NULL && isdigit(opt[1]))
        {
            /* -f0-5 <=> -f=0-5; -i1 <=> -i=1; -o3 <=> -o=3 */
            opt_len = 1;
            xopt = opt + 1;
        }
        else if (opt[0] == 'z' && isalpha(opt[1]) && isdigit(opt[2]))
        {
            /* -zc3-9 <=> -zc=3-9; etc. */
            opt_len = 2;
            xopt = opt + 2;
        }

        /* Check the simple options (i.e. without option arguments). */
        simple_opt = 1;
        if ((strncmp("?", opt, opt_len) == 0) ||
            (strncmp("help", opt, opt_len) == 0))
        {
            /* -? | -h | ... | -help */
            local_options.help = 1;
        }
        else if (strncmp("backup", opt, opt_len) == 0)
        {
            /* -b | ... | -backup */
            options.backup = 1;
        }
        else if (strncmp("clobber", opt, opt_len) == 0)
        {
            /* -c | ... | -clobber */
            /* Clobbering is enabled by default; do nothing. */
        }
        else if (strcmp("debug", opt) == 0)
        {
            /* -debug */
            /* Do not abbreviate this internal option. */
            local_options.debug = 1;
        }
        else if (strncmp("fast", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -fast */
            if (local_options.fast == 0)
                local_options.fast = OPNG_OPTIM_LEVEL_FAST;
            else if (local_options.fast != OPNG_OPTIM_LEVEL_FAST)
                local_options.fast = OPNG_OPTIM_LEVEL_MIN - 1;  /* error */
        }
        else if (strncmp("fastest", opt, opt_len) == 0 && opt_len >= 5)
        {
            /* -faste | ... | -fastest */
            if (local_options.fast == 0)
                local_options.fast = OPNG_OPTIM_LEVEL_FASTEST;
            else if (local_options.fast != OPNG_OPTIM_LEVEL_FASTEST)
                local_options.fast = OPNG_OPTIM_LEVEL_MIN - 1;  /* error */
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
            warning("The option -full is deprecated; enabling -paranoid");
            options.paranoid = 1;
        }
        else if (strncmp("keep", opt, opt_len) == 0)
        {
            /* -k | ... | -keep */
            warning("The option -keep is deprecated; enabling -backup");
            options.backup = 1;
        }
        else if (strncmp("no-clobber", opt, opt_len) == 0 && opt_len >= 5)
        {
            /* -no-cl | ... | -no-clobber */
            options.no_clobber = 1;
        }
        else if (strncmp("no-create", opt, opt_len) == 0 && opt_len >= 5)
        {
            /* -no-cr | ... | -no-create */
            options.no_create = 1;
        }
        else if (strncmp("nb", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -nb */
            options.nb = 1;
        }
        else if (strncmp("nc", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -nc */
            options.nc = 1;
        }
        else if (strncmp("nm", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -nm */
            options.nm = 1;
        }
        else if (strncmp("np", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -np */
            options.np = 1;
        }
        else if (strncmp("nx", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -nx */
            options.nb = options.nc = options.nm = options.np = 1;
        }
        else if (strncmp("nz", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -nz */
            options.nz = 1;
        }
        else if (strncmp("paranoid", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -pa | ... | -paranoid */
            options.paranoid = 1;
        }
        else if (strncmp("preserve", opt, opt_len) == 0 && opt_len >= 3)
        {
            /* -pre | ... | -preserve */
            options.preserve = 1;
        }
        else if ((strncmp("quiet", opt, opt_len) == 0) ||
                 (strncmp("silent", opt, opt_len) == 0 && opt_len >= 3))
        {
            /* -q | ... | -quiet | -sil | ... | -silent */
            local_options.quiet = 1;
        }
        else if (strncmp("simulate", opt, opt_len) == 0 && opt_len >= 3)
        {
            /* -sim | ... | -simulate */
#if 0       /* Don't deprecate -simulate just yet. */
            warning("The option -simulate is deprecated; enabling -no-create");
#endif
            options.no_create = 1;
        }
        else if (strncmp("snip", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -sn | ... | -snip */
            options.snip = 1;
        }
        else if (strncmp("stdin", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -stdi | -stdin */
            options.use_stdin = 1;
        }
        else if (strncmp("stdout", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -stdo | ... | -stdout */
            if (!options.use_stdout)
            {
                options.use_stdout = 1;
                local_options.out_name = "STDOUT";
                ++out_name_count;
            }
        }
        else if (strncmp("v", opt, opt_len) == 0)
        {
            /* -v */
            options.verbose = 1;
            local_options.version = 1;
        }
        else if (strncmp("verbose", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -verb | ... | -verbose */
            options.verbose = 1;
        }
        else if (strncmp("version", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -vers | ... | -version */
            local_options.version = 1;
        }
        else  /* possibly an option with an argument */
        {
            simple_opt = 0;
            if (xopt == NULL)
            {
                if (++i < argc)
                    xopt = argv[i];
                else
                    xopt = "";
            }
        }

        /* Check the options that require arguments. */
        if (simple_opt)
        {
            if (xopt != NULL)
                error(EX_USAGE, "No argument allowed for option: %s", arg);
        }
        else if (strncmp("dir", opt, opt_len) == 0)
        {
            /* -d PATH | ... | -dir PATH */
            if (local_options.dir_name != NULL)
                error(EX_USAGE,
                      "Multiple output directories are not permitted");
            if (xopt[0] == 0)
                error_option_arg("-dir", NULL);
            local_options.dir_name = xopt;
        }
        else if (strncmp("f", opt, opt_len) == 0)
        {
            /* -f RANGESET */
            set = check_rangeset_option("-f", xopt, OPNG_FILTER_SET_MASK);
            options.filter_set |= set;
        }
        else if (strncmp("i", opt, opt_len) == 0)
        {
            /* -i NUM */
            val = check_num_option("-i", xopt, 0, 1);
            if (options.interlace < 0)
                options.interlace = val;
            else if (options.interlace != val)
                error(EX_USAGE, "Multiple interlace types are not permitted");
        }
        else if (strncmp("log", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -lo PATH | -log PATH */
            error(EX_USAGE,
                  "The option -log is no longer supported; "
                  "use shell redirection");
        }
        else if (strncmp("o", opt, opt_len) == 0)
        {
            /* -o NUM */
            val = check_num_option("-o", xopt, 1, 99);
            if (options.optim_level == 0)
                options.optim_level = val;
            else if (options.optim_level != val)
                error(EX_USAGE,
                      "Multiple optimization levels are not permitted");
        }
        else if (strncmp("out", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -ou PATH | -out PATH */
            if (xopt[0] == 0)
                error_option_arg("-out", NULL);
            local_options.out_name = xopt;
            ++out_name_count;
        }
        else if (strncmp("protect", opt, opt_len) == 0 && opt_len >= 3)
        {
            /* -pro OBJECTS | ... | -protect OBJECTS */
            check_transform_option("-protect", xopt);
        }
        else if (strncmp("reset", opt, opt_len) == 0)
        {
            /* -r OBJECTS | ... | -reset OBJECTS */
            check_transform_option("-reset", xopt);
        }
        else if (strncmp("set", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -se OBJECT=VALUE | -set OBJECT=VALUE */
            check_transform_option("-set", xopt);
        }
        else if (strncmp("strip", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -st OBJECTS | ... | -strip OBJECTS */
            check_transform_option("-strip", xopt);
        }
        else if (strncmp("zc", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -zc RANGESET */
            set = check_rangeset_option("-zc", xopt, OPNG_ZCOMPR_LEVEL_SET_MASK);
            options.zcompr_level_set |= set;
        }
        else if (strncmp("zm", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -zm RANGESET */
            set = check_rangeset_option("-zm", xopt, OPNG_ZMEM_LEVEL_SET_MASK);
            options.zmem_level_set |= set;
        }
        else if (strncmp("zs", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -zs RANGESET */
            set = check_rangeset_option("-zs", xopt, OPNG_ZSTRATEGY_SET_MASK);
            options.zstrategy_set |= set;
        }
        else if (strncmp("zw", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -zw NUM */
            val = check_power2_option("-zw", xopt, 8, 15);
            if (options.zwindow_bits == 0)
                options.zwindow_bits = val;
            else if (options.zwindow_bits != val)
                error(EX_USAGE, "Multiple window sizes are not permitted");
        }
        else
        {
            error(EX_USAGE, "Unrecognized option: %s", arg);
        }
    }

    fnames = argv + i;
    fname_count = argc - i;

    if (local_options.out_name != NULL)
    {
        if (fname_count > 1)
            error(EX_USAGE,
                  "The options -out and -stdout require one input file");
        if (out_name_count > 1)
            error(EX_USAGE, "Multiple output files are not permitted");
        if (local_options.dir_name != NULL)
            error(EX_USAGE,
                  "The option -dir cannot be used with -out or -stdout");
        options.out = 1;
    }

    /* Console applications that read binary files from stdin should
     * issue a useful help screen if (!isatty(stdin)). Moreover, console
     * applications that write binary files to stdout should not display
     * garbage if (!isatty(stdout)).
     * Unfortunately, this is not generally the case... [ct]
     */
    if (fname_count == 0 && !local_options.help && !local_options.version)
    {
        if (optk_ftty(stdin) != 0)
        {
            /* stdin is either a terminal or unknown. */
            local_options.help = -1;  /* short help */
        }
        else
        {
            /* stdin is not a terminal. */
            options.use_stdin = 1;
        }
        /* optk_ftty(stdout) needs not be checked here.
         * Images can only be sent to stdout by explicit use of -stdout.
         */
    }
    if (options.use_stdin)
        error(EX_UNAVAILABLE,
              "Reading from STDIN is currently not supported");
    if (options.use_stdout && optk_ftty(stdout) > 0)
        error(EX_CANTCREAT,
              "Can't write binary file to the terminal output");

    postprocess_options();
}

/*
 * File processing
 */
static int
process_files(void)
{
    const char *fname;
    const char *out_name;
    int result;
    int i;

    if (opng_set_options(the_optimizer, &options) < 0)
    {
        /* This should not happen. Have we checked all the options? */
        warning("[BUG] Failed to detect a usage error");
        return EX_USAGE;
    }
    opng_set_transformer(the_optimizer, the_transformer);

    result = 0;
    for (i = 0; i < fname_count; ++i)
    {
        fname = fnames[i];
        out_name =
            (local_options.out_name != NULL) ? local_options.out_name : fname;
        if (opng_optimize_file(the_optimizer,
                               fname, out_name, local_options.dir_name) != 0)
        {
            /* Error(s) found but not fixed. */
            result = 2;
        }
    }
    return result;
}

/*
 * main
 */
int
main(int argc, char *argv[])
{
    int result;
    const struct opng_version_info *version_info_ptr;

    /* Initialize the program. */
    if (initialize() < 0)
        error(EX_UNAVAILABLE, "Can't start");
    result = 0;

    /* Parse the user options. */
    parse_args(argc, argv);

    if (local_options.version)
    {
        /* Print the copyright and version info. */
        printf("%s\n", msg_intro);
    }

    if (local_options.help)
    {
        if (local_options.help < 0)
        {
            /* Print the basic help text. */
            printf("%s%s%s%s",
                   msg_help_synopsis,
                   msg_help_basic_options,
                   msg_help_examples,
                   msg_help_more);
        }
        else
        {
            /* Print the extended help text. */
            printf("%s%s%s",
                   msg_help_synopsis,
                   msg_help_options,
                   msg_help_examples);
        }
    }
    else if (local_options.version && fname_count == 0)
    {
        /* Print the licensing terms and the extended version info. */
        for (version_info_ptr = opng_get_version_info();
             version_info_ptr->library_name != NULL;
             ++version_info_ptr)
        {
            printf("Using %s", version_info_ptr->library_name);
            if (version_info_ptr->library_version != NULL)
                printf(" version %s", version_info_ptr->library_version);
            printf("\n");
        }
        printf("\n%s", msg_license);
    }
    else
    {
        /* Execute the program. */
        result = process_files();
    }

    /* Finalize the program. */
    finalize();
    return result;
}
