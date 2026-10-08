#ifndef _GG_Utils_h_
#define _GG_Utils_h_

#include "Export.h"

#include <filesystem>
#include <string_view>
#include <string>

#if !defined(_WIN32)

namespace GG {
    inline std::filesystem::path StringToPath(std::string_view path_str)
    { return std::filesystem::path(path_str); }

    inline std::string PathToString(const std::filesystem::path& path)
    { return path.generic_string(); }
}

#else
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>

namespace GG {
    inline std::filesystem::path StringToPath(std::string_view path_str)
    {
        // convert UTF-8 string to UTF-16
        int utf16_sz = MultiByteToWideChar(CP_UTF8, 0, path_str.data(), path_str.length(), NULL, 0);
        std::wstring utf16_string(utf16_sz, 0);
        if (utf16_sz > 0)
            MultiByteToWideChar(CP_UTF8, 0, path_str.data(), path_str.size(), utf16_string.data(), utf16_sz);
        static_assert(std::is_same_v<std::filesystem::path::string_type, std::wstring>);
        return std::filesystem::path(utf16_string);
    }

    inline std::string PathToString(const std::filesystem::path& path)
    {
        auto native_string = path.generic_wstring();
        // convert UTF-16 native path to UTF-8
        int utf8_sz = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                          native_string.data(), native_string.size(),
                                          NULL, 0, NULL, NULL);
        std::string utf8_string(utf8_sz, 0);
        if (utf8_sz > 0)
            WideCharToMultiByte(CP_UTF8, 0, native_string.data(), native_string.size(),
                                utf8_string.data(), utf8_sz, NULL, NULL);
        return utf8_string;
    }
}
#endif
#endif