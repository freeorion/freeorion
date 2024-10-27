//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/PtRect.h
//!
//! Contains the utility classes Pt and Rect.

#ifndef _GG_PtRect_h_
#define _GG_PtRect_h_

#include <GG/Base.h>
#include <GG/StrongTypedef.h>

namespace GG {

// X screen coordinate
POSITION_TYPEDEF(X)
// Y screen coordinate
POSITION_TYPEDEF(Y)

/** \brief A GG screen coordinate class. */
struct GG_API Pt
{
    constexpr Pt() = default;

    constexpr Pt(X x_, Y y_) noexcept :
        x(x_),
        y(y_)
    {}

#if defined(__cpp_impl_three_way_comparison)
    [[nodiscard]] constexpr auto operator<=>(const Pt&) const noexcept = default;
#else
    [[nodiscard]] constexpr bool operator==(const Pt& rhs) const noexcept
    { return x == rhs.x && y == rhs.y; };
    [[nodiscard]] constexpr bool operator!=(const Pt& rhs) const noexcept
    { return x != rhs.x || y != rhs.y; };
#endif


    [[nodiscard]] constexpr Pt operator-() const noexcept { return Pt(-x, -y); }
    constexpr Pt& operator+=(Pt rhs) noexcept             { x += rhs.x; y += rhs.y; return *this; }
    constexpr Pt& operator-=(Pt rhs) noexcept             { x -= rhs.x; y -= rhs.y; return *this; }
    constexpr Pt& operator/=(const double rhs) noexcept   { x /= rhs;   y /= rhs;   return *this; }
    constexpr Pt& operator*=(const double rhs) noexcept   { x *= rhs;   y *= rhs;   return *this; }

    [[nodiscard]] operator std::string() const;

    X x = X0;
    Y y = Y0;

    static constexpr std::size_t hash(std::size_t x) {
        uint64_t h = static_cast<uint64_t>(x);
        // from Boost hash_mix_impl
        const uint64_t m = (uint64_t(0xe9846afu) << 32u) + uint64_t(0x9b1a615du);
        h ^= h >> 32u;
        h *= m;
        h ^= h >> 32u;
        h *= m;
        h ^= h >> 28u;
        return static_cast<std::size_t>(h);
    }

    static constexpr std::size_t hash(std::size_t x, std::size_t y)
    { return hash(x + std::size_t{0x9e3779b9u} + y); } // from Boost hash_combine
};

inline constexpr Pt Pt0{GG::X0, GG::Y0};

GG_API std::ostream& operator<<(std::ostream& os, Pt pt);

[[nodiscard]] GG_API constexpr bool operator<(Pt lhs, Pt rhs) noexcept     { return lhs.x < rhs.x && lhs.y < rhs.y; }   ///< returns true if \a lhs.x and \a lhs.y are both less than the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr bool operator>(Pt lhs, Pt rhs) noexcept     { return lhs.x > rhs.x && lhs.y > rhs.y; }   ///< returns true if \a lhs.x and \a lhs.y are both greater than the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr bool operator<=(Pt lhs, Pt rhs) noexcept    { return lhs.x <= rhs.x && lhs.y <= rhs.y; } ///< returns true if \a lhs.x and \a lhs.y are both less than or equal to the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr bool operator>=(Pt lhs, Pt rhs) noexcept    { return lhs.x >= rhs.x && lhs.y >= rhs.y; } ///< returns true if \a lhs.x and \a lhs.y are both greater than or equal to the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr Pt   operator+(Pt lhs, Pt rhs) noexcept     { return Pt{lhs.x + rhs.x, lhs.y + rhs.y}; } ///< returns the vector sum of \a lhs and \a rhs
[[nodiscard]] GG_API constexpr Pt   operator-(Pt lhs, Pt rhs) noexcept     { return Pt{lhs.x - rhs.x, lhs.y - rhs.y}; } ///< returns the vector difference of \a lhs and \a rhs
[[nodiscard]] GG_API constexpr Pt   operator*(Pt lhs, double rhs) noexcept { return Pt{ToX(lhs.x * rhs), ToY(lhs.y * rhs)}; }     ///< returns the vector with components multiplied by \a rhs
[[nodiscard]] GG_API constexpr Pt   operator/(Pt lhs, double rhs) noexcept { return Pt{ToX(lhs.x / rhs), ToY(lhs.y / rhs)}; }     ///< returns the vector with components divided by \a rhs

/** \brief A GG rectangle class.

    This is essentially just two points that bound the rectangle. */
struct GG_API Rect
{
    constexpr Rect() = default;

