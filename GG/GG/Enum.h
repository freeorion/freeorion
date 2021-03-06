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
    const std::string& operator[](EnumType value) const;
    EnumType operator[](const std::string& name) const;

    void Insert(int& default_value, const std::string& entry);
    size_t size() const;

    static constexpr EnumType BAD_VALUE = (EnumType)INT_MIN;
private:
    std::map<std::string, EnumType> m_name_to_value_map;
    std::map<EnumType, std::string> m_value_to_name_map;
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
void BuildEnumMap(EnumMap<EnumType>& map, const std::string& enum_name,
                  const char* comma_separated_names)
{
    std::stringstream name_stream(comma_separated_names);

    int default_value = 0;
    std::string name;
    while (std::getline(name_stream, name, ','))
        map.Insert(default_value, std::move(name));
}

/** An enum macro for use inside classes.
  * Enables << and >> for your enum,
  * all of which will exist in whatever namespace this
  * macro is used. */
#define GG_CLASS_ENUM(EnumName, ...)                                                    \
    enum class EnumName : int {                                                         \
        __VA_ARGS__                                                                     \
    };                                                                                  \
                                                                                        \
    friend inline std::istream& operator>>(std::istream& is, EnumName& value) {         \
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.size() == 0)                                                            \
            ::GG::BuildEnumMap(map, #EnumName, #__VA_ARGS__);                           \
                                                                                        \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = map[name];                                                              \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    friend inline std::ostream& operator<<(std::ostream& os, EnumName value) {          \
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.size() == 0)                                                            \
            ::GG::BuildEnumMap(map, #EnumName, #__VA_ARGS__);                           \
                                                                                        \
        const std::string& name = map[value];                                           \
        return os << name;                                                              \
    }                                                                                   \

/** An enum macro for use outside of classes.
  * Enables << and >> for your enum,
  * all of which will exist in whatever namespace this
  * macro is used. */
#define GG_ENUM(EnumName, ...)                                                          \
    enum class EnumName : int {                                                         \
        __VA_ARGS__                                                                     \
    };                                                                                  \
                                                                                        \
    inline std::istream& operator>>(std::istream& is, EnumName& value) {                \
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.size() == 0)                                                            \
            ::GG::BuildEnumMap(map, #EnumName, #__VA_ARGS__);                           \
                                                                                        \
        std::string name;                                                               \
        is >> name;                                                                     \
        value = map[name];                                                              \
        return is;                                                                      \
    }                                                                                   \
                                                                                        \
    inline std::ostream& operator<<(std::ostream& os, EnumName value) {                 \
        ::GG::EnumMap<EnumName>& map = ::GG::GetEnumMap<EnumName>();                    \
        if (map.size() == 0)                                                            \
            ::GG::BuildEnumMap(map, #EnumName, #__VA_ARGS__);                           \
                                                                                        \
        const std::string& name = map[value];                                           \
        return os << name;                                                              \
    }                                                                                   \

/////////////
// EnumMap //
/////////////
template <typename EnumType>
const std::string& EnumMap<EnumType>::operator[](EnumType value) const
{
    auto it = m_value_to_name_map.find(value);
    if (it != m_value_to_name_map.end()) {
        return it->second;
    } else {
        static std::string none("None");
        return none;
    }
}

template <typename EnumType>
EnumType EnumMap<EnumType>::operator[](const std::string& name) const
{
    auto it = m_name_to_value_map.find(name);
    if (it != m_name_to_value_map.end()) {
        return it->second;
    } else {
        return BAD_VALUE;
    }
}

template <typename EnumType>
size_t EnumMap<EnumType>::size() const
{ return m_name_to_value_map.size(); }

template <typename EnumType>
void EnumMap<EnumType>::Insert(int& default_value, const std::string& entry)
{
    std::stringstream name_and_value(entry);

    std::string name;
    std::getline(name_and_value, name, '=');

    std::string value_str;
    EnumType value;
    if (std::getline(name_and_value, value_str)) {
        value = (EnumType)strtol(value_str.c_str(), nullptr, 0);
    } else {
        value = (EnumType)default_value;
    }

    boost::trim(name);

    m_name_to_value_map[name] = value;
    m_value_to_name_map[value] = std::move(name);
    default_value = static_cast<int>(value) + 1;
}

}


#endif
