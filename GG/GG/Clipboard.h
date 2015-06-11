#ifdef WIN32
    #include <GG/Clipboard_Windows.h>
#else
    // TODO: Include more platform specific clipboard implementations.
    #include <GG/Clipboard_Generic.h>
#endif
