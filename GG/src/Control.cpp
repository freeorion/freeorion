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

#include <GG/Control.h>

#include <GG/WndEvent.h>

using namespace GG;

////////////////////////////////////////////////
// GG::Control
////////////////////////////////////////////////
Control::Control() :
    Wnd (),
    m_disabled(false)
{}

Control::Control(X x, Y y, X w, Y h, Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Wnd(x, y, w, h, flags),
    m_disabled(false)
{}

void Control::DropsAcceptable(DropsAcceptableIter first,
                              DropsAcceptableIter last,
                              const Pt& pt) const
{
    if (Parent())
        Parent()->DropsAcceptable(first, last, pt);
}

Clr Control::Color() const
{ return m_color; }

bool Control::Disabled() const
{ return m_disabled; }

void Control::AcceptDrops(const std::vector<Wnd*>& wnds, const Pt& pt)
{
    if (Parent())
        Parent()->AcceptDrops(wnds, pt);
}

void Control::SetColor(Clr c)
{ m_color = c; }

void Control::Disable(bool b/* = true*/)
{ m_disabled = b; }

void Control::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }

void Control::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }

void Control::KeyRelease(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }
