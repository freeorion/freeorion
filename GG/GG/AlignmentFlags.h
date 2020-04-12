// -*- C++ -*-
/* GG is a GUI for OpenGL.
   Copyright (C) 2007 T. Zachary Laine

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

/** \file AlignmentFlags.h \brief Contains the Alignment flag type and the
    global alignment flag constants. */

#ifndef _GG_AlignmentFlags_h_
#define _GG_AlignmentFlags_h_

#include <GG/Base.h>
#include <GG/Exception.h>
#include <GG/Flags.h>


namespace GG {

/** Alignment flags. */
GG_FLAG_TYPE(Alignment);
extern GG_API const Alignment ALIGN_NONE;     ///< No alignment selected.
extern GG_API const Alignment ALIGN_VCENTER;  ///< Vertically-centered.
extern GG_API const Alignment ALIGN_TOP;      ///< Aligned to top.
extern GG_API const Alignment ALIGN_BOTTOM;   ///< Aligned to bottom.
extern GG_API const Alignment ALIGN_CENTER;   ///< Horizontally-centered.
extern GG_API const Alignment ALIGN_LEFT;     ///< Aligned to left.
extern GG_API const Alignment ALIGN_RIGHT;    ///< Aligned to right.

} // namespace GG

#endif
