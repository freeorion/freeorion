#ifndef _I18N_h_
#define _I18N_h_

#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "Export.h"

/** Returns locale, which may be previously cached */
FO_COMMON_API std::locale GetLocale(const std::string& name = std::string(""));

/** Returns a language-specific string for the key-string \a str */
FO_COMMON_API const std::string& UserString(const std::string& str);

/** Returns a language-specific vector of strings for given @a key. */
FO_COMMON_API std::vector<std::string> UserStringList(const std::string& key);

/** Returns true iff a user-string exists for the key string \a str */
FO_COMMON_API bool UserStringExists(const std::string& str);

/** Clears all loaded strings, so that subsequent UserString lookups will cause
  * the stringtable(s) to be reloaded. */
FO_COMMON_API void FlushLoadedStringTables();

/** Placeholder for non local translations, evaluates to a non operation */
#define UserStringNop(key) key

/** Returns the language of the StringTable currently in use */
const std::string& Language();

/** Wraps boost::format such that it won't crash if passed the wrong number of arguments */
FO_COMMON_API boost::format FlexibleFormat(const std::string& string_to_format);

/** Returns the stringified form of \a n as a roman number.  "Only" defined for 1 <= n <= 3999, as we can't display the
    symbol for 5000. */
FO_COMMON_API std::string RomanNumber(unsigned int n);

/** Converts double to string with \a digits digits.  Represents large numbers
  * with SI prefixes. */
FO_COMMON_API std::string DoubleToString(double val, int digits, bool always_show_sign);

/** Returns sign of value, accounting for SMALL_UI_DISPLAY_VALUE: +1 for
  * positive values and -1 for negative values if their absolute value is
  * larger than SMALL VALUE, and returns 0 for zero values or values with
  * absolute value less than SMALL_UI_DISPLAY_VALUE */
FO_COMMON_API int EffectiveSign(double val);

/** returns a boost::format pre-filled with a list of up to 10 items with some introductory text similar to
 These are 3 fruit: apples, pears and bananas.
 The Container types need to support size(), begin() and end().
 The headers are boost::format strings fed the size of the words list and then any header words passed in.
 TODO: Adopt an out of house i18n framework and handle of empty, singular, pairs, triplets etc. correctly. */

template<typename HeaderContainer, typename ListContainer>
boost::format FlexibleFormatList(
    const HeaderContainer& header_words,
    const ListContainer& words,
    const std::string& plural_header_template,
    const std::string& single_header_template,
    const std::string& empty_header_template,
    const std::string& dual_header_template )
{
    std::string header_template;
    switch (words.size()) {
    case 0:
        header_template = empty_header_template;
        break;
    case 1:
        header_template = single_header_template;
        break;
    case 2:
        header_template = dual_header_template;
        break;
    default:
        header_template = plural_header_template;
    }

    boost::format header_fmt = FlexibleFormat(header_template) % boost::lexical_cast<std::string>(words.size());

    for (const auto& word : header_words) {
        header_fmt % word;
    }

    std::string template_str;

    switch (words.size()) {
    case 0:
        template_str = UserString("FORMAT_LIST_0_ITEMS");
        break;
    case 1:
        template_str = UserString("FORMAT_LIST_1_ITEMS");
        break;
    case 2:
        template_str = UserString("FORMAT_LIST_2_ITEMS");
        break;
    case 3:
        template_str = UserString("FORMAT_LIST_3_ITEMS");
        break;
    case 4:
        template_str = UserString("FORMAT_LIST_4_ITEMS");
        break;
    case 5:
        template_str = UserString("FORMAT_LIST_5_ITEMS");
        break;
    case 6:
        template_str = UserString("FORMAT_LIST_6_ITEMS");
        break;
    case 7:
        template_str = UserString("FORMAT_LIST_7_ITEMS");
        break;
    case 8:
        template_str = UserString("FORMAT_LIST_8_ITEMS");
        break;
    case 9:
        template_str = UserString("FORMAT_LIST_9_ITEMS");
        break;
    case 10:
        template_str = UserString("FORMAT_LIST_10_ITEMS");
        break;
    default:
        template_str = UserString("FORMAT_LIST_MANY_ITEMS");
        break;
    }

    boost::format fmt = FlexibleFormat(template_str) % header_fmt.str();

    for (const auto& word : words) {
        fmt % word;
    }

    return fmt;
}

template<typename Container>
boost::format FlexibleFormatList(const Container& words) {
    return FlexibleFormatList(std::vector<std::string>(), words,
                              UserString("FORMAT_LIST_DEFAULT_PLURAL_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_SINGLE_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_EMPTY_HEADER"),
                              UserString("FORMAT_LIST_DEFAULT_DUAL_HEADER"));
}

template<typename Container>
boost::format FlexibleFormatList(
    const Container& words, const std::string& all_header) {
    return FlexibleFormatList(std::vector<std::string>(), words, all_header, all_header, all_header, all_header);
}
template<typename Container>
boost::format FlexibleFormatList(
    const Container& words, const std::string& plural_header, const std::string& single_header) {
    return FlexibleFormatList(std::vector<std::string>(), words, plural_header, single_header, plural_header, plural_header);
}
template<typename Container>
boost::format FlexibleFormatList(
    const Container& words, const std::string& plural_header
    , const std::string& single_header, const std::string& empty_header) {
    return FlexibleFormatList(std::vector<std::string>(), words, plural_header, single_header, empty_header, plural_header);
}

template<typename T1, typename T2>
boost::format FlexibleFormatList(const T2& header_words, const T1& words) {
    return FlexibleFormatList(header_words, words
                              , UserString("FORMAT_LIST_DEFAULT_PLURAL_HEADER")
                              , UserString("FORMAT_LIST_DEFAULT_SINGLE_HEADER")
                              , UserString("FORMAT_LIST_DEFAULT_EMPTY_HEADER")
                              , UserString("FORMAT_LIST_DEFAULT_DUAL_HEADER"));
}

template<typename T1, typename T2>
boost::format FlexibleFormatList(
    const T2& header_words, const T1& words, const std::string& all_header) {
    return FlexibleFormatList(header_words, words, all_header, all_header, all_header, all_header);
}
template<typename T1, typename T2>
boost::format FlexibleFormatList(
    const T2& header_words, const T1& words, const std::string& plural_header, const std::string& single_header) {
    return FlexibleFormatList(header_words, words, plural_header, single_header, plural_header, plural_header);
}

template<typename T1, typename T2>
boost::format FlexibleFormatList(
    const T2& header_words, const T1& words, const std::string& plural_header
    , const std::string& single_header, const std::string& empty_header) {
    return FlexibleFormatList(header_words, words, plural_header, single_header, empty_header, plural_header);
}

#endif // _I18N_h_
