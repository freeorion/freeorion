#ifndef CLIPBOARD_GENERIC_H
#define CLIPBOARD_GENERIC_H

#include <string>

namespace GG {

    /** Non-platform specific clipboard class.  Uses an application-specific
      * clipboard instead of the system clipboard. */
    class Clipboard {
    public:
        std::string Text();
        bool SetText(const std::string&);

    private:
        friend Clipboard GetClipboard();
        Clipboard() = default;
    };

    Clipboard GetClipboard();
}

#endif
