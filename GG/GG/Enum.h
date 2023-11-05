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
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <limits>

namespace GG {

/** This class is not meant for public consumption.
  * Access this class through the functions generated
  * in the GG_ENUM or GG_CLASS_ENUM macro invocation. */
template <typename EnumType>
class EnumMap {
public:
    constexpr explicit EnumMap(const char* comma_separated_names)
    { Build(comma_separated_names); }

    [[nodiscard]] constexpr std::string_view operator[](EnumType value) const noexcept
    {
        std::size_t idx = 0;
        for (; idx < m_size; ++idx)
        {
            if (m_values[idx] == value)
            {
                auto name_it = m_names.cbegin() + idx;
                return *name_it;
            }
        }
        return "None";
    }

    [[nodiscard]] constexpr EnumType operator[](std::string_view name) const noexcept
    { return FromString(name); }

    [[nodiscard]] constexpr EnumType FromString(std::string_view name,
                                                EnumType not_found_result = EnumType(0)) const noexcept
    {
        std::size_t idx = 0;
        for (; idx < m_size; ++idx) // TODO: use constexpr std::find once C++20 is available
        {
            if (m_names[idx] == name)
            {
                auto value_it = m_values.cbegin() + idx;
                return *value_it;
            }
        }
        return not_found_result;
    }

    [[nodiscard]] constexpr auto size() const noexcept { return m_size; }

