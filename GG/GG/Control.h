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

/** \file Control.h \brief Contains the Control class, the base class for all
    GG controls. */

#ifndef _GG_Control_h_
#define _GG_Control_h_

#include <GG/Wnd.h>


namespace GG {

/** \brief An abstract base class for all control classes.

    Each control has (like all windows) coordinates offset from the upper-left
    corner of it's parent's client area.  All controls may be disabled.  By
    default, a Control forwards several types of events and requests for
    action to its parent Wnd (e.g. AcceptDrops()).  In particular, keyboard
    input not handled by the Control is forwarded to the Control's parent.
    Any class derived from Control should do the same with any keyboard input
    it does not need for its own use.  For instance, an Edit control needs to
    know about arrow key keyboard input, but it should pass other key presses
    like 'ESC' to its parent. */
class GG_API Control : public Wnd
{
public:
    /** \name Accessors */ ///@{
    Clr Color() const;      ///< returns the color of the control
    bool Disabled() const;  ///< returns true if the control is disabled, false otherwise
    //@}

    /** \name Mutators */ ///@{
    /** Sets the color of the control. */
    virtual void SetColor(Clr c);

    /** Disables/enables the control; disabled controls appear greyed. */
    virtual void Disable(bool b = true);
    //@}

protected:
    /** \name Structors */ ///@{
    Control();

    Control(X x, Y y, X w, Y h, Flags<WndFlag> flags = INTERACTIVE);
    //@}

    void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void KeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;

    Clr  m_color;    ///< the color of the control
    bool m_disabled; ///< whether or not this control is disabled
};

} // namespace GG

#endif
