
OptiPNG 0.6.5: Advanced PNG optimizer.
Copyright (C) 2001-2011 Cosmin Truta.
======================================

License
-------
  Open-Source (the zlib license)
  See the accompanying LICENSE.txt

Resources
---------
  Home page:
    http://optipng.sourceforge.net/
  Download:
    http://sourceforge.net/project/showfiles.php?group_id=151404
  Announcements:
    https://sourceforge.net/news/?group_id=151404
  Support:
    http://sourceforge.net/tracker/?group_id=151404
    ctruta (at) gmail (dot) com

Build instructions
------------------
  Extract the source archive:
        tar -xzf optipng-0.6.5.tar.gz   # or:
        unzip optipng-0.6.5.zip

  On Unix:
  - Run configure and make:
        cd optipng-0.6.5/
        ./configure && make

  On Windows, using Microsoft Visual C++ 6.0 or later:
  - Load and build the project:
        optipng-0.6.5/prj/visualc6/optipng.dsw
  - The output directory is:
        optipng-0.6.5/bin/

  Alternatively, use the applicable makefile as follows:
        cd optipng-0.6.5/src/
        make -f scripts/bcc32.mak       # or:
        make -f scripts/gcc.mak         # or:
        nmake -f scripts/visualc.mak    # etc.

Installation instructions
-------------------------
  Build the program according to the instructions above.

  On Unix:
  - Make the "install" target:
        sudo make install
  - To uninstall, make the "uninstall" target:
        sudo make uninstall

  On Windows:
    Copy optipng.exe to a directory found in PATH.

