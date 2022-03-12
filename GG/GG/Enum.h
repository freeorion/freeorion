//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Enum.h
//!
//! Contains the utility classes and macros that allow for easy conversion to
//! and from an enum value and its textual representation.

#ifndef _GG_Enum_h_
#define _GG_Enum_h_


#include <climits>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <boost/algorithm/string/trim.hpp>


namespace GG {

#define GG_ENUM_NAME_BUFFER_SIZE 80

/** This class is not meant for public consumption.
    * Access this class through the functions generated
    * in the GG_ENUM or GG_CLASS_ENUM macro invocation. */
template <typename EnumType>
class EnumMap {
public:
    [[nodiscard]] constexpr std::string_view operator[](EnumType value) const
    {
        auto value_it = std::find(m_values.cbegin(), m_values.cend(), value);
        if (value_it == m_values.cend())
            return "None";
        auto dist = std::distance(m_values.cbegin(), value_it);
        auto name_it = m_names.cbegin();
        std::advance(name_it, dist);
        return *name_it;
    }

    [[nodiscard]] constexpr EnumType operator[](std::string_view name) const
    {
        auto name_it = std::find(m_names.cbegin(), m_names.cend(), name);
        if (name_it == m_names.cend())
            return std::numeric_limits<EnumType>::max();
        auto dist = std::distance(m_names.cbegin(), name_it);
        auto value_it = m_values.cbegin();
        std::advance(value_it, dist);
        return *value_it;
    }

    [[nodiscard]] constexpr size_t size() const { return m_size; }

    [[nodiscard]] constexpr bool empty() const { return m_size == 0; }

    /** Do not call this function directly.
      * Instead, rely on the functions generated
      * by the GG_ENUM or GG_CLASS_ENUM macro invocations. */
    constexpr void Build(const char* comma_separated_names)
    {
        if (Count(comma_separated_names) > CAPACITY)
            throw std::invalid_argument("too many comma separated enum vals to build map");
        auto count_names = SplitApply(comma_separated_names, Trim, ',');
        for (size_t i = 0; i < count_names.first; ++i)
            Insert(count_names.second[i]);
    }

private:
    // Formats entries passed as series of "SYMBOL = 0x1b" into key-value pairs
    // and inserts into this map
    constexpr void Insert(std::string_view entry)
    {
        auto count_trimmed_name_value_str = SplitApply<2>(entry, Trim, '=');
        static_assert(std::is_same_v<decltype(count_trimmed_name_value_str.first), size_t>);
        static_assert(std::is_same_v<decltype(count_trimmed_name_value_str.second),
                                     std::array<std::string_view, 2>>);

        auto count = count_trimmed_name_value_str.first;
        if (count < 2)
            return;
        auto [name, value_str] = count_trimmed_name_value_str.second;

        EnumType value = ToEnumType(value_str);
        //std::cout << "inserting entry: " << entry << "  as name: " << name
        //          << "  value string: " << value_str
        //          << "  value: " << static_cast<int>(value) << std::endl;

        const auto place_idx = m_size++;
        if (m_size >= CAPACITY)
            throw std::runtime_error("Too many entries inserted into EnumMap.");

        m_names[place_idx] = name;
        m_values[place_idx] = value;
    }

    static constexpr size_t CAPACITY = std::numeric_limits<unsigned char>::max();


    [[nodiscard]] static constexpr std::pair<std::string_view, std::string_view> Split(
        std::string_view delim_separated_vals, const char delim)
    {
        auto comma_idx = delim_separated_vals.find_first_of(delim);
        if (comma_idx == std::string_view::npos)
            return {delim_separated_vals, ""};
        return {delim_separated_vals.substr(0, comma_idx),
            delim_separated_vals.substr(comma_idx + 1)};
    }