    [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

private:
    constexpr void Build(const char* comma_separated_names)
    {
        if (Count(comma_separated_names, ',') > CAPACITY)
            throw std::invalid_argument("too many comma separated enum vals to build map");
        auto count_names = SplitApply(comma_separated_names, Trim, ',');
        for (std::size_t i = 0; i < count_names.first; ++i)
            Insert(count_names.second[i]);
    }

    // Formats entries passed as series of "SYMBOL = 0x1b" into key-value pairs
    // and inserts into this map
    constexpr void Insert(std::string_view entry)
    {
        const auto [parts_count, trimmed_strs] = SplitApply<2>(entry, Trim, '='); // separate into parts before and after =
        static_assert(std::is_same_v<decltype(parts_count), const std::size_t>);
        static_assert(std::is_same_v<decltype(trimmed_strs), const std::array<std::string_view, 2>>);

        const auto name = trimmed_strs[0];

        const auto value = [had_value_specified{parts_count >= 2}, value_str{trimmed_strs[1]}, this]() {
            if (had_value_specified)
                return EnumType(ToInt(value_str));

            // increment last-specified value or default to 0 for the first
            using value_num_t = std::underlying_type_t<EnumType>;
            const value_num_t next_value_num = (m_size == 0u) ? 0 : (static_cast<value_num_t>(m_values[m_size-1]) + 1);
            return EnumType(next_value_num);
        }();

        const auto place_idx = m_size++;
        if (m_size >= CAPACITY)
            throw std::runtime_error("Too many entries inserted into EnumMap.");

        m_names[place_idx] = name;
        m_values[place_idx] = value;
    }

    static constexpr std::size_t CAPACITY = std::numeric_limits<uint8_t>::max();

public:
    [[nodiscard]] static constexpr std::pair<std::string_view, std::string_view> Split(
        std::string_view delim_separated_vals, const char delim)
    {
        auto comma_idx = delim_separated_vals.find_first_of(delim);
        if (comma_idx == std::string_view::npos)
            return {delim_separated_vals, ""};
        return {delim_separated_vals.substr(0, comma_idx),
            delim_separated_vals.substr(comma_idx + 1)};
    }
private:

    static constexpr std::string_view test_cs_names = "123 = 7  , next_thing =   124, last thing  =-1";
    static constexpr auto first_and_rest = Split(test_cs_names, ',');
    static constexpr auto second_and_rest = Split(first_and_rest.second, ',');
    static constexpr auto third_and_rest = Split(second_and_rest.second, ',');
    static constexpr auto fourth_and_rest = Split(third_and_rest.second, ',');
    static_assert(first_and_rest.first == "123 = 7  ");
    static_assert(fourth_and_rest.first.empty());
    static constexpr std::string_view third_result_expected = " last thing  =-1";
    static_assert(third_and_rest.first == third_result_expected);


    // how many times does \a delim appear in text?
    [[nodiscard]] static constexpr auto Count(std::string_view text, const char delim)
    {
        std::size_t retval = 1;
        for (std::size_t i = 0; i < text.length(); ++i)
            retval += text[i] == delim;
        return retval;
    }
    static constexpr auto comma_count = Count(test_cs_names, ',');
    static_assert(comma_count == 3);

public:
    template <std::size_t RETVAL_CAP = CAPACITY, typename F>
    [[nodiscard]] static constexpr std::pair<std::size_t, std::array<std::string_view, RETVAL_CAP>>
        SplitApply(std::string_view comma_separated_names, F&& fn, char delim)
    {
        std::size_t count = 0;
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
private:
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
        constexpr std::string_view dec_chars = "0123456789";
        //static_assert(dec_chars.find('5') == 5);
        //static_assert(dec_chars.find('0') == 0);
        //static_assert(dec_chars.find('b') == std::string_view::npos);
        constexpr std::string_view hex_chars = "0123456789abcdef";
        //static_assert(hex_chars.find('b') == 11);

        auto base = Base(txt);
        if (base == 0)
            return 0;

        std::string_view valid_chars = base == 10 ? dec_chars :
            base == 16 ? hex_chars : "";
        bool is_negative = txt[0] == '-';
        bool is_hex = base == 16;

        int retval = 0;
        for (auto c : txt.substr(static_cast<size_t>((is_negative ? 1u : 0u) + 2u*is_hex))) {
            retval *= base;
            std::size_t digit = valid_chars.find(c);
            retval += static_cast<int>(digit);
        }

        return retval * (is_negative ? -1 : 1);
    }
    static_assert(ToInt("-104") == -104);
    static_assert(ToInt("853104") == 853104);
    static_assert(ToInt("0") == 0);
    static_assert(ToInt("0xa0") == 160);
    static_assert(ToInt("0x5d") == 93);
    static_assert(ToInt(std::string_view{}) == 0);


    std::size_t                            m_size = 0;
    std::array<std::string_view, CAPACITY> m_names{};
    std::array<EnumType, CAPACITY>         m_values{};
};


/** An enum macro for use inside classes.
  * Enables << and >> for your enum,
  * all of which will exist in whatever namespace this
  * macro is used. */
#define GG_CLASS_ENUM(EnumName, UnderlyingType, ...)                                    \
    enum class EnumName : UnderlyingType {                                              \
        __VA_ARGS__                                                                     \
    };                                                                                  \
                                                                                        \
    static inline const EnumMap<EnumName>& GetEnumMap() noexcept                        \
    {                                                                                   \
        constexpr EnumMap<EnumName> cmap(#__VA_ARGS__);                                 \
        static const auto& map{cmap};                                                   \
        return map;                                                                     \
    }                                                                                   \
                                                                                        \
    friend inline std::istream& operator>>(std::istream& is, EnumName& value) {         \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = GetEnumMap()[name];                                                     \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    friend inline std::ostream& operator<<(std::ostream& os, EnumName value)            \
    { return os << GetEnumMap()[value]; }


template <typename EnumType>
constexpr EnumMap<EnumType> CGetEnumMap() noexcept
{
    constexpr EnumMap<EnumType> cmap("");
    return cmap;
}

template <typename EnumType>
inline const EnumMap<EnumType>& GetEnumMap() noexcept
{
    static const auto& map{CGetEnumMap<EnumType>()};
    return map;
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
    template <>                                                                         \
    constexpr EnumMap<EnumName> CGetEnumMap() noexcept                                  \
    {                                                                                   \
        constexpr EnumMap<EnumName> cmap(#__VA_ARGS__);                                 \
        return cmap;                                                                    \
    }                                                                                   \
                                                                                        \
    template <>                                                                         \
    inline const EnumMap<EnumName>& GetEnumMap() noexcept                               \
    {                                                                                   \
        static const auto& map{CGetEnumMap<EnumName>()};                                \
        return map;                                                                     \
    }                                                                                   \
                                                                                        \
    inline std::istream& operator>>(std::istream& is, EnumName& value) {                \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = GetEnumMap<EnumName>()[name];                                           \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    inline std::ostream& operator<<(std::ostream& os, EnumName value)                   \
    { return os << GetEnumMap<EnumName>()[value]; }                                     \
                                                                                        \
    [[nodiscard]] constexpr std::string_view to_string(EnumName value) noexcept         \
    { return CGetEnumMap<EnumName>()[value]; }                                          \
                                                                                        \
    [[nodiscard]] constexpr EnumName EnumName##FromString(                              \
        std::string_view sv, EnumName result_not_found = EnumName(0)) noexcept          \
    { return CGetEnumMap<EnumName>().FromString(sv, result_not_found); }
}


#endif
