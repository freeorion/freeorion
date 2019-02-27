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

/** \file Clr.h \brief Contains the utility class Clr, which represents colors
    in GG. */

#ifndef _GG_Clr_h_
#define _GG_Clr_h_

#include <GG/Export.h>

#include <string>
#include <stdexcept>
#include <sstream>


namespace GG {

/** \brief A simple 32-bit structure that can act as a packed 32-bit unsigned
    integer representation of a RGBA color, a vector of the four unsigned
    bytes that compose an RGBA color, or the individual unsigned bytes "a",
    "r", "g", and "b".

    You should not use literals to initialize Color objects; depending on the
    endian-ness of the machine, 0x00FFFFFF would be transparent white
    (little-endian) or opaque yellow (big-endian).*/
struct Clr
{
    /** \name Structors */ ///@{
    Clr() :
        r(0), g(0), b(0), a(0)
        {}

    /** ctor that constructs a Clr from four ints that represent the color channels */
    constexpr Clr(unsigned char r_,
                  unsigned char g_,
                  unsigned char b_,
                  unsigned char a_) :
        r(r_), g(g_), b(b_), a(a_)
        {}
    //@}

    unsigned char r;   ///< the red channel
    unsigned char g;   ///< the green channel
    unsigned char b;   ///< the blue channel
    unsigned char a;   ///< the alpha channel
};

GG_API std::ostream& operator<<(std::ostream& os, const Clr& pt);


/** Named ctor that constructs a Clr from four floats that represent the color
    channels (each must be >= 0.0 and <= 1.0). */
inline Clr FloatClr(float r, float g, float b, float a)
{
    return Clr(static_cast<unsigned char>(r * 255),
               static_cast<unsigned char>(g * 255),
               static_cast<unsigned char>(b * 255),
               static_cast<unsigned char>(a * 255));
}

/** Named ctor that constructs a Clr from a string that represents the color
    channels in the format '#RRGGBB', '#RRGGBBAA' where each channel value
    ranges from 0 to FF.  When the alpha component is left out the alpha
    value FF is assumed.
    @throws std::invalid_argument if the hex_colour string is not well formed
    */
inline Clr HexClr(const std::string& hex_colour)
{
    std::istringstream iss(hex_colour);

    unsigned long rgba = 0;
    if ((hex_colour.size() == 7 || hex_colour.size() == 9) &&
            '#' == iss.get() && !(iss >> std::hex >> rgba).fail())
    {
        GG::Clr retval = GG::Clr(0, 0, 0, 255);

        if (hex_colour.size() == 7) {
            retval.r = (rgba >> 16) & 0xFF;
            retval.g = (rgba >> 8)  & 0xFF;
            retval.b = rgba         & 0xFF;
            retval.a = 255;
        } else {
            retval.r = (rgba >> 24) & 0xFF;
            retval.g = (rgba >> 16) & 0xFF;
            retval.b = (rgba >> 8)  & 0xFF;
            retval.a = rgba         & 0xFF;
        }

        return retval;
    }

    throw std::invalid_argument("GG::HexClr could not interpret hex colour string");
}

/** Returns true iff \a rhs and \a lhs are identical. */
inline bool operator==(const Clr& rhs, const Clr& lhs)
{ return rhs.r == lhs.r && rhs.g == lhs.g && rhs.b == lhs.b && rhs.a == lhs.a; }

/** Returns true iff \a rhs and \a lhs are different. */
inline bool operator!=(const Clr& rhs, const Clr& lhs)
{ return !(rhs == lhs); }

/** Returns the input Clr scaned by the input factor \a s. */
inline Clr operator*(const Clr& lhs, float s)
{
    return Clr(static_cast<unsigned char>(lhs.r * s),
               static_cast<unsigned char>(lhs.g * s),
               static_cast<unsigned char>(lhs.b * s),
               static_cast<unsigned char>(lhs.a * s));
}

/** Returns the component-wise sum of input Clrs. */
inline Clr operator+(const Clr& lhs, const Clr& rhs)
{ return Clr(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a); }

/** Clr comparisons */
inline bool operator<(const Clr& lhs, const Clr& rhs)
{
    if (rhs.r != lhs.r)
        return rhs.r < lhs.r;
    if (rhs.g != lhs.g)
        return rhs.g < lhs.g;
    if (rhs.b != lhs.b)
        return rhs.b < lhs.b;
    return rhs.a < lhs.a;
}

} // namespace GG

#endif

