//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Control.h>
#include <GG/WndEvent.h>


using namespace GG;

////////////////////////////////////////////////
// GG::Control
////////////////////////////////////////////////
void Control::MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }

void Control::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }

void Control::KeyRelease(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }
