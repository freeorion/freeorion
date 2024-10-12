//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/UnicodeCharsets.h
//!
//! Contains the UnicodeCharsets class, and functions related to the character
//! sets defined in the Unicode standard.

#ifndef _GG_UnicodeCharsets_h_
#define _GG_UnicodeCharsets_h_


#include <cassert>
#include <set>
#include <vector>
#include <GG/Base.h>


namespace GG {

/** \brief Represents the name and character range of a set of Unicode
    characters.

    Such sets are known as "scripts" in Unicode parlance.  Note that the last
    character in the range is actually one past the last character, in the style
    of the STL. */
struct GG_API UnicodeCharset
{
    static constexpr std::size_t BLOCK_SIZE = 16;

    constexpr UnicodeCharset() = default;
    constexpr UnicodeCharset(std::string_view script_name, uint32_t first_char,
                             uint32_t last_char) noexcept :
        m_script_name(script_name),
        m_first_char(first_char),
        m_last_char(last_char + 1)
    {
        assert(!script_name.empty());
        assert(m_first_char % BLOCK_SIZE == 0);
        assert(m_last_char % BLOCK_SIZE == 0);
        assert(m_first_char < m_last_char);
    }

    // compare first char, then last char, then name
    constexpr bool operator<(const UnicodeCharset& rhs) const noexcept
    {
        return (m_first_char < rhs.m_first_char) ||
            (m_first_char == rhs.m_first_char && m_last_char < rhs.m_last_char) ||
            (m_first_char == rhs.m_first_char && m_last_char == rhs.m_last_char && m_script_name < rhs.m_script_name);
    }

#if defined(__cpp_impl_three_way_comparison)
    [[nodiscard]] constexpr bool operator==(const UnicodeCharset& rhs) const noexcept = default;
#else
    [[nodiscard]] constexpr bool operator==(const UnicodeCharset& rhs) const noexcept
    {
        return m_script_name == rhs.m_script_name &&
               m_first_char == rhs.m_first_char &&
               m_last_char == rhs.m_last_char;
    };
    [[nodiscard]] constexpr bool operator!=(const UnicodeCharset& rhs) const noexcept
    { return !(*this == rhs); };
#endif


    std::string_view m_script_name;
    uint32_t m_first_char = 0;
    uint32_t m_last_char = 0;
};

/** Returns the set of the UnicodeCharset's required to render \a str. */
GG_API std::vector<UnicodeCharset> UnicodeCharsetsToRender(std::string_view str);

/** Returns the UnicodeCharset in which \a c can be found, or 0 if no such
    UnicodeCharset exists. */
GG_API const UnicodeCharset* CharsetContaining(uint32_t c) noexcept;

/** Returns the UnicodeCharset called \a name, or 0 if no such UnicodeCharset
    exists. */
GG_API const UnicodeCharset* CharsetWithName(std::string_view name) noexcept;

}


#endif
