#ifndef CLIPBOARD_WINDOWS_H
#define CLIPBOARD_WINDOWS_H

#include <string>

namespace GG {

    class Clipboard {
    public:
        Clipboard(Clipboard&&);
        ~Clipboard();

        std::string Text();
        bool SetText(const std::string&);

    private:
        friend Clipboard GetClipboard();
        Clipboard();

        bool DataAvailable(unsigned format);
        bool UnicodeAvailable();
        bool m_clipboard_owner;
    };

    Clipboard GetClipboard();
}

#endif
