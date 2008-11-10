/**
 ** optipng.h
 ** OptiPNG programming interface.
 **
 ** Copyright (C) 2001-2008 Cosmin Truta.
 ** OptiPNG is open-source software, and is distributed under the same
 ** licensing and warranty terms as libpng.
 **/


#ifndef OPTIPNG_H
#define OPTIPNG_H

#include "cbitset.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Program options (e.g. extracted from the command line)
 */
struct opng_options
{
    int help;
    int ver;
    int optim_level;
    int interlace;
    int keep, quiet;
    int nb, nc, np, nz;
    int fix;
    int force;
    int full;
    int preserve;
    int simulate;
    int snip;
    bitset_t compr_level_set;
    bitset_t mem_level_set;
    bitset_t strategy_set;
    bitset_t filter_set;
    int window_bits;
    char *out_name;
    char *dir_name;
    char *log_name;
};


/*
 * Program UI callbacks
 */
struct opng_ui
{
    void (*printf_fn)(const char *fmt, ...);
    void (*flush_fn)(void);
    void (*progress_fn)(unsigned long num, unsigned long denom);
    void (*panic_fn)(const char *msg);
};


/*
 * Engine initialization
 */
int opng_initialize(const struct opng_options *options,
                    const struct opng_ui *ui);


/*
 * Engine execution
 */
int opng_optimize(const char *infile_name);


/*
 * Engine finalization
 */
int opng_finalize(void);


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif  /* OPTIPNG_H */