    static constexpr std::string_view test_cs_names = "123 = 7  , next_thing =   124, last thing  =-1";
    static constexpr auto first_and_rest = Split(test_cs_names, ',');
    static constexpr auto second_and_rest = Split(first_and_rest.second, ',');
    static constexpr auto third_and_rest = Split(second_and_rest.second, ',');
    static constexpr auto fourth_and_rest = Split(third_and_rest.second, ',');
    static_assert(fourth_and_rest.first.empty());
    static constexpr std::string_view third_result_expected = " last thing  =-1";
    static_assert(third_and_rest.first == third_result_expected);


    // how many times does \a delim appear in text?
    [[nodiscard]] static constexpr size_t Count(std::string_view text, const char delim = ',')
    {
        size_t retval = 1;
        for (size_t i = 0; i < text.length(); ++i)
            retval += text[i] == delim;
        return retval;
    }
    static constexpr auto comma_count = Count(test_cs_names);
    static_assert(comma_count == 3);


    template <size_t RETVAL_CAP = CAPACITY, typename F>
    [[nodiscard]] static constexpr std::pair<size_t, std::array<std::string_view, RETVAL_CAP>>
        SplitApply(std::string_view comma_separated_names, F&& fn, char delim)
    {
        size_t count = 0;
        std::array<std::string_view, RETVAL_CAP> retval;
        while (count < RETVAL_CAP) {
            auto next_and_rest = Split(comma_separated_names, delim);
            if (next_and_rest.first.empty())
                break;
            retval[count++] = fn(next_and_rest.first);
            comma_separated_names = next_and_rest.second;
        }
        return {count, retval};
    }


    [[nodiscard]] static constexpr std::string_view Trim(std::string_view padded)
    {
        constexpr std::string_view whitespace = " \b\f\n\r\t\v";
        auto start_idx = padded.find_first_not_of(whitespace);
        auto end_idx = padded.find_last_not_of(whitespace);
        return padded.substr(start_idx, end_idx - start_idx + 1);
    }
    static constexpr std::string_view test_text = " \n\f  something = \t 0x42\b\t  ";
    static constexpr std::string_view trimmed_text_expected = "something = \t 0x42";
    static_assert(Trim(test_text) == trimmed_text_expected);


    static constexpr auto split_count_vals = SplitApply(test_cs_names, Trim, ',');
    static_assert(split_count_vals.first == 3);
    static_assert(!split_count_vals.second[0].empty());
    static constexpr std::string_view expected_first_trimmed_name = "123 = 7";
    static_assert(split_count_vals.second[0] == expected_first_trimmed_name);


    static constexpr std::string_view bin = "1";
    static constexpr std::string_view hex = "0x4f";
    static constexpr std::string_view dec_neg = "-39";
    static constexpr std::string_view dec_big = "237534";
    static constexpr std::string_view dec_sml = "123";
    static constexpr std::string_view alpha = "xxxx";

    [[nodiscard]] static constexpr bool IsDecChar(char c)
    { return c >= '0' && c <= '9'; }
    [[nodiscard]] static constexpr bool IsDec(std::string_view txt)
    {
        return txt.length() >= 1 &&
            (IsDecChar(txt[0]) || (txt[0] == '-' && txt.length() >= 2)) &&
            txt.substr(1).find_first_not_of("0123456789") == std::string_view::npos;
    }
    static_assert(IsDec(dec_neg) && IsDec(dec_big) && IsDec(dec_sml) &&
                  !IsDec(hex) && IsDec(bin) && !IsDec(alpha));


    [[nodiscard]] static constexpr bool IsAtoF(char c)
    { return c >= 'a' && c <= 'f'; }
    [[nodiscard]] static constexpr bool IsHexChar(char c)
    { return IsDecChar(c) || IsAtoF(c); }
    [[nodiscard]] static constexpr bool IsHex(std::string_view txt)
    {
        return txt.length() == 4 && txt[0] == '0' && txt[1] == 'x' &&
            IsHexChar(txt[2]) && IsHexChar(txt[3]);
    }
    static_assert(IsHex(hex) && !IsHex(bin) && !IsHex(dec_neg) && IsHex("0x1d"));


    [[nodiscard]] static constexpr unsigned int Base(std::string_view txt) {
        if (IsHex(txt))
            return 16;
        if (IsDec(txt))
            return 10;
        return 0;
    }
    static_assert(Base("124") == 10);
    static_assert(Base("0x5d") == 16);


