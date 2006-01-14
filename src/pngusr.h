/*
 * pngusr.h -- libpng configuration file for OptiPNG.
 *
 * This file is #included by pngconf.h only if PNG_USER_CONFIG is #defined.
 * In this case, it *must* be #included when compiling libpng *and* OptiPNG.
 */


#ifndef PNGUSR_H
#define PNGUSR_H


/*
 * PNG_ZBUF_SIZE is not important to OptiPNG,
 * but a bigger value may improve the speed.
 */
#define PNG_ZBUF_SIZE 0x10000

/*
 * Remove the libpng features that are not needed by OptiPNG.
 */
#define PNG_NO_ERROR_NUMBERS
#define PNG_NO_FLOATING_POINT_SUPPORTED
#define PNG_NO_LEGACY_SUPPORTED
#define PNG_NO_MNG_FEATURES
#define PNG_NO_PROGRESSIVE_READ
#define PNG_NO_SETJMP_SUPPORTED
#define PNG_NO_READ_TRANSFORMS
#define PNG_NO_WRITE_TRANSFORMS
#define PNG_NO_USER_MEM


#endif	/* PNGUSR_H */
