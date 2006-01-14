/**
 ** memleaks.cpp
 ** Automatic detection of memory leaks.
 ** This module works inside the Microsoft Visual C++ IDE.
 **
 ** NOT COPYRIGHTED by Cosmin Truta, 2004-2005.
 **/

#if defined(_MSC_VER) && defined(_DEBUG)

#include <crtdbg.h>

static struct MemLeaksDetector
{
    MemLeaksDetector() {}
   ~MemLeaksDetector() { _CrtDumpMemoryLeaks(); }
} memLeaksDetector;

#endif  /* _MSC_VER && _DEBUG */
