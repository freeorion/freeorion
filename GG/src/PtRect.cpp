//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/PtRect.h>
#if __has_include(<charconv>)
  #include <charconv>
#endif

using namespace GG;

////////////////////////////////////////////////
// GG::Pt
////////////////////////////////////////////////
std::ostream& GG::operator<<(std::ostream& os, Pt pt)
{
    os << "(" << pt.x << ", " << pt.y << ")";
    return os;
}

GG::Pt::operator std::string() const {
#if defined(__cpp_lib_to_chars)
    std::array<std::string::value_type, 64> buffer{"("}; // rest should be nulls. should be big enough for two ints as decimal text and some padding chars
    auto result = std::to_chars(buffer.data() + 1, buffer.data() + buffer.size() - 4, Value(x));
    *result.ptr = ',';
    *++result.ptr = ' ';
    result = std::to_chars(result.ptr + 1, buffer.data() + buffer.size() - 2, Value(y));
    *result.ptr = ')';
    return {buffer.data()};
#else
    std::string retval{};
    retval.reserve(60);
    retval.append("(")
        .append(std::to_string(Value(x)))
        .append(", ")
        .append(std::to_string(Value(y)))
        .append(")");
    return retval;
#endif
}

////////////////////////////////////////////////
// GG::Rect
////////////////////////////////////////////////
std::ostream& GG::operator<<(std::ostream& os, Rect rect)
{
    os << "[" << rect.ul << " - " << rect.lr << "]";
    return os;
}
