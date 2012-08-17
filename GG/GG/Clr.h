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


namespace GG {

/** \brief A simple 32-bit structure that can act as a packed 32-bit unsigned
    integer representation of a RGBA color, a vector of the four unsigned
    bytes that compose an RGBA color, or the individual unsigned bytes "a",
    "r", "g", and "b".

    You should not use literals to initialize Color objects; depending on the
    endian-ness of the machine, 0x00FFFFFF would be transparent white
    (little-endian) or opaque yellow (big-endian).*/
struct GG_API Clr
{
    /** \name Structors */ ///@{
    /** default ctor */
    Clr() :
        r(0), g(0), b(0), a(0)
        {}

    /** ctor that constructs a Clr from four ints that represent the color channels */
    Clr(unsigned char r_,
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

/** Named ctor that constructs a Clr from four floats that represent the color
    channels (each must be >= 0.0 and <= 1.0). */
inline Clr FloatClr(float r, float g, float b, float a)
{
    return Clr(static_cast<unsigned char>(r * 255),
               static_cast<unsigned char>(g * 255),
               static_cast<unsigned char>(b * 255),
               static_cast<unsigned char>(a * 255));
}

/** Returns true iff \a rhs and \a lhs are identical. */
inline bool operator==(const Clr& rhs, const Clr& lhs)
{ return rhs.r == lhs.r && rhs.g == lhs.g && rhs.b == lhs.b && rhs.a == lhs.a; }

/** Returns true iff \a rhs and \a lhs are different. */
inline bool operator!=(const Clr& rhs, const Clr& lhs)
{ return !(rhs == lhs); }

} // namespace GG

#endif