    [[nodiscard]] static constexpr int ToInt(std::string_view txt) {
        using C = std::string_view::value_type;
        static_assert(std::is_same_v<C, char>);
        constexpr std::string_view dec_chars = "0123456789";
        static_assert(dec_chars.find('5') == 5);
        static_assert(dec_chars.find('0') == 0);
        static_assert(dec_chars.find('b') == std::string_view::npos);
        constexpr std::string_view hex_chars = "0123456789abcdef";
        static_assert(hex_chars.find('b') == 11);

        unsigned char base = Base(txt);
        if (base == 0)
            return 0;

        std::string_view valid_chars = base == 10 ? dec_chars :
            base == 16 ? hex_chars : "";

        bool negative = txt[0] == '-';
        size_t idx = negative;
        int retval = valid_chars.find(txt[idx++]);

        for (; idx < txt.length(); ++idx) {
            retval *= base;
            retval += valid_chars.find(txt[idx]);
        }

        return retval * (1 - 2*negative);
    }

    [[nodiscard]] static constexpr EnumType ToEnumType(std::string_view str)
    { return EnumType(ToInt(str)); }


    size_t                            m_size = 0;
    std::array<std::string, CAPACITY> m_names{};
    std::array<EnumType, CAPACITY>    m_values{};
};

/** Do not call this function directly.
  * Instead, rely on the functions generated
  * by the GG_ENUM or GG_CLASS_ENUM macro invocations. */
template <typename EnumType>
EnumMap<EnumType>& GetEnumMap()
{
    static EnumMap<EnumType> map;
    return map;
}


/** An enum macro for use inside classes.
  * Enables << and >> for your enum,
  * all of which will exist in whatever namespace this
  * macro is used. */
#define GG_CLASS_ENUM(EnumName, UnderlyingType, ...)                                    \
    enum class EnumName : UnderlyingType {                                              \
        __VA_ARGS__                                                                     \
    };                                                                                  \
                                                                                        \
    friend inline std::istream& operator>>(std::istream& is, EnumName& value) {         \
        auto& map = ::GG::GetEnumMap<EnumName>();                                       \
        if (map.empty())                                                                \
            map.Build(#__VA_ARGS__);                                                    \
                                                                                        \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = map[name];                                                              \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    friend inline std::ostream& operator<<(std::ostream& os, EnumName value) {          \
        auto& map = ::GG::GetEnumMap<EnumName>();                                       \
        if (map.empty())                                                                \
            map.Build(#__VA_ARGS__);                                                    \
                                                                                        \
        return os << map[value];                                                        \
    }

/** An enum macro for use outside of classes.
  * Enables << and >> for your enum,
  * all of which will exist in whatever namespace this
  * macro is used. */
#define GG_ENUM(EnumName, UnderlyingType, ...)                                          \
    enum class EnumName : UnderlyingType {                                              \
        __VA_ARGS__                                                                     \
    };                                                                                  \
                                                                                        \
    inline std::istream& operator>>(std::istream& is, EnumName& value) {                \
        auto& map = ::GG::GetEnumMap<EnumName>();                                       \
        if (map.size() == 0)                                                            \
            map.Build(#__VA_ARGS__);                                                    \
                                                                                        \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = map[name];                                                              \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    inline std::ostream& operator<<(std::ostream& os, EnumName value) {                 \
        auto& map = ::GG::GetEnumMap<EnumName>();                                       \
        if (map.empty())                                                                \
            map.Build(#__VA_ARGS__);                                                    \
                                                                                        \
        return os << map[value];                                                        \
    }                                                                                   \
                                                                                        \
    [[nodiscard]] inline std::string_view to_string(EnumName value) {                   \
        auto& map = ::GG::GetEnumMap<EnumName>();                                       \
        if (map.empty())                                                                \
            map.Build(#__VA_ARGS__);                                                    \
                                                                                        \
        return map[value];                                                              \
    }

/////////////
// EnumMap //
/////////////


}


#endif
