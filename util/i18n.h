#ifndef _I18N_h_
#define _I18N_h_

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>

#include "StringTable.h"

#include "Export.h"

#if !defined(FREEORION_ANDROID)
/** Returns locale, which may be previously cached */
[[nodiscard]] FO_COMMON_API const std::locale& GetLocale(std::string_view name = "en_US.UTF-8");
#endif

/** Returns a language-specific string for the key-string \a str */
[[nodiscard]] FO_COMMON_API const std::string& UserString(const std::string& str);
[[nodiscard]] FO_COMMON_API const std::string& UserString(const std::string_view str);
[[nodiscard]] FO_COMMON_API const std::string& UserString(const char* str);

/** Returns all entries in current stringtable */
using AllStringsResultT = decltype(std::declval<StringTable>().AllStrings());
static_assert(std::is_const_v<std::remove_reference_t<AllStringsResultT>>);
[[nodiscard]] FO_COMMON_API AllStringsResultT& AllStringtableEntries(bool default_table = false);

/** Returns a language-specific vector of strings for given @a key. */
[[nodiscard]] FO_COMMON_API std::vector<std::string> UserStringList(const std::string& key);

/** Returns true iff a user-string exists for the key string \a str */
[[nodiscard]] FO_COMMON_API bool UserStringExists(const std::string& str);
[[nodiscard]] FO_COMMON_API bool UserStringExists(const std::string_view str);
[[nodiscard]] FO_COMMON_API bool UserStringExists(const char* str);

/** Clears all loaded strings, so that subsequent UserString lookups will cause
  * the stringtable(s) to be reloaded. */
FO_COMMON_API void FlushLoadedStringTables();

/** Placeholder for non local translations, evaluates to a non operation */
#define UserStringNop(key) key

/** Returns the language of the StringTable currently in use */
[[nodiscard]] const std::string& Language();

/** Wraps boost::format such that it won't crash if passed the wrong number of arguments */
[[nodiscard]] FO_COMMON_API boost::format FlexibleFormat(const std::string& string_to_format);

/** Returns the stringified form of \a n as a roman number.  "Only" defined for 1 <= n <= 3999,
  * as we can't display the symbol for 5000. */
[[nodiscard]] FO_COMMON_API std::string RomanNumber(uint16_t n);
[[nodiscard]] inline std::string RomanNumber(auto n) {
    if constexpr (std::is_signed_v<decltype(n)>)
        n = (n >= 0 ? n : -n);
    return RomanNumber(static_cast<uint16_t>(n));
}

/** Converts double to string with \a digits digits.
  * Represents large numbers with SI prefixes. */
[[nodiscard]] FO_COMMON_API std::string DoubleToString(double val, uint8_t digits, bool always_show_sign);

/** Returns sign of value, accounting for SMALL_UI_DISPLAY_VALUE: +1 for
  * positive values and -1 for negative values if their absolute value is
  * larger than SMALL VALUE, and returns 0 for zero values or values with
  * absolute value less than SMALL_UI_DISPLAY_VALUE */
[[nodiscard]] FO_COMMON_API int EffectiveSign(double val);

/** returns a boost::format pre-filled with a list of up to 10 items with some introductory text similar to
  * These are 3 fruit: apples, pears and bananas.
  * The Container types need to support size(), begin() and end().
  * The headers are boost::format strings fed the size of the words list and then any header words passed in.
  * TODO: Adopt an out of house i18n framework and handle of empty, singular, pairs, triplets etc. correctly. */