    constexpr Rect(const Pt pt1, const Pt pt2) noexcept(noexcept(std::min(X0, X1))) :
        ul{std::min(pt1.x, pt2.x), std::min(pt1.y, pt2.y)},
        lr{std::max(pt1.x, pt2.x), std::max(pt1.y, pt2.y)}
    {}

    constexpr Rect(X x1, Y y1, X x2, Y y2) noexcept :
        ul{x1, y1},
        lr{x2, y2}
    {}

    [[nodiscard]] constexpr X  Left() const noexcept       { return ul.x; }            ///< returns the left boundary of the Rect
    [[nodiscard]] constexpr X  Right() const noexcept      { return lr.x; }            ///< returns the right boundary of the Rect
    [[nodiscard]] constexpr Y  Top() const  noexcept       { return ul.y; }            ///< returns the top boundary of the Rect
    [[nodiscard]] constexpr Y  Bottom() const noexcept     { return lr.y; }            ///< returns the bottom boundary of the Rect
    [[nodiscard]] constexpr Pt UpperLeft() const noexcept  { return ul; }              ///< returns the upper-left corner of the Rect
    [[nodiscard]] constexpr Pt LowerRight() const noexcept { return lr; }              ///< returns the lower-right corner of the Rect
    [[nodiscard]] constexpr X  Width() const noexcept      { return lr.x - ul.x; }     ///< returns the width of the Rect
    [[nodiscard]] constexpr Y  Height() const noexcept     { return lr.y - ul.y; }     ///< returns the height of the Rect
    [[nodiscard]] constexpr X  MidX() const noexcept       { return (lr.x + ul.x)/2; } ///< returns the horizontal mid-point of the Rect
    [[nodiscard]] constexpr Y  MidY() const noexcept       { return (lr.y + ul.y)/2; } ///< returns the vertical mid-point of the Rect

    [[nodiscard]] constexpr bool Contains(Pt pt) const noexcept { return ul <= pt && pt < lr; }

    constexpr Rect& operator+=(Pt pt) noexcept { ul += pt; lr += pt; return *this; } ///< shifts the Rect by adding \a pt to each corner
    constexpr Rect& operator-=(Pt pt) noexcept { ul -= pt; lr -= pt; return *this; } ///< shifts the Rect by subtracting \a pt from each corner

#if defined(__cpp_impl_three_way_comparison)
    [[nodiscard]] constexpr auto operator<=>(const Rect&) const noexcept = default;
#else
    [[nodiscard]] constexpr bool operator==(const Rect& rhs) const noexcept
    { return ul == rhs.ul && lr == rhs.lr; };
    [[nodiscard]] constexpr bool operator!=(const Rect& rhs) const noexcept
    { return ul != rhs.ul || lr != rhs.lr; };
#endif

    Pt ul, lr;
};

GG_API std::ostream& operator<<(std::ostream& os, Pt pt); ///< Pt stream-output operator for debug output

[[nodiscard]] GG_API constexpr Rect operator+(Rect rect, Pt pt) noexcept { return Rect(rect.ul + pt, rect.lr + pt); } ///< returns \a rect shifted by adding \a pt to each corner
[[nodiscard]] GG_API constexpr Rect operator-(Rect rect, Pt pt) noexcept { return Rect(rect.ul - pt, rect.lr - pt); } ///< returns \a rect shifted by subtracting \a pt from each corner
[[nodiscard]] GG_API constexpr Rect operator+(Pt pt, Rect rect) noexcept { return rect + pt; } ///< returns \a rect shifted by adding \a pt to each corner
[[nodiscard]] GG_API constexpr Rect operator-(Pt pt, Rect rect) noexcept { return rect - pt; } ///< returns \a rect shifted by subtracting \a pt from each corner

GG_API std::ostream& operator<<(std::ostream& os, Rect rect); ///< Rect stream-output operator for debug output

// Hash functions
[[nodiscard]] GG_API inline std::size_t hash_value(X x) { return std::hash<int32_t>()(Value(x)); }
[[nodiscard]] GG_API inline std::size_t hash_value(Y y) { return std::hash<int32_t>()(Value(y)); }
[[nodiscard]] GG_API inline std::size_t hash_value(Pt pt) {
    static_assert(std::is_same_v<std::decay_t<decltype(Value(pt.x))>, int32_t>);
    static_assert(static_cast<int64_t>(1) + (static_cast<int64_t>(1) << 32u) == 4294967297);
    auto temp = static_cast<int64_t>(Value(pt.x)) + (static_cast<int64_t>(Value(pt.y)) << 32u);
    return std::hash<decltype(temp)>()(temp);
}
[[nodiscard]] GG_API inline std::size_t hash_value(Rect r)
{ return Pt::hash(hash_value(r.ul), hash_value(r.lr)); }

}


#endif
