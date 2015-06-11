#ifndef WIN32

#include <GG/Clipboard_Generic.h>

namespace {
    static std::string clipboard_text;
}

namespace GG {
    std::string Clipboard::Text()
    { return clipboard_text; }

    bool Clipboard::SetText(const std::string& text) {
        clipboard_text = text;
        return true;
    }

    Clipboard GetClipboard() {
        return Clipboard();
    }
}

#endif
