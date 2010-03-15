
OptiPNG 0.6.4: Advanced PNG optimizer.
Copyright (C) 2001-2010 Cosmin Truta.
======================================

Goals
-----
  Provide a powerful PNG converter/optimizer.
  Demonstrate how to implement an efficient PNG encoder.

License
-------
  Open-Source (zlib/libpng license).
  Please see the attached LICENSE.

URI
---
  Home page:
    http://optipng.sourceforge.net/
  Download:
    http://sourceforge.net/project/showfiles.php?group_id=151404
  Support:
    http://sourceforge.net/tracker/?group_id=151404

Build instructions
------------------
  Extract the source archive:
    e.g. "tar -xzf optipng-0.6.4.tar.gz" or "unzip optipng-0.6.4.zip"

  Go to the source directory:
    "cd optipng-0.6.4/src/"

  Run the appropriate makefile from the scripts directory:
    e.g. "make -f scripts/gcc.mak" or "nmake -f scripts/visualc.mak"
  OR
  If you are using Microsoft Visual C++ 6.0 or later,
    load and build the project "prj/visualc6/optipng.dsw"

Installation instructions
-------------------------
  Follow the build instructions above.

  On Unix:
    Make the "install" target, using the same makefile
    that was used for building the program:
      e.g. "make -f scripts/gcc.mak install"
    To uninstall, make the "uninstall" target:
      e.g. "make -f scripts/gcc.mak uninstall"

  On Windows:
    Copy "optipng.exe" to a directory found in PATH.

Support
-------
  Visit the SourceForge.net tracker:
    http://sourceforge.net/tracker/?group_id=151404
  OR
  Send email to ctruta (at) gmail (dot) com

