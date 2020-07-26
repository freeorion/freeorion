//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

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
    Clr Color() const;      ///< returns the color of the control
    bool Disabled() const;  ///< returns true if the control is disabled, false otherwise

    /** Sets the color of the control. */
    virtual void SetColor(Clr c);

    /** Disables/enables the control; disabled controls appear greyed. */
    virtual void Disable(bool b = true);

protected:
    Control();
    Control(X x, Y y, X w, Y h, Flags<WndFlag> flags = INTERACTIVE);

    void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void KeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;

    Clr  m_color;               ///< the color of the control
    bool m_disabled = false;    ///< whether or not this control is disabled
};

}

#endif
