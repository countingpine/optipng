/*
 * parser_test.c
 * Test for opngtrans/parser.h
 *
 * Copyright (C) 2011 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#define OPNGLIB_INTERNAL
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const struct
{
    const char *strlist;
    unsigned int expected_offset;
    unsigned int expected_length;
    opng_id_t expected_id;
} tests[] =
{
    { "",                   0,  0, OPNG_ID_ERR_SYNTAX         },
    { ",;",                 0,  0, OPNG_ID_ERR_SYNTAX         },
    { "all",                0,  3, OPNG_ID_ALL                },
    { "  all  ",            2,  3, OPNG_ID_ALL                },
    { "all, ",              0,  3, OPNG_ID_ALL                },
    { "all ; ",             0,  3, OPNG_ID_ALL                },
    { ",all",               0,  0, OPNG_ID_ERR_SYNTAX         },
    { "image.alpha",        0, 11, OPNG_ID_IMAGE_ALPHA        },
    { "alpha",              0,  5, OPNG_ID_IMAGE_ALPHA        },
    { "image.chroma",       0, 12, OPNG_ID_IMAGE_CHROMA_BT709 },
    { "image.chroma.bt601", 0, 18, OPNG_ID_IMAGE_CHROMA_BT601 },
    { "image.chroma.bt709", 0, 18, OPNG_ID_IMAGE_CHROMA_BT709 },
    { "PLTE",               0,  4, OPNG_ID_CHUNK_IMAGE        },
    { "tRNS",               0,  4, OPNG_ID_CHUNK_IMAGE        },
    { "tEXt,tEXt,iTXt",     0,  4, OPNG_ID_CHUNK_META         },
    { "ima",                0,  3, OPNG_ID_ERR_UNKNOWN        },
    { "imag",               0,  4, OPNG_ID_CHUNK_META         },
    { "image",              0,  5, OPNG_ID_IMAGE              },
    { "image.",             0,  0, OPNG_ID_ERR_SYNTAX         },
    { "image.m",            0,  7, OPNG_ID_ERR_UNKNOWN        },
    { "UNKNOWN",            0,  7, OPNG_ID_ERR_UNKNOWN        },
    { "UNKNOWN.OBJECT",     0, 14, OPNG_ID_ERR_UNKNOWN        },
    { ".IN.VALID",          0,  0, OPNG_ID_ERR_SYNTAX         },
    { "IN.VALID.",          0,  0, OPNG_ID_ERR_SYNTAX         },
    { "IN..VALID",          0,  0, OPNG_ID_ERR_SYNTAX         },
    { NULL,                 0,  0, 0                          }
};

void
check_string_to_object(void)
{
    size_t name_offset, name_length;
    size_t i;
    unsigned int result;

    for (i = 0; tests[i].strlist != NULL; ++i)
    {
        result = opng_string_to_id(tests[i].strlist, &name_offset, &name_length);
        if (result != tests[i].expected_id ||
            name_offset != tests[i].expected_offset ||
            name_length != tests[i].expected_length)
        {
            printf("** FAIL: check_string_to_object: %s\n", tests[i].strlist);
            printf("   result: %u; expected: %u\n",
                   result, tests[i].expected_id);
            printf("   (offset, length): (%u, %u); expected: (%u, %u)\n",
                   (unsigned int)name_offset, (unsigned int)name_length,
                   tests[i].expected_offset, tests[i].expected_length);
            abort();
        }
    }
}

int
main()
{
    check_string_to_object();
    /* TODO: check_parse_object_value(); */
    /* TODO: check_parse_objects(); */
    return 0;
}
