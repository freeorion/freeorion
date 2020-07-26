//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

/** \file UnicodeCharsets.h \brief Contains the UnicodeCharsets class, and
    functions related to the character sets defined in the Unicode
    standard. */

#ifndef _UnicodeCharsets_h_
#define _UnicodeCharsets_h_

#include <GG/Base.h>

#include <set>
#include <vector>


namespace GG {

/** \brief Represents the name and character range of a set of Unicode
    characters.

    Such sets are known as "scripts" in Unicode parlance.  Note that the last
    character in the range is actually one past the last character, in the style
    of the STL. */
struct GG_API UnicodeCharset
{
    UnicodeCharset();
    UnicodeCharset(std::string script_name, std::uint32_t first_char, std::uint32_t last_char);

    std::string m_script_name;
    std::uint32_t m_first_char;
    std::uint32_t m_last_char;
};

/** Returns true iff all of \a lhs's and \a rhs's members compare equal. */
GG_API bool operator==(const UnicodeCharset& lhs, const UnicodeCharset& rhs);

/** Returns true iff \a lhs.m_first_char < \a rhs.m_first_char. */
GG_API bool operator<(const UnicodeCharset& lhs, const UnicodeCharset& rhs);


/** Returns a vector containing all defined UnicodeCharset's. */
GG_API const std::vector<UnicodeCharset>& AllUnicodeCharsets();

/** Returns the set of the UnicodeCharset's required to render \a str. */
GG_API std::set<UnicodeCharset> UnicodeCharsetsToRender(const std::string& str);

/** Returns the UnicodeCharset in which \a c can be found, or 0 if no such
    UnicodeCharset exists. */
GG_API const UnicodeCharset* CharsetContaining(std::uint32_t c);

/** Returns the UnicodeCharset called \a name, or 0 if no such UnicodeCharset
    exists. */
GG_API const UnicodeCharset* CharsetWithName(const std::string& name);

}

#endif
