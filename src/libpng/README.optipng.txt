Name: libpng
Summary: The PNG reference library
Authors: Glenn Randers-Pehrson et al.
Version: 1.6.10
License: the libpng license (zlib-like); see LICENSE
URL: http://libpng.org/

Modified version: 1.6.10-optipng
Modifications:
- Added pnglibconf.h.optipng to allow a leaner and meaner libpng build.
- Added PNGLIBCONF_H_PREBUILT to scripts/makefile.* to allow the use of
  user-supplied prebuilt config headers (such as pnglibconf.h.optipng).
- Silenced "unused parameter" build warnings.
