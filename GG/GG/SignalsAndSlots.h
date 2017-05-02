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

#ifndef _GG_SignalsAndSlots_h_
#define _GG_SignalsAndSlots_h_

#include <boost/bind.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/preprocessor/cat.hpp>

#include <GG/Signal0.h>
#include <GG/Signal1.h>
#include <GG/Signal2.h>
#include <GG/Signal3.h>
#include <GG/Signal4.h>
#include <GG/Signal5.h>
#include <GG/Signal6.h>
#include <GG/Signal7.h>
#include <GG/Signal8.h>

/** \file SignalsAndSlots.h \brief Contains the Connect() functions, which
    simplify the connection of boost signals and slots. */

namespace GG {

/** Connects a signal to a slot functor of the same signature, putting \a
    _slot in slot group 0, at position \a at within group 0.  Slot call groups
    are called in ascending order. */
template <class SigT> inline
boost::signals2::connection
Connect(SigT& sig, const typename SigT::slot_type& _slot, boost::signals2::connect_position at = boost::signals2::at_back)
{
    return sig.connect(_slot, at);
}

/** Connects a signal to a slot functor of the same signature, putting \a
    _slot in slot group \a grp, at position \a at within group \a grp.  Slot
    call groups are called in ascending order. */
template <class SigT> inline
boost::signals2::connection
Connect(SigT& sig, const typename SigT::slot_type& _slot, int grp, boost::signals2::connect_position at = boost::signals2::at_back)
{
    return sig.connect(grp, _slot, at);
}

} // namespace GG


#endif


