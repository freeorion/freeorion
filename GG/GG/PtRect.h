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

/** \file PtRect.h \brief Contains the utility classes Pt and Rect. */

#ifndef _GG_PtRect_h_
#define _GG_PtRect_h_

#include <GG/Base.h>
#include <GG/StrongTypedef.h>
#include <boost/functional/hash.hpp>


namespace GG {

/** \class GG::X
    \brief The x-coordinate value type.

    X has an underlying value type of int.  \see GG_STRONG_INTEGRAL_TYPEDEF */
GG_STRONG_INTEGRAL_TYPEDEF(X, int);

/** \class GG::Y
    \brief The y-coordinate value type.

    Y has an underlying value type of int.  \see GG_STRONG_INTEGRAL_TYPEDEF */
GG_STRONG_INTEGRAL_TYPEDEF(Y, int);

// some useful coordinate constants
extern GG_API const X X0;
extern GG_API const X X1;
extern GG_API const Y Y0;
extern GG_API const Y Y1;

/** \brief A GG screen coordinate class. */
struct GG_API Pt
{
    /** \name Structors */ ///@{
    Pt();

    Pt(X x_, Y y_);     ///< Ctor that creates a Pt ( \a _x , \a y ).
    Pt(X_d x_, Y y_);   ///< Ctor that creates a Pt ( \a _x , \a y ).
    Pt(X x_, Y_d y_);   ///< Ctor that creates a Pt ( \a _x , \a y ).
    Pt(X_d x_, Y_d y_); ///< Ctor that creates a Pt ( \a _x , \a y ).
    //@}

    /** \name Accessors */ ///@{
    /** Returns true if x < \a rhs.x or returns true if x == \a rhs.x and y
        <\a rhs.y.  This is useful for sorting Pts in STL containers and
        algorithms. */
    bool Less(const Pt& rhs) const
        { return x < rhs.x ? true : (x == rhs.x ? (y < rhs.y ? true : false) : false); }
    //@}

    /** \name Mutators */ ///@{
    void  operator+=(const Pt& rhs)     { x += rhs.x; y += rhs.y; }     ///< Adds \a rhs to Pt.
    void  operator-=(const Pt& rhs)     { x -= rhs.x; y -= rhs.y; }     ///< Subtracts \a rhs from Pt.
    Pt    operator-() const             { return Pt(-x, -y); }          ///< Negates Pt.
    Pt    operator/=(const double rhs)  { return Pt(x / rhs, y / rhs); }///< Devides components of Pt by \a rhs
    Pt    operator*=(const double rhs)  { return Pt(x * rhs, y * rhs); }///< Devides components of Pt by \a rhs
    //@}

    X x; ///< The x component.
    Y y; ///< The y component.
};

GG_API std::ostream& operator<<(std::ostream& os, const Pt& pt);


/** \brief A GG rectangle class.

    This is essentially just two points that bound the rectangle. */
struct GG_API Rect
{
    /** \name Structors */ ///@{
    Rect();

    Rect(const Pt& pt1, const Pt& pt2);    ///< ctor that constructs a Rect from two corners; any two opposing corners will do
    Rect(X x1, Y y1, X x2, Y y2);  ///< ctor that constructs a Rect from its left, upper, right, and bottom boundaries
    //@}

    /** \name Accessors */ ///@{
    X   Left() const        { return ul.x; }            ///< returns the left boundary of the Rect
    X   Right() const       { return lr.x; }            ///< returns the right boundary of the Rect
    Y   Top() const         { return ul.y; }            ///< returns the top boundary of the Rect
    Y   Bottom() const      { return lr.y; }            ///< returns the bottom boundary of the Rect
    Pt  UpperLeft() const   { return ul; }              ///< returns the upper-left corner of the Rect
    Pt  LowerRight() const  { return lr; }              ///< returns the lower-right corner of the Rect
    X   Width() const       { return lr.x - ul.x; }     ///< returns the width of the Rect
    Y   Height() const      { return lr.y - ul.y; }     ///< returns the height of the Rect
    X   MidX() const        { return (lr.x + ul.x)/2; } ///< returns the horizontal mid-point of the Rect
    Y   MidY() const        { return (lr.y + ul.y)/2; } ///< returns the vertical mid-point of the Rect


    bool  Contains(const Pt& pt) const; ///< returns true iff \a pt falls inside the Rect
    //@}

    /** \name Mutators */ ///@{
    void operator+=(const Pt& pt)      { ul += pt; lr += pt; } ///< shifts the Rect by adding \a pt to each corner
    void operator-=(const Pt& pt)      { ul -= pt; lr -= pt; } ///< shifts the Rect by subtracting \a pt from each corner
    //@}

