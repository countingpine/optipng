Name: zlib
Summary: A general-purpose data compression library
Authors: Jean-loup Gailly and Mark Adler
Version: 1.2.11
License: zlib
URL: http://zlib.net/

Modified version: 1.2.11-optipng
Modifications:
- Defined NO_GZCOMPRESS and NO_GZIP to compile out the gzip-processing code.
- Set TOO_FAR to the largest possible value to increase the probability of
  producing better-compressed deflate streams.
- Cherry-picked a Cygwin build fix from upstream.
- Changed ZLIB_VERSION to "1.2.11-optipng" and ZLIB_VERNUM to 0x12bf.
