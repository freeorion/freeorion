//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2007 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/AlignmentFlags.h>


///////////////////////////////////////
// Alignment
///////////////////////////////////////
namespace GG {
GG_FLAGSPEC_IMPL(Alignment);

bool RegisterAlignments()
{
    FlagSpec<Alignment>& spec = FlagSpec<Alignment>::instance();
    spec.insert(ALIGN_NONE,     "ALIGN_NONE");
    spec.insert(ALIGN_VCENTER,  "ALIGN_VCENTER");
    spec.insert(ALIGN_TOP,      "ALIGN_TOP");
    spec.insert(ALIGN_BOTTOM,   "ALIGN_BOTTOM");
    spec.insert(ALIGN_CENTER,   "ALIGN_CENTER");
    spec.insert(ALIGN_LEFT,     "ALIGN_LEFT");
    spec.insert(ALIGN_RIGHT,    "ALIGN_RIGHT");
    return true;
}
bool dummy = RegisterAlignments();

}