    Pt ul; ///< the upper-left corner of the Rect
    Pt lr; ///< the lower-right corner of the Rect
};

GG_API inline bool operator==(const Pt& lhs, const Pt& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; } ///< returns true if \a lhs is identical to \a rhs
GG_API inline bool operator!=(const Pt& lhs, const Pt& rhs) { return !(lhs == rhs); }                    ///< returns true if \a lhs differs from \a rhs
GG_API inline bool operator<(const Pt& lhs, const Pt& rhs)  { return lhs.x < rhs.x && lhs.y < rhs.y; }   ///< returns true if \a lhs.x and \a lhs.y are both less than the corresponding components of \a rhs
GG_API inline bool operator>(const Pt& lhs, const Pt& rhs)  { return lhs.x > rhs.x && lhs.y > rhs.y; }   ///< returns true if \a lhs.x and \a lhs.y are both greater than the corresponding components of \a rhs
GG_API inline bool operator<=(const Pt& lhs, const Pt& rhs) { return lhs.x <= rhs.x && lhs.y <= rhs.y; } ///< returns true if \a lhs.x and \a lhs.y are both less than or equal to the corresponding components of \a rhs
GG_API inline bool operator>=(const Pt& lhs, const Pt& rhs) { return lhs.x >= rhs.x && lhs.y >= rhs.y; } ///< returns true if \a lhs.x and \a lhs.y are both greater than or equal to the corresponding components of \a rhs
GG_API inline Pt   operator+(const Pt& lhs, const Pt& rhs)  { return Pt(lhs.x + rhs.x, lhs.y + rhs.y); } ///< returns the vector sum of \a lhs and \a rhs
GG_API inline Pt   operator-(const Pt& lhs, const Pt& rhs)  { return Pt(lhs.x - rhs.x, lhs.y - rhs.y); } ///< returns the vector difference of \a lhs and \a rhs
GG_API inline Pt   operator*(const Pt& lhs, double rhs)     { return Pt(lhs.x * rhs, lhs.y * rhs); }     ///< returns the vector with components multiplied by \a rhs
GG_API inline Pt   operator/(const Pt& lhs, double rhs)     { return Pt(lhs.x / rhs, lhs.y / rhs); }     ///< returns the vector with components divided by \a rhs

GG_API std::ostream& operator<<(std::ostream& os, const Pt& pt); ///< Pt stream-output operator for debug output

/** returns true if \a lhs is identical to \a rhs */
GG_API inline bool operator==(const Rect& lhs, const Rect& rhs) { return lhs.ul.x == rhs.ul.x && lhs.lr.x == rhs.lr.x && lhs.ul.y == rhs.ul.y && lhs.lr.y == rhs.lr.y; }

/** returns true if \a lhs differs from \a rhs */
GG_API inline bool operator!=(const Rect& lhs, const Rect& rhs) { return !(lhs == rhs); }

GG_API inline Rect operator+(const Rect& rect, const Pt& pt) { return Rect(rect.ul + pt, rect.lr + pt); } ///< returns \a rect shifted by adding \a pt to each corner
GG_API inline Rect operator-(const Rect& rect, const Pt& pt) { return Rect(rect.ul - pt, rect.lr - pt); } ///< returns \a rect shifted by subtracting \a pt from each corner
GG_API inline Rect operator+(const Pt& pt, const Rect& rect) { return rect + pt; } ///< returns \a rect shifted by adding \a pt to each corner
GG_API inline Rect operator-(const Pt& pt, const Rect& rect) { return rect - pt; } ///< returns \a rect shifted by subtracting \a pt from each corner

GG_API std::ostream& operator<<(std::ostream& os, const Rect& rect); ///< Rect stream-output operator for debug output

    // Hash functions
    // Replace with C++11 equilvalent when converted to C++11
    GG_API inline std::size_t hash_value(X const& x) { return boost::hash<int>()(Value(x)); }
    GG_API inline std::size_t hash_value(Y const& y) { return boost::hash<int>()(Value(y)); }
    GG_API inline std::size_t hash_value(Pt const& pt) {
        std::size_t seed(0);
        boost::hash_combine(seed, pt.x);
        boost::hash_combine(seed, pt.y);
        return seed;
    }
    GG_API inline std::size_t hash_value(Rect const& r) {
        std::size_t seed(0);
        boost::hash_combine(seed, r.ul);
        boost::hash_combine(seed, r.lr);
        return seed;
    }

} // namepace GG

#endif
