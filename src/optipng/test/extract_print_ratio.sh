#!/bin/sh

set -e

testdir=$(dirname "$0")
srcfile="$testdir/../opngoptim.c"

header='/*\
 * *print_ratio.generated.c\
 * Generated from opngoptim.c\
 *\
 * Copyright (C) 2008-2011 Cosmin Truta.\
 *\
 * This software is distributed under the zlib license.\
 * Please see the attached LICENSE for more information.\
 */\
\
#include "print_ratio.h"\
#include <stdio.h>\
'

snprintf_hack='#ifdef _MSC_VER\
#define snprintf _snprintf\
#endif\
'

extract_fprint_ratio_script="
1 i\\
$header
/return/ d
s/usr_printf(/return fprintf(stream, /
/^opng_print_ratio/ i\\
int
s/^opng_print_ratio(/fprint_ratio(FILE *stream, /
/^fprint_ratio/,/^}/ p
"

extract_sprint_ratio_script="
1 i\\
$header
1 i\\
$snprintf_hack
/return/ d
s/usr_printf(/return snprintf(buf, bufsize, /
/^opng_print_ratio/ i\\
int
s/^opng_print_ratio(/sprint_ratio(char *buf, size_t bufsize, /
/^sprint_ratio/,/^}/ p
"

sed -n "$extract_fprint_ratio_script" "$srcfile" > fprint_ratio.generated.c
sed -n "$extract_sprint_ratio_script" "$srcfile" > sprint_ratio.generated.c
