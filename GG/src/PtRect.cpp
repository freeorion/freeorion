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
    os << "(" << Value(pt.x) << ", " << Value(pt.y) << ")";
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

namespace StaticTests {
    static_assert(ToX(0.50f) == X1);
    static_assert(ToX(0.49) == X0);
    static_assert(ToY(0.0) == Y0);
    static_assert(ToY(-0.5f) == -Y1);
    static_assert(ToY(-1.49f) == -Y1);
    static_assert(ToY(-1.5) == Y{-2});

    static_assert(-X0 == X0);
    static_assert(-X1 == X{-1});
    static_assert(X1 + 1.0f == 2.0f);
    static_assert(std::is_same_v<decltype(X0 + 1.0), decltype(1.0 + X0)>);
    static_assert(std::is_same_v<decltype(X0 + 1.0f), float>);
    static_assert(X0/2 == X0);
    static_assert(std::is_same_v<decltype(42 - X0), int>);
    static_assert(std::is_same_v<decltype(Y0 - 2.4), decltype(2.4 - Y0)>);

    static_assert(X1 + X0 == X1);
    static_assert(Value(X{10} + 5) == 15);
    static_assert(X{10} + 5 == 3 + X{12});
    static_assert([](){ X x(X0); x += X1; return x; }() == X1);
    static_assert(Value([](){ X x(X1); x += (-2); return x; }()) == -1);
    static_assert(X0 + 1.0 == 1.0);
    static_assert(10.0 + X1 == 11.0 + X0);

    static_assert(X1 - X0 == X1);
    static_assert(Value(X{10} - 5) == 5);
    static_assert(10 - X{5} == 5);
    static_assert([](){ X x(X1); x -= X1; return x; }() == X0);
    static_assert([](){ X x(X0); x -= (-1); return x; }() == X1);
    static_assert(X1 - 1.0f == 0.0f);
    static_assert(10.0f - X1 == 9.0f + X0);

    static_assert(X1 * X0 == X0);
    static_assert(X{10} * 5 == X{50});
    static_assert(5 * X{10} == X{50});
    static_assert(X{10} * 5 == 2 * X{25});
    static_assert([](){ X x(X1); x *= X1; return x; }() == X1);
    static_assert(Value([](){ X x(X1); x *= (-1); return x; }()) == -1);
    static_assert(X1 * 1.0 == 1.0);
    static_assert(10.0 * X1 == X1 * 10.0);

    static_assert(X1 / X1 == 1);
    static_assert(X{10} / 5 == X{2});
    static_assert(X{5} / 10.0 == 25/50.0);
    static_assert(Value([](){ X x(X1); x /= (-1); return x; }()) == -1);
    static_assert(Value([](){ X x{21}; x /= 3.0; return x; }()) == 7);
    static_assert(X1 / 1.0 == 1.0);

    static_assert([](){ X x(X0); return ++x; }() == X1);
    static_assert([](){ Y y(Y0); y++; return y; }() == Y1);
    static_assert([](){ Y y(Y1); y++; return y; }() != Y1);
    static_assert([](){ X x(X1); return --x; }() == X0);
    static_assert([](){ Y y(Y1); y--; return y; }() == Y0);
    static_assert([](){ Y y(Y0); y--; return y; }() != Y0);

    constexpr Y szy{22};
    constexpr int margin = 3;
    constexpr int tw = Value(szy - 4 * margin);
    static_assert(tw == 10);
    constexpr int ow = tw + 3 * margin;
    static_assert(ow == 19);
    constexpr X szx{5};
    constexpr X tl = szx - tw - margin * 5 / 2;
    static_assert(Value(tl) == -12);
    constexpr auto btnl = szx - ow - margin;
    static_assert(btnl == GG::X(-17));
    constexpr auto btnr = szx - margin;
    static_assert(Value(btnr) == 2);
    static_assert(margin - szx == -Value(btnr));
}
