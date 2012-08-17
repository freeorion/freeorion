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


const bool GG::INSTRUMENT_ALL_SIGNALS = false;

void GG::KeypadKeyToPrintable(Key& key, Flags<ModKey> mod_keys)
{
    using namespace GG;

    if (GGK_KP0 <= key && key <= GGK_KP9 && (mod_keys & MOD_KEY_NUM)) {
        key = Key(GGK_0 + (key - GGK_KP0));
    } else {
        switch (key) {
        case GGK_KP_PERIOD:
            if (mod_keys & MOD_KEY_NUM) key = GGK_PERIOD;
            break;
        case GGK_KP_DIVIDE:   key = GGK_SLASH;    break;
        case GGK_KP_MULTIPLY: key = GGK_ASTERISK; break;
        case GGK_KP_MINUS:    key = GGK_MINUS;    break;
        case GGK_KP_PLUS:     key = GGK_PLUS;     break;
        case GGK_KP_EQUALS:   key = GGK_EQUALS;   break;
        default: break;
        }
    }
}
