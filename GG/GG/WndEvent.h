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

/** \file WndEvent.h \brief Contains the WndEvent class, which is used
    internally by GG to dispatch event messages to Wnds. */

#ifndef _GG_WndEvent_h_
#define _GG_WndEvent_h_

#include <GG/Base.h>
#include <GG/Exception.h>
#include <GG/Flags.h>

#include <map>


namespace GG {

class Timer;
class Wnd;

// Adpated from SDLKey enum in SDL_keysym.h of the SDL library.
GG_FLAG_TYPE(ModKey);
extern GG_API const ModKey MOD_KEY_NONE;         ///< No modifier key.
extern GG_API const ModKey MOD_KEY_LSHIFT;       ///< The left Shift key.
extern GG_API const ModKey MOD_KEY_RSHIFT;       ///< The right Shift key.
extern GG_API const ModKey MOD_KEY_LCTRL;        ///< The left Control key.
extern GG_API const ModKey MOD_KEY_RCTRL;        ///< The right Control key.
extern GG_API const ModKey MOD_KEY_LALT;         ///< The left Alt key.
extern GG_API const ModKey MOD_KEY_RALT;         ///< The right Alt key.
extern GG_API const ModKey MOD_KEY_LMETA;        ///< The left Meta key.
extern GG_API const ModKey MOD_KEY_RMETA;        ///< The right Meta key.
extern GG_API const ModKey MOD_KEY_NUM;          ///< The Num Lock key.
extern GG_API const ModKey MOD_KEY_CAPS;         ///< The Caps Lock key.
extern GG_API const ModKey MOD_KEY_MODE;         ///< The Mode key.
extern GG_API const Flags<ModKey> MOD_KEY_CTRL;  ///< Either Control key.
extern GG_API const Flags<ModKey> MOD_KEY_SHIFT; ///< Either Shift key.
extern GG_API const Flags<ModKey> MOD_KEY_ALT;   ///< Either Alt key.
extern GG_API const Flags<ModKey> MOD_KEY_META;  ///< Either Meta key.

/** \brief Encapsulates a Wnd event that is passed from the singleton GUI to a
    Wnd.

    The various types of WndEvents correspond to the various message member
    functions of Wnd, some of which have different parameterizations.  Rather
    than have a less-efficient but more-easily-extensible hierarchy of
    WndEvent types, a single WndEvent type exists that has all possible
    parameters to a Wnd message function call.  Therefore, not all of
    WndEvent's accessors will return sensical results, depending on the
    WndEventType of the WndEvent.  Note that Wnd events may be filtered before
    they actually reach the target Wnd \see Wnd */
class GG_API WndEvent
{
public:
    /** The types of Wnd events.  Each of these corresponds to a Wnd member
        function of the same name. */
    enum EventType {
        LButtonDown,
        LDrag,
        LButtonUp,
        LClick,
        LDoubleClick,
        MButtonDown,
        MDrag,
        MButtonUp,
        MClick,
        MDoubleClick,
        RButtonDown,
        RDrag,
        RButtonUp,
        RClick,
        RDoubleClick,
        MouseEnter,
        MouseHere,
        MouseLeave,
        MouseWheel,
        DragDropEnter,
        DragDropHere,
        DragDropLeave,
        KeyPress,
        KeyRelease,
        GainingFocus,
        LosingFocus,
        TimerFiring
    };

    /** Constructs an WndEvent that is used to invoke a function taking
        parameters (const GG::Pt& pt, Flags<ModKey> mod_keys), eg
        LButtonDown(). */
    WndEvent(EventType type, const Pt& pt, Flags<ModKey> mod_keys);

    /** Constructs an WndEvent that is used to invoke a function taking
        parameters (const Pt& pt, const Pt& move, Flags<ModKey> mod_keys), eg
        LDrag(). */
    WndEvent(EventType type, const Pt& pt, const Pt& move, Flags<ModKey> mod_keys);

    /** Constructs an WndEvent that is used to invoke a function taking
        parameters (const Pt& pt, int move, Flags<ModKey> mod_keys), eg
        MouseWheel(). */
    WndEvent(EventType type, const Pt& pt, int move, Flags<ModKey> mod_keys);

    /** Constructs an WndEvent that is used to invoke a function taking
        parameters (const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds,
        Flags<ModKey> mod_keys), eg DragDropEnter(). */
    WndEvent(EventType type, const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys);

    /** Constructs an WndEvent that is used to invoke a function taking
        parameters (Key key, Flags<ModKey> mod_keys), eg KeyPress(). */
    WndEvent(EventType type, Key key, boost::uint32_t code_point, Flags<ModKey> mod_keys);

    /** Constructs an WndEvent that is used to invoke a function taking
        parameters (unsigned int, Timer*), eg TimerFiring(). */
    WndEvent(EventType type, unsigned int ticks, Timer* timer);

    /** Constructs an WndEvent that is used to invoke a function taking no
        parameters, eg GainingFocus(). */
    explicit WndEvent(EventType type);

    EventType                 Type() const;         ///< returns the type of the WndEvent
    const Pt&                 Point() const;        ///< returns the point at which the event took place, if any
    Key                       GetKey() const;       ///< returns the key pressed or released in the WndEvent, if any
    boost::uint32_t           KeyCodePoint() const; ///< returns the Unicode code point for the key pressed or released in the WndEvent, if any.  \note This may be zero, even in a KeyPress or KeyRelease event, if Unicode support is unavailable.
    Flags<ModKey>             ModKeys() const;      ///< returns the modifiers to the WndEvent's keypress, if any
    const Pt&                 DragMove() const;     ///< returns the amount of drag movement represented by the WndEvent, if any
    int                       WheelMove() const;    ///< returns the ammount of mouse wheel movement represented by the WndEvent, if any
    const std::map<Wnd*, Pt>& DragDropWnds() const; ///< returns the drag-and-drop wnds represented by the WndEvent, if any
    unsigned int              Ticks() const;        ///< returns the number of ticks represented by the WndEvent. if any
    Timer*                    GetTimer() const;     ///< returns the Timer represented by the WndEvent. if any

private:
    EventType          m_type;
    Pt                 m_point;
    Key                m_key;
    boost::uint32_t    m_key_code_point;
    Flags<ModKey>      m_mod_keys;
    Pt                 m_drag_move;
    int                m_wheel_move;
    std::map<Wnd*, Pt> m_drag_drop_wnds;
    unsigned int       m_ticks;
    Timer*             m_timer;
};

} // namespace GG

#endif
