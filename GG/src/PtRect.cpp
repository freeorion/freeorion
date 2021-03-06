//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/PtRect.h>


using namespace GG;

////////////////////////////////////////////////
// GG::Pt
////////////////////////////////////////////////
std::ostream& GG::operator<<(std::ostream& os, const Pt& pt)
{
    os << "(" << pt.x << ", " << pt.y << ")";
    return os;
}


////////////////////////////////////////////////
// GG::Rect
////////////////////////////////////////////////
std::ostream& GG::operator<<(std::ostream& os, const Rect& rect)
{
    os << "[" << rect.ul << " - " << rect.lr << "]";
    return os;
}
