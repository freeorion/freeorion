// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */
   
/** \file Enum.h \brief Contains the utility classes and macros that allow for
    easy conversion to and from an enum value and its textual
    representation. */

#ifndef _GG_Enum_h_
#define _GG_Enum_h_

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <climits>

#include <boost/algorithm/string/trim.hpp>

namespace GG {

    #define GG_ENUM_NAME_BUFFER_SIZE 80

    /** This class is not meant for public consumption.
      * Access this class through the functions generated
      * in the GG_ENUM or GG_CLASS_ENUM macro invocation. */
    template <class EnumType>
    class EnumMap {
    public:
        const std::string& operator[](EnumType value) const;
        EnumType operator[](const std::string& name) const;

        void Insert(int& default_value, const std::string& entry);
        size_t size() const;

        static const EnumType BAD_VALUE = (EnumType)INT_MIN;
    private:
        std::map<std::string, EnumType> m_name_to_value_map;
        std::map<EnumType, std::string> m_value_to_name_map;
    };

    /** Do not call this function directly.
      * Instead, rely on the functions generated
      * by the GG_ENUM or GG_CLASS_ENUM macro invocations. */
    template <class EnumType>
    EnumMap<EnumType>& GetEnumMap() {
        static EnumMap<EnumType> map;
        return map;
    }
    
    /** Do not call this function directly.
      * Instead, rely on the functions generated
      * by the GG_ENUM or GG_CLASS_ENUM macro invocations. */
    template <class EnumType>
    void BuildEnumMap(EnumMap<EnumType>& map, const std::string& enum_name, const char* comma_separated_names) {
        std::stringstream name_stream(comma_separated_names);

        int default_value = 0;
        std::string name;
        while (std::getline(name_stream, name, ',')) {
            map.Insert(default_value, name);
        }
    }

/** An enum macro for use inside classes.
  * Enables << and >> for your enum,
  * all of which will exist in whatever namespace this
  * macro is used. */
#define GG_CLASS_ENUM(EnumName, ...)                                                    \
    enum EnumName {                                                                     \
        __VA_ARGS__                                                                     \
     };                                                                                 \
                                                                                        \
    friend inline std::istream& operator>>(std::istream& is, EnumName& value) {  \
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
    friend inline std::ostream& operator<<(std::ostream& os, EnumName value) {   \
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
    enum EnumName : int {                                                               \
        __VA_ARGS__                                                                     \
    };                                                                                  \
                                                                                        \
    inline std::istream& operator>>(std::istream& is, EnumName& value) {         \
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
    inline std::ostream& operator<<(std::ostream& os, EnumName value) {          \
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
    template <class EnumType>
    const std::string& EnumMap<EnumType>::operator[](EnumType value) const {
        auto it = m_value_to_name_map.find(value);
        if (it != m_value_to_name_map.end()) {
            return it->second;
        } else {
            static std::string none("None");
            return none;
        }
    }

    template <class EnumType>
    EnumType EnumMap<EnumType>::operator[](const std::string& name) const {
        auto it = m_name_to_value_map.find(name);
        if (it != m_name_to_value_map.end()) {
            return it->second;
        } else {
            return BAD_VALUE;
        }
    }

    template <class EnumType>
    size_t EnumMap<EnumType>::size() const {
        return m_name_to_value_map.size();
    }

    template <class EnumType>
    void EnumMap<EnumType>::Insert(int& default_value, const std::string& entry) {
        std::stringstream name_and_value(entry);

        std::string name;
        std::getline(name_and_value, name, '=');

        std::string value_str;
        EnumType value;
        if (std::getline(name_and_value, value_str)) {
            value = (EnumType)strtol(value_str.c_str(), nullptr, 0);
        }
        else {
            value = (EnumType)default_value;
        }

        boost::trim(name);

        m_name_to_value_map[name] = value;
        m_value_to_name_map[value] = name;
        default_value = value + 1;
    }

} // namespace GG

#endif
