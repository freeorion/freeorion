//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Export.h
//!
//! Contains the GG_API macro, which is used to specify which class and function
//! symbols will be exported in SO's/DLL's.

#ifndef _GG_Export_h_
#define _GG_Export_h_

#ifndef GG_API
# ifdef _MSC_VER
#  ifdef GiGi_EXPORTS
#   define GG_API __declspec(dllexport)
#  else
#   define GG_API __declspec(dllimport)
#  endif
# endif
# ifdef __GNUC__
#  define GG_API __attribute__((__visibility__("default")))
# endif
#endif

#endif
