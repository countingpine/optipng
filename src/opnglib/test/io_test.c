/*
 * io_test.c
 * Test for optk/io.h
 *
 * Copyright (C) 2010-2012 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "optk/io.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


void
print_substr(const char *str, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
        putc(str[i], stdout);
    putc('\n', stdout);
}

void
check_paths(void)
{
    const char *path = "/a/b/c.d.e";
    char *str;
    size_t len;

    printf("path = %s\n", path);

    printf("optk_get_basename: ");
    str = optk_get_basename(path, &len);
    print_substr(str, len);
    assert(len == 5 && memcmp(str, "c.d.e", len) == 0);

    printf("optk_get_dirname (with_dir_separator = 0): ");
    str = optk_get_dirname(path, 0, &len);
    print_substr(str, len);
    assert(len == 4 && memcmp(str, "/a/b", len) == 0);

    printf("optk_get_dirname (with_dir_separator = 1): ");
    str = optk_get_dirname(path, 1, &len);
    print_substr(str, len);
    assert(len == 5 && memcmp(str, "/a/b/", len) == 0);

    printf("optk_get_extname (with_ext_separator = 0, num_ext = 0): ");
    str = optk_get_extname(path, 0, 0, &len);
    print_substr(str, len);
    assert(len == 0);

    printf("optk_get_extname (with_ext_separator = 0, num_ext = 1): ");
    str = optk_get_extname(path, 0, 1, &len);
    print_substr(str, len);
    assert(len == 1 && memcmp(str, "e", len) == 0);

    printf("optk_get_extname (with_ext_separator = 0, num_ext = 2): ");
    str = optk_get_extname(path, 0, 2, &len);
    print_substr(str, len);
    assert(len == 3 && memcmp(str, "d.e", len) == 0);

    printf("optk_get_extname (with_ext_separator = 1, num_ext = -1): ");
    str = optk_get_extname(path, 1, -1, &len);
    print_substr(str, len);
    assert(len == 4 && memcmp(str, ".d.e", len) == 0);

    /* TODO: Add more path tests. */
}

void
check_files(const char *path1, const char *path2)
{
    if (path2 != NULL)
    {
        printf("path1 = %s\n", path1);
        printf("path2 = %s\n", path2);
    }
    else
    {
        printf("path = %s\n", path1);
        path2 = path1;
    }

    printf("optk_test (mode = \"e\"): %d\n", optk_test(path1, "e"));
    printf("optk_test (mode = \"f\"): %d\n", optk_test(path1, "f"));
    printf("optk_test (mode = \"d\"): %d\n", optk_test(path1, "d"));
    printf("optk_test (mode = \"r\"): %d\n", optk_test(path1, "r"));
    printf("optk_test (mode = \"w\"): %d\n", optk_test(path1, "w"));
    printf("optk_test (mode = \"x\"): %d\n", optk_test(path1, "x"));
    printf("optk_test (mode = \"\"): %d\n", optk_test(path1, "??"));
    printf("optk_test (mode = \"??\"): %d\n", optk_test(path1, "??"));

    printf("optk_test_dir: %d\n", optk_test_dir(path1));

    printf("optk_test_eq: %d\n", optk_test_eq(path1, path2));

    /* TODO: Add more file tests. */
}

int
main(int argc, char *argv[])
{
    printf("Testing path operations...\n");
    check_paths();

    if (argc >= 2)
    {
        printf("\nTesting file operations...\n");
        check_files(argv[1], argv[2]);  /* argv[2] may be NULL */
    }

    return 0;
}
