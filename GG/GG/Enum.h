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
    [[nodiscard]] std::string_view operator[](EnumType value) const
    {
        auto value_it = std::find(m_values.cbegin(), m_values.cend(), value);
        if (value_it == m_values.cend())
            return "None";
        auto dist = std::distance(m_values.cbegin(), value_it);
        auto name_it = m_names.cbegin();
        std::advance(name_it, dist);
        return *name_it;
    }

    [[nodiscard]] EnumType operator[](std::string_view name) const
    {
        auto name_it = std::find(m_names.cbegin(), m_names.cend(), name);
        if (name_it == m_names.cend())
            return std::numeric_limits<EnumType>::max();
        auto dist = std::distance(m_names.cbegin(), name_it);
        auto value_it = m_values.cbegin();
        std::advance(value_it, dist);
        return *value_it;
    }

    void Insert(std::string_view entry)
    {
        // entires passed as series of "SYMBOL = 0x1b" formatted key-value pairs
        // parse into key name and value
        auto [name, value_str] = Split(entry);
        EnumType value = EnumType(strtol(value_str.data(), nullptr, 0));

        const auto place_idx = m_size++;
        if (m_size >= CAPACITY)
            throw std::runtime_error("Too many entries inserted into EnumMap.");

        m_names[place_idx] = name;
        m_values[place_idx] = value;
    }

    [[nodiscard]] size_t size() const { return m_size; }
    [[nodiscard]] bool empty() const { return m_size == 0; }

private:
    static constexpr size_t CAPACITY = std::numeric_limits<unsigned char>::max();

    [[nodiscard]] static constexpr std::pair<std::string_view, std::string_view>
        Split(std::string_view entry, char separator = '=')
    {
        constexpr char* whitespace = " \b\f\n\r\t\v";
        auto name_start = entry.find_first_not_of(whitespace);
        auto eq_pos = entry.find_first_of(separator, name_start);
        auto name_to_eq = entry.substr(name_start, eq_pos - name_start + 1);
        auto name_to_before_eq = name_to_eq.substr(0, name_to_eq.length() - 1);
        auto name_end = name_to_before_eq.find_last_not_of(whitespace);
        auto name = name_to_before_eq.substr(0, name_end + 1);
        auto after_eq = entry.substr(eq_pos + 1);
        auto value_start = after_eq.find_first_not_of(whitespace);
        auto value_to_end = after_eq.substr(value_start);
        auto value_end = value_to_end.find_first_of(whitespace);
        auto value = after_eq.substr(value_start, value_end);

        return {name, value};
    };

    static constexpr std::string_view test_text = "   something = \t 0x42\b\t  ";
    static constexpr auto test_name_value = Split(test_text);
    static_assert(test_name_value.first == "something" && test_name_value.second == "0x42");


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

/** Do not call this function directly.
  * Instead, rely on the functions generated
  * by the GG_ENUM or GG_CLASS_ENUM macro invocations. */
template <typename EnumType>
void BuildEnumMap(EnumMap<EnumType>& map, const char* comma_separated_names)
{
    std::stringstream name_stream(comma_separated_names);

    std::string name;
    while (std::getline(name_stream, name, ','))
        map.Insert(name);
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
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.empty())                                                                \
            ::GG::BuildEnumMap(map, #__VA_ARGS__);                                      \
                                                                                        \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = map[name];                                                              \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    friend inline std::ostream& operator<<(std::ostream& os, EnumName value) {          \
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.empty())                                                                \
            ::GG::BuildEnumMap(map, #__VA_ARGS__);                                      \
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
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.size() == 0)                                                            \
            ::GG::BuildEnumMap(map, #__VA_ARGS__);                                      \
                                                                                        \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = map[name];                                                              \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    inline std::ostream& operator<<(std::ostream& os, EnumName value) {                 \
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.empty())                                                                \
            ::GG::BuildEnumMap(map, #__VA_ARGS__);                                      \
                                                                                        \
        return os << map[value];                                                        \
    }                                                                                   \
                                                                                        \
    [[nodiscard]] inline std::string_view to_string(EnumName value) {                   \
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.empty())                                                                \
            ::GG::BuildEnumMap(map, #__VA_ARGS__);                                      \
                                                                                        \
        return map[value];                                                              \
    }

/////////////
// EnumMap //
/////////////


}


#endif
