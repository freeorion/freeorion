//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2007 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

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
