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

#include <GG/WndEvent.h>


using namespace GG;

///////////////////////////////////////
// ModKeys
///////////////////////////////////////
const ModKey GG::MOD_KEY_NONE         (0x0000);
const ModKey GG::MOD_KEY_LSHIFT       (0x0001);
const ModKey GG::MOD_KEY_RSHIFT       (0x0002);
const ModKey GG::MOD_KEY_LCTRL        (0x0040);
const ModKey GG::MOD_KEY_RCTRL        (0x0080);
const ModKey GG::MOD_KEY_LALT         (0x0100);
const ModKey GG::MOD_KEY_RALT         (0x0200);
const ModKey GG::MOD_KEY_LMETA        (0x0400);
const ModKey GG::MOD_KEY_RMETA        (0x0800);
const ModKey GG::MOD_KEY_NUM          (0x1000);
const ModKey GG::MOD_KEY_CAPS         (0x2000);
const ModKey GG::MOD_KEY_MODE         (0x4000);

namespace {
    bool RegisterModKeys()
    {
        FlagSpec<ModKey>& spec = FlagSpec<ModKey>::instance();
        spec.insert(MOD_KEY_NONE,   "MOD_KEY_NONE",     true);
        spec.insert(MOD_KEY_LSHIFT, "MOD_KEY_LSHIFT",   true);
        spec.insert(MOD_KEY_RSHIFT, "MOD_KEY_RSHIFT",   true);
        spec.insert(MOD_KEY_LCTRL,  "MOD_KEY_LCTRL",    true);
        spec.insert(MOD_KEY_RCTRL,  "MOD_KEY_RCTRL",    true);
        spec.insert(MOD_KEY_LALT,   "MOD_KEY_LALT",     true);
        spec.insert(MOD_KEY_RALT,   "MOD_KEY_RALT",     true);
        spec.insert(MOD_KEY_LMETA,  "MOD_KEY_LMETA",    true);
        spec.insert(MOD_KEY_RMETA,  "MOD_KEY_RMETA",    true);
        spec.insert(MOD_KEY_NUM,    "MOD_KEY_NUM",      true);
        spec.insert(MOD_KEY_CAPS,   "MOD_KEY_CAPS",     true);
        spec.insert(MOD_KEY_MODE,   "MOD_KEY_MODE",     true);
        return true;
    }
    bool dummy = RegisterModKeys();
}

const Flags<ModKey> GG::MOD_KEY_CTRL  ((MOD_KEY_LCTRL | MOD_KEY_RCTRL));
const Flags<ModKey> GG::MOD_KEY_SHIFT ((MOD_KEY_LSHIFT | MOD_KEY_RSHIFT));
const Flags<ModKey> GG::MOD_KEY_ALT   ((MOD_KEY_LALT | MOD_KEY_RALT));
const Flags<ModKey> GG::MOD_KEY_META  ((MOD_KEY_LMETA | MOD_KEY_RMETA));

GG_FLAGSPEC_IMPL(ModKey);


///////////////////////////////////////
// class GG::WndEvent
///////////////////////////////////////
WndEvent::WndEvent(EventType type, const Pt& pt, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(mod_keys),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0),
    m_text(0)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const Pt& move, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(mod_keys),
    m_drag_move(move),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0),
    m_text(0)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, int move, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(mod_keys),
    m_wheel_move(move),
    m_ticks(0),
    m_timer(0),
    m_text(0)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(mod_keys),
    m_wheel_move(0),
    m_drag_drop_wnds(drag_drop_wnds),
    m_ticks(0),
    m_timer(0),
    m_text(0)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const std::vector<Wnd*>& drag_drop_wnds) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0),
    m_text(0),
    m_dropped_wnds(drag_drop_wnds)
{}

WndEvent::WndEvent(EventType type, Key key, boost::uint32_t code_point, Flags<ModKey> mod_keys) :
    m_type(type),
    m_key(key),
    m_key_code_point(code_point),
    m_mod_keys(mod_keys),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0),
    m_text(0)
{}

WndEvent::WndEvent(EventType type, unsigned int ticks, Timer* timer) :
    m_type(type),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(),
    m_wheel_move(0),
    m_ticks(ticks),
    m_timer(timer),
    m_text(0)
{}

WndEvent::WndEvent (WndEvent::EventType type, const std::string* text):
    m_type(type),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0),
    m_text(text)
{}

WndEvent::WndEvent(EventType type) :
    m_type(type),
    m_key(GGK_UNKNOWN),
    m_key_code_point(0),
    m_mod_keys(), 
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0),
    m_text(0)
{}

WndEvent::EventType WndEvent::Type() const
{ return m_type; }

const Pt& WndEvent::Point() const
{ return m_point; }

Key WndEvent::GetKey() const
{ return m_key; }

boost::uint32_t WndEvent::KeyCodePoint() const
{ return m_key_code_point; }

Flags<ModKey> WndEvent::ModKeys() const
{ return m_mod_keys; }

const Pt& WndEvent::DragMove() const
{ return m_drag_move; }

int WndEvent::WheelMove() const
{ return m_wheel_move; }

const std::map<Wnd*, Pt>& WndEvent::DragDropWnds() const
{ return m_drag_drop_wnds; }

std::vector<Wnd*>& WndEvent::GetDragDropWnds() const
{ return m_dropped_wnds; }

unsigned int WndEvent::Ticks() const
{ return m_ticks; }

Timer* WndEvent::GetTimer() const
{ return m_timer; }

const std::string* WndEvent::GetText() const
{ return m_text; }

