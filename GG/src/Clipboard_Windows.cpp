#ifdef WIN32

#include <GG/Clipboard_Windows.h>

#include <Windows.h>
#include <locale>
#include <codecvt>


namespace GG {
    Clipboard::Clipboard() {
        m_clipboard_owner = (OpenClipboard(nullptr) != 0);
    }

    Clipboard::Clipboard(Clipboard&& clipboard) {
        // Not thread-safe.  Shouldn't be an issue in practice.
        m_clipboard_owner = clipboard.m_clipboard_owner;
        clipboard.m_clipboard_owner = false;
    }

    Clipboard::~Clipboard() {
        if (m_clipboard_owner) CloseClipboard();
    }

    std::string Clipboard::Text() {
        if (!UnicodeAvailable()) return "";

        HGLOBAL data = GetClipboardData(CF_UNICODETEXT);
        if (data == 0) return "";

        std::u16string utf16_text((char16_t*)GlobalLock(data));
        GlobalUnlock(data);

        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
        std::string text = conversion.to_bytes(utf16_text);

        return text;
    }

    bool Clipboard::SetText(const std::string& text) {
        EmptyClipboard();

        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
        std::u16string utf16_text = conversion.from_bytes(text);
        
        HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE, (utf16_text.size() + 1) * 2);
        if (data == 0) return false;

        memcpy(GlobalLock(data), utf16_text.c_str(), (utf16_text.size() + 1) * 2);
        GlobalUnlock(data);

        if (!SetClipboardData(CF_UNICODETEXT, data)) {
            GlobalFree(data);
            return false;
        }

        return true;
    }

    bool Clipboard::DataAvailable(unsigned format) {
        UINT result = 0;
        do {
            result = EnumClipboardFormats(result);
            if (result == format) return true;
        } while (result != 0);

        return false;
    }

    bool Clipboard::UnicodeAvailable() {
        return DataAvailable(CF_UNICODETEXT);
    }

    Clipboard GetClipboard() {
        return Clipboard();
    }
}

#endif
