//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Clr.h
//!
//! Contains the utility class Clr, which represents colors in GG.

#ifndef _GG_Clr_h_
#define _GG_Clr_h_


#include <sstream>
#include <stdexcept>
#include <string>
#include <GG/Export.h>


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

    unsigned char r;   ///< the red channel
    unsigned char g;   ///< the green channel
    unsigned char b;   ///< the blue channel
    unsigned char a;   ///< the alpha channel
};

GG_API std::ostream& operator<<(std::ostream& os, const Clr& pt);

//! Returns the lightened version of color clr.  LightenClr leaves the alpha
//! channel unchanged, and multiplies the other channels by some factor.
inline Clr LightenClr(const Clr& clr, float factor = 2.0)
{
    return Clr(
        std::min(static_cast<int>(clr.r * factor), 255),
        std::min(static_cast<int>(clr.g * factor), 255),
        std::min(static_cast<int>(clr.b * factor), 255),
        clr.a);
}

//! Returns the darkened version of color clr.  DarkenClr leaves the alpha
//! channel unchanged, and divides the other channels by some factor.
inline Clr DarkenClr(const Clr& clr, float factor = 2.0)
{
    return Clr(
        static_cast<int>(clr.r / factor),
        static_cast<int>(clr.g / factor),
        static_cast<int>(clr.b / factor),
        clr.a);
}

inline Clr InvertClr(const Clr& clr)
{
    return Clr(255 - clr.r,
               255 - clr.g,
               255 - clr.b,
               clr.a);
}

inline Clr BlendClr(const Clr& src, const Clr& dst, float factor)
{
    return Clr(static_cast<unsigned char>(src.r * factor + dst.r * (1 - factor)),
               static_cast<unsigned char>(src.g * factor + dst.g * (1 - factor)),
               static_cast<unsigned char>(src.b * factor + dst.b * (1 - factor)),
               static_cast<unsigned char>(src.a * factor + dst.a * (1 - factor)));
}

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

}


#endif
