/*
 * tiff2pnm.c
 * A test program for minitiff.
 *
 * Copyright (C) 2006-2017 Cosmin Truta.
 *
 * minitiff is open-source software, distributed under the zlib license.
 * For conditions of distribution and use, see copyright notice in minitiff.h.
 */

#include "minitiff.h"

#include <stdio.h>
#include <stdlib.h>


static int tiff2pnm(const char *in_path, const char *out_path)
{
    FILE *in_stream;
    FILE *out_stream;
    struct minitiff_info info;
    size_t width, height, depth, y;
    unsigned char *row;
    int ioerr;

    in_stream = fopen(in_path, "rb");
    if (in_stream == NULL)
    {
        fprintf(stderr, "error: Can't open input TIFF file: %s\n", in_path);
        return -1;
    }

    minitiff_init_info(&info);
    minitiff_read_info(&info, in_stream);
    minitiff_validate_info(&info);

    width = info.width;
    height = info.height;
    depth = info.samples_per_pixel;
    if (depth != 1 && depth != 3)
    {
        fprintf(stderr,
                "error: Invalid number of color planes in TIFF files: %lu\n",
                (unsigned long)depth);
        minitiff_destroy_info(&info);
        fclose(in_stream);
        return -1;
    }

    row = (unsigned char *)malloc(depth * width);
    if (row == NULL)
    {
        fprintf(stderr, "critical error: Out of memory\n");
        minitiff_destroy_info(&info);
        fclose(in_stream);
        exit(EXIT_FAILURE);
    }

    out_stream = fopen(out_path, "wb");
    if (out_stream != NULL)
    {
        ioerr = 0;
        fprintf(out_stream,
                "P%c\n%lu %lu\n255\n",
                (depth == 1) ? '5' : '6',
                (unsigned long)width, (unsigned long)height);
        for (y = 0; y < height; ++y)
        {
            minitiff_read_row(&info, row, y, in_stream);
            fwrite(row, depth, width, out_stream);
        }
        if (ferror(in_stream))
        {
            ioerr = 1;
            fprintf(stderr,
                    "error: Can't read input TIFF file: %s\n", in_path);
        }
        if (ferror(out_stream))
        {
            ioerr = 1;
            fprintf(stderr,
                    "error: Can't write output PNM file: %s\n", out_path);
        }
        fclose(out_stream);
    }
    else
    {
        ioerr = 1;
        fprintf(stderr, "error: Can't open output PNM file: %s\n", out_path);
    }

    minitiff_destroy_info(&info);
    fclose(in_stream);
    fclose(out_stream);
    return ioerr ? -1 : 1;
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        fprintf(stderr, "usage: tiff2pnm input.tif output.pnm\n");
        return EXIT_FAILURE;
    }
    return (tiff2pnm(argv[1], argv[2]) >= 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
