#if defined(_MSC_VER) && defined(_DEBUG)

#include <crtdbg.h>

static class OPNG_Debug_State
{
public:
    OPNG_Debug_State();
   ~OPNG_Debug_State();
} debug_state;

inline OPNG_Debug_State::OPNG_Debug_State()
{
}

inline OPNG_Debug_State::~OPNG_Debug_State()
{
   _CrtDumpMemoryLeaks();
}

#endif  /* _DEBUG */
