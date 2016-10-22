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
   
/** \file Export.h \brief Contains the GG_API macro, which is used to specify
    which class and function symbols will be exported in SO's/DLL's. */

#ifndef _GG_Export_h_
#define _GG_Export_h_

#ifndef GG_API
# ifdef _MSC_VER
#  ifdef GiGi_EXPORTS
#   define GG_API __declspec(dllexport)
#  else
#   define GG_API __declspec(dllimport)
#  endif
# else
#  define GG_API
# endif
#endif

#endif
