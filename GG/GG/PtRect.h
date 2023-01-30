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


#include <boost/functional/hash.hpp>
#include <GG/Base.h>
#include <GG/StrongTypedef.h>


namespace GG {

/** \class GG::X
    \brief The x-coordinate value type.

    X has an underlying value type of int.  \see GG_STRONG_INTEGRAL_TYPEDEF */
GG_STRONG_INTEGRAL_TYPEDEF(X, int32_t);

/** \class GG::Y
    \brief The y-coordinate value type.

    Y has an underlying value type of int.  \see GG_STRONG_INTEGRAL_TYPEDEF */
GG_STRONG_INTEGRAL_TYPEDEF(Y, int32_t);

// some useful coordinate constants
constexpr X X0{0};
constexpr X X1{1};
constexpr Y Y0{0};
constexpr Y Y1{1};

/** \brief A GG screen coordinate class. */
struct GG_API Pt
{
    constexpr Pt() = default;

    constexpr Pt(X x_, Y y_) noexcept :
        x(x_),
        y(y_)
    {}

    constexpr Pt(X_d x_, Y y_) noexcept :
        x(x_),
        y(y_)
    {}

    constexpr Pt(X x_, Y_d y_) noexcept :
        x(x_),
        y(y_)
    {}

    constexpr Pt(X_d x_, Y_d y_) noexcept :
        x(x_),
        y(y_)
    {}

    /** Returns true if x < \a rhs.x or returns true if x == \a rhs.x and y
        <\a rhs.y.  This is useful for sorting Pts in STL containers and
        algorithms. */
    [[nodiscard]] constexpr bool Less(Pt rhs) const noexcept
    { return x < rhs.x ? true : (x == rhs.x ? (y < rhs.y ? true : false) : false); }

    [[nodiscard]] constexpr Pt operator-() const noexcept { return Pt(-x, -y); }
    constexpr Pt& operator+=(Pt rhs) noexcept             { x += rhs.x; y += rhs.y; return *this; }
    constexpr Pt& operator-=(Pt rhs) noexcept             { x -= rhs.x; y -= rhs.y; return *this; }
    constexpr Pt& operator/=(const double rhs) noexcept   { x /= rhs;   y /= rhs;   return *this; }
    constexpr Pt& operator*=(const double rhs) noexcept   { x *= rhs;   y *= rhs;   return *this; }

    [[nodiscard]] operator std::string() const;

    X x = X0;
    Y y = Y0;
};

GG_API std::ostream& operator<<(std::ostream& os, Pt pt);

[[nodiscard]] GG_API constexpr inline bool operator==(Pt lhs, Pt rhs) noexcept    { return lhs.x == rhs.x && lhs.y == rhs.y; } ///< returns true if \a lhs is identical to \a rhs
[[nodiscard]] GG_API constexpr inline bool operator!=(Pt lhs, Pt rhs) noexcept    { return !(lhs == rhs); }                    ///< returns true if \a lhs differs from \a rhs
[[nodiscard]] GG_API constexpr inline bool operator<(Pt lhs, Pt rhs) noexcept     { return lhs.x < rhs.x && lhs.y < rhs.y; }   ///< returns true if \a lhs.x and \a lhs.y are both less than the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr inline bool operator>(Pt lhs, Pt rhs) noexcept     { return lhs.x > rhs.x && lhs.y > rhs.y; }   ///< returns true if \a lhs.x and \a lhs.y are both greater than the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr inline bool operator<=(Pt lhs, Pt rhs) noexcept    { return lhs.x <= rhs.x && lhs.y <= rhs.y; } ///< returns true if \a lhs.x and \a lhs.y are both less than or equal to the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr inline bool operator>=(Pt lhs, Pt rhs) noexcept    { return lhs.x >= rhs.x && lhs.y >= rhs.y; } ///< returns true if \a lhs.x and \a lhs.y are both greater than or equal to the corresponding components of \a rhs
[[nodiscard]] GG_API constexpr inline Pt   operator+(Pt lhs, Pt rhs) noexcept     { return Pt{lhs.x + rhs.x, lhs.y + rhs.y}; } ///< returns the vector sum of \a lhs and \a rhs
[[nodiscard]] GG_API constexpr inline Pt   operator-(Pt lhs, Pt rhs) noexcept     { return Pt{lhs.x - rhs.x, lhs.y - rhs.y}; } ///< returns the vector difference of \a lhs and \a rhs
[[nodiscard]] GG_API constexpr inline Pt   operator*(Pt lhs, double rhs) noexcept { return Pt{lhs.x * rhs, lhs.y * rhs}; }     ///< returns the vector with components multiplied by \a rhs
[[nodiscard]] GG_API constexpr inline Pt   operator/(Pt lhs, double rhs) noexcept { return Pt{lhs.x / rhs, lhs.y / rhs}; }     ///< returns the vector with components divided by \a rhs

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

    Pt ul; ///< the upper-left corner of the Rect
    Pt lr; ///< the lower-right corner of the Rect
};

GG_API std::ostream& operator<<(std::ostream& os, Pt pt); ///< Pt stream-output operator for debug output

/** returns true if \a lhs is identical to \a rhs */
[[nodiscard]] GG_API inline constexpr bool operator==(Rect lhs, Rect rhs) noexcept { return lhs.ul.x == rhs.ul.x && lhs.lr.x == rhs.lr.x && lhs.ul.y == rhs.ul.y && lhs.lr.y == rhs.lr.y; }

/** returns true if \a lhs differs from \a rhs */
[[nodiscard]] GG_API inline constexpr bool operator!=(Rect lhs, Rect rhs) noexcept { return !(lhs == rhs); }

[[nodiscard]] GG_API inline constexpr Rect operator+(Rect rect, Pt pt) noexcept { return Rect(rect.ul + pt, rect.lr + pt); } ///< returns \a rect shifted by adding \a pt to each corner
[[nodiscard]] GG_API inline constexpr Rect operator-(Rect rect, Pt pt) noexcept { return Rect(rect.ul - pt, rect.lr - pt); } ///< returns \a rect shifted by subtracting \a pt from each corner
[[nodiscard]] GG_API inline constexpr Rect operator+(Pt pt, Rect rect) noexcept { return rect + pt; } ///< returns \a rect shifted by adding \a pt to each corner
[[nodiscard]] GG_API inline constexpr Rect operator-(Pt pt, Rect rect) noexcept { return rect - pt; } ///< returns \a rect shifted by subtracting \a pt from each corner

GG_API std::ostream& operator<<(std::ostream& os, Rect rect); ///< Rect stream-output operator for debug output

// Hash functions
// Replace with C++11 equilvalent when converted to C++11
[[nodiscard]] GG_API inline std::size_t hash_value(X x) { return boost::hash<int>()(Value(x)); }
[[nodiscard]] GG_API inline std::size_t hash_value(Y y) { return boost::hash<int>()(Value(y)); }
[[nodiscard]] GG_API inline std::size_t hash_value(Pt pt) {
    std::size_t seed(0);
    boost::hash_combine(seed, pt.x);
    boost::hash_combine(seed, pt.y);
    return seed;
}
[[nodiscard]] GG_API inline std::size_t hash_value(Rect r) {
    std::size_t seed(0);
    boost::hash_combine(seed, r.ul);
    boost::hash_combine(seed, r.lr);
    return seed;
}

}


#endif
