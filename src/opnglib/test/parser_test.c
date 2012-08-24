/*
 * parser_test.c
 * Test for opngtrans/parser.h
 *
 * Copyright (C) 2011-2012 Cosmin Truta.
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

enum
{
    NONE  = 0,
    IDS   = 1,
    ID_EQ = 2
};

const struct
{
    const char *strlist;
    unsigned int expected_1st_offset;
    unsigned int expected_1st_length;
    unsigned int expected_value_offset;
    int expected_format;
    opng_id_t expected_ids;
} tests[] =
{
    { "",                     0,  0,  0, NONE,  OPNG_ID__NONE                },
    { ",;",                   0,  0,  0, NONE,  OPNG_ID__NONE                },
    { "all",                  0,  3,  0, IDS,   OPNG_ID_ALL                  },
    { "  all  ",              2,  3,  0, IDS,   OPNG_ID_ALL                  },
    { "all,all",              0,  3,  0, IDS,   OPNG_ID_ALL                  },
    { "all, all",             0,  3,  0, IDS,   OPNG_ID_ALL                  },
    { "all ;image.alpha",     0,  3,  0, IDS,   OPNG_ID_ALL | OPNG_ID_IMAGE_ALPHA },
    { " all ; image.alpha ",  1,  3,  0, IDS,   OPNG_ID_ALL | OPNG_ID_IMAGE_ALPHA },
    { "all ; ",               0,  3,  0, NONE,  OPNG_ID_ALL                  },
    { ";all",                 0,  0,  0, NONE,  OPNG_ID__NONE                },
    { "image.alpha",          0, 11,  0, IDS,   OPNG_ID_IMAGE_ALPHA          },
    { "image.blue.precision", 0, 20,  0, IDS,   OPNG_ID_IMAGE_BLUE_PRECISION },
    { "image.chroma",         0, 12,  0, IDS,   OPNG_ID_IMAGE_CHROMA_BT709   },
    { "image.chroma.bt601",   0, 18,  0, IDS,   OPNG_ID_IMAGE_CHROMA_BT601   },
    { "image.chroma.bt709",   0, 18,  0, IDS,   OPNG_ID_IMAGE_CHROMA_BT709   },
    { "PLTE",                 0,  4,  0, IDS,   OPNG_ID_CHUNK_IMAGE          },
    { "tRNS",                 0,  4,  0, IDS,   OPNG_ID_CHUNK_IMAGE          },
    { "acTL,fcTL,fdAT",       0,  4,  0, IDS,   OPNG_ID_CHUNK_ANIMATION      },
    { "tEXt,tEXt,iTXt",       0,  4,  0, IDS,   OPNG_ID_CHUNK_META           },
    { "tEXt.Comment",         0, 12,  0, IDS,   OPNG_ID__UNKNOWN             },
    { "tEXt=Comment",         0,  4,  5, ID_EQ, OPNG_ID_CHUNK_META           },
    { "tEXt =Comment",        0,  4,  6, ID_EQ, OPNG_ID_CHUNK_META           },
    { " tEXt=Comment",        1,  4,  6, ID_EQ, OPNG_ID_CHUNK_META           },
    { "tEXt/Comment",         0,  4,  0, NONE,  OPNG_ID_CHUNK_META           },
    { "UNKNOWN",              0,  7,  0, IDS,   OPNG_ID__UNKNOWN             },
    { "UN.KNOWN",             0,  8,  0, IDS,   OPNG_ID__UNKNOWN             },
    { ".IN.VALID",            0,  0,  0, NONE,  OPNG_ID__NONE                },
    { "IN.VALID.",            0,  0,  0, NONE,  OPNG_ID__NONE                },
    { "IN..VALID",            0,  0,  0, NONE,  OPNG_ID__NONE                },
    { NULL,                   0,  0,  0, NONE,  OPNG_ID__NONE                }
};

void
check_string_to_object(void)
{
    opng_id_t id;
    size_t name_offset, name_length;
    size_t i;

    printf("Testing opng_string_to_id...\n");
    for (i = 0; tests[i].strlist != NULL; ++i)
    {
        id = opng_string_to_id(tests[i].strlist, &name_offset, &name_length);
        if ((id & tests[i].expected_ids) != id ||
            name_offset != tests[i].expected_1st_offset ||
            name_length != tests[i].expected_1st_length)
        {
            printf("** FAIL: opng_string_to_id: \"%s\"\n", tests[i].strlist);
            printf("   result: 0x%x, not in: 0x%x\n",
                   id, tests[i].expected_ids);
            printf("   (offset, length): (%u, %u), expected: (%u, %u)\n",
                   (unsigned int)name_offset, (unsigned int)name_length,
                   tests[i].expected_1st_offset, tests[i].expected_1st_length);
            abort();
        }
    }
}

void
check_parse_objects(void)
{
    opng_id_t ids;
    int result, expected_result;
    struct opng_parse_err_info err_info;
    size_t i;

    printf("Testing opng_parse_objects...\n");
    for (i = 0; tests[i].strlist != NULL; ++i)
    {
        ids = 0;
        result = opng_parse_objects(&ids, NULL,
                                    tests[i].strlist, ~0, &err_info);
        expected_result = (tests[i].expected_format == IDS) ? 0 : -1;
        if (result != expected_result || ids != tests[i].expected_ids)
        {
            printf("** FAIL: opng_parse_objects: \"%s\"\n", tests[i].strlist);
            printf("   ids: 0x%x, expected: 0x%x\n", ids, tests[i].expected_ids);
            printf("   result: %d, expected: %d\n", result, expected_result);
            abort();
        }
    }
}

void
check_parse_object_value(void)
{
    opng_id_t id;
    size_t value_offset;
    int result, expected_result;
    struct opng_parse_err_info err_info;
    size_t i;

    printf("Testing opng_parse_object_value...\n");
    for (i = 0; tests[i].strlist != NULL; ++i)
    {
        result = opng_parse_object_value(&id, &value_offset,
                                         tests[i].strlist, ~0, &err_info);
        if (tests[i].expected_format == ID_EQ)
            expected_result = 0;
        else
        {
            value_offset = 0;
            result = expected_result = -1;
        }
        if (result != expected_result ||
            (id & tests[i].expected_ids) != id ||
            value_offset != tests[i].expected_value_offset)
        {
            printf("** FAIL: opng_parse_object_value: \"%s\"\n", tests[i].strlist);
            printf("   id: 0x%x, expected: 0x%x\n", id, tests[i].expected_ids);
            printf("   value offset: %d, expected: %d\n",
                   value_offset, tests[i].expected_value_offset);
            printf("   result: %d, expected: %d\n", result, expected_result);
            abort();
        }
    }
}

int
main()
{
    check_string_to_object();
    check_parse_objects();
    check_parse_object_value();
    return 0;
}
