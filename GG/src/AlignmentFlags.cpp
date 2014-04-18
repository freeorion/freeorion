/* GG is a GUI for SDL and OpenGL.
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

#include <GG/AlignmentFlags.h>


using namespace GG;

///////////////////////////////////////
// Alignment
///////////////////////////////////////

const Alignment GG::ALIGN_NONE         (0);
const Alignment GG::ALIGN_VCENTER      (1 << 0);
const Alignment GG::ALIGN_TOP          (1 << 1);
const Alignment GG::ALIGN_BOTTOM       (1 << 2);
const Alignment GG::ALIGN_CENTER       (1 << 3);
const Alignment GG::ALIGN_LEFT         (1 << 4);
const Alignment GG::ALIGN_RIGHT        (1 << 5);

GG_FLAGSPEC_IMPL(Alignment);

namespace {
    bool RegisterAlignments()
    {
        FlagSpec<Alignment>& spec = FlagSpec<Alignment>::instance();
        spec.insert(ALIGN_NONE,     "ALIGN_NONE",   true);
        spec.insert(ALIGN_VCENTER,  "ALIGN_VCENTER",true);
        spec.insert(ALIGN_TOP,      "ALIGN_TOP",    true);
        spec.insert(ALIGN_BOTTOM,   "ALIGN_BOTTOM", true);
        spec.insert(ALIGN_CENTER,   "ALIGN_CENTER", true);
        spec.insert(ALIGN_LEFT,     "ALIGN_LEFT",   true);
        spec.insert(ALIGN_RIGHT,    "ALIGN_RIGHT",  true);
        return true;
    }
    bool dummy = RegisterAlignments();
}