template<typename HeaderContainer, typename ListContainer>
[[nodiscard]] boost::format FlexibleFormatList(
    const HeaderContainer& header_words,
    const ListContainer& words,
    const std::string& plural_header_template,
    const std::string& single_header_template,
    const std::string& empty_header_template,
    const std::string& dual_header_template)
{
    auto& header_template = [&, sz{words.size()}]() -> auto& {
        switch (sz) {
        case 0: return empty_header_template; break;
        case 1: return single_header_template; break;
        case 2: return dual_header_template; break;
        default: return plural_header_template; break;
        }
    }();
    boost::format header_fmt = FlexibleFormat(header_template) % std::to_string(words.size());
    for (const auto& word : header_words)
        header_fmt % word;

    const auto& template_str{UserString([sz{words.size()}]() {
        switch (sz) {
        case 0: return UserStringNop("FORMAT_LIST_0_ITEMS"); break;
        case 1: return UserStringNop("FORMAT_LIST_1_ITEMS"); break;
        case 2: return UserStringNop("FORMAT_LIST_2_ITEMS"); break;
        case 3: return UserStringNop("FORMAT_LIST_3_ITEMS"); break;
        case 4: return UserStringNop("FORMAT_LIST_4_ITEMS"); break;
        case 5: return UserStringNop("FORMAT_LIST_5_ITEMS"); break;
        case 6: return UserStringNop("FORMAT_LIST_6_ITEMS"); break;
        case 7: return UserStringNop("FORMAT_LIST_7_ITEMS"); break;
        case 8: return UserStringNop("FORMAT_LIST_8_ITEMS"); break;
        case 9: return UserStringNop("FORMAT_LIST_9_ITEMS"); break;
        case 10:return UserStringNop("FORMAT_LIST_10_ITEMS"); break;
        default:return UserStringNop("FORMAT_LIST_MANY_ITEMS"); break;
        }
    }())};
    boost::format fmt = FlexibleFormat(template_str) % header_fmt.str();
    for (const auto& word : words)
        fmt % word;

    return fmt;
}

template<typename Container>
[[nodiscard]] boost::format FlexibleFormatList(const Container& words)
{
    return FlexibleFormatList(std::vector<std::string>(), words,
                              UserString("FORMAT_LIST_DEFAULT_PLURAL_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_SINGLE_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_EMPTY_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_DUAL_HEADER"));
}

template<typename Container>
[[nodiscard]] boost::format FlexibleFormatList(
    const Container& words, const std::string& all_header)
{
    return FlexibleFormatList(std::vector<std::string>(), words, all_header, all_header, all_header, all_header);
}

template<typename Container>
[[nodiscard]] boost::format FlexibleFormatList(
    const Container& words, const std::string& plural_header, const std::string& single_header)
{
    return FlexibleFormatList(std::vector<std::string>(), words, plural_header, single_header, plural_header, plural_header);
}

template<typename Container>
[[nodiscard]] boost::format FlexibleFormatList(
    const Container& words, const std::string& plural_header,
    const std::string& single_header, const std::string& empty_header)
{
    return FlexibleFormatList(std::vector<std::string>(), words, plural_header, single_header, empty_header, plural_header);
}

template<typename T1, typename T2>
[[nodiscard]] boost::format FlexibleFormatList(const T2& header_words, const T1& words)
{
    return FlexibleFormatList(header_words, words,
                              UserString("FORMAT_LIST_DEFAULT_PLURAL_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_SINGLE_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_EMPTY_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_DUAL_HEADER"));
}

template<typename T1, typename T2>
[[nodiscard]] boost::format FlexibleFormatList(
    const T2& header_words, const T1& words, const std::string& all_header)
{
    return FlexibleFormatList(header_words, words, all_header, all_header, all_header, all_header);
}

template<typename T1, typename T2>
[[nodiscard]] boost::format FlexibleFormatList(
    const T2& header_words, const T1& words, const std::string& plural_header, const std::string& single_header)
{
    return FlexibleFormatList(header_words, words, plural_header, single_header, plural_header, plural_header);
}

template<typename T1, typename T2>
[[nodiscard]] boost::format FlexibleFormatList(
    const T2& header_words, const T1& words, const std::string& plural_header,
    const std::string& single_header, const std::string& empty_header)
{
    return FlexibleFormatList(header_words, words, plural_header, single_header, empty_header, plural_header);
}


#endif
