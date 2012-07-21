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
   
/** \file EveLayout.h TODO. */

#ifndef _EveLayout_h_
#define _EveLayout_h_

#include <GG/adobe/eve_parser.hpp>
#include <GG/adobe/dictionary_fwd.hpp>

#include <iosfwd>


namespace adobe {
    class sheet_t;
}

namespace boost {
    class any;
}

namespace GG {

class Wnd;

class EveLayout
{
public:
    EveLayout(adobe::sheet_t& sheet);
    ~EveLayout();

    adobe::dictionary_t Contributing() const;

    void Print(std::ostream& os) const;

    adobe::eve_callback_suite_t BindCallbacks();

    void AddCell(adobe::eve_callback_suite_t::cell_type_t type,
                 adobe::name_t name,
                 const adobe::line_position_t& position,
                 const adobe::array_t& initializer,
                 const std::string& brief,
                 const std::string& detailed);

    boost::any AddView(const boost::any& parent,
                       const adobe::line_position_t& position,
                       adobe::name_t name,
                       const adobe::array_t& parameters,
                       const std::string& brief,
                       const std::string& detailed);

    Wnd& Finish();

private:
    class Impl;
    Impl* m_impl;
};

}

#endif
