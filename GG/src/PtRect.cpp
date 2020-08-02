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

const X GG::X0(0);
const X GG::X1(1);
const Y GG::Y0(0);
const Y GG::Y1(1);

////////////////////////////////////////////////
// GG::Pt
////////////////////////////////////////////////
Pt::Pt() :
    x(0),
    y(0)
{}

Pt::Pt(X x_, Y y_) :
    x(x_),
    y(y_)
{}

Pt::Pt(X_d x_, Y y_) :
    x(x_),
    y(y_)
{}

Pt::Pt(X x_, Y_d y_) :
    x(x_),
    y(y_)
{}

Pt::Pt(X_d x_, Y_d y_) :
    x(x_),
    y(y_)
{}

std::ostream& GG::operator<<(std::ostream& os, const Pt& pt)
{
    os << "(" << pt.x << ", " << pt.y << ")";
    return os;
}


////////////////////////////////////////////////
// GG::Rect
////////////////////////////////////////////////
Rect::Rect()
{}

Rect::Rect(const Pt& pt1, const Pt& pt2)
{
    ul.x = std::min(pt1.x, pt2.x);
    ul.y = std::min(pt1.y, pt2.y);
    lr.x = std::max(pt1.x, pt2.x);
    lr.y = std::max(pt1.y, pt2.y);
}

Rect::Rect(X x1, Y y1, X x2, Y y2) :
    ul(Pt(x1, y1)),
    lr(Pt(x2, y2))
{}

bool Rect::Contains(const Pt& pt) const 
{ return ul <= pt && pt < lr; }

std::ostream& GG::operator<<(std::ostream& os, const Rect& rect)
{
    os << "[" << rect.ul << " - " << rect.lr << "]";
    return os;
}
