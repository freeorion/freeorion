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

/** \file ZList.h \brief Contains the ZList class, which maintains the
    Z-/depth-position of Wnds for GUI. */

#ifndef _GG_ZList_h_
#define _GG_ZList_h_

#include <GG/Base.h>

#include <list>


namespace GG {
    class Wnd;

/** \brief A Z-ordering (depth-ordering) of the windows in the GUI.

    Windows being moved up, inserted, or added to the top of the list are
    checked against other windows at the insertion point and after; if any of
    these windows are modal or on-top windows, the inserted window is placed
    after them if it is not also modal or on-top. Z-values decrease into the
    screen.  Windows in the z-list are kept in front-to-back order.  No
    windows may share the same z-value.  Add, Remove, MoveUp, and MoveDown all
    also add/remove/move all descendent windows.*/
class GG_API ZList : public std::list<Wnd*>
{
public:
    /** \name Accessors */ ///@{
    /** Returns pointer to the window under the point pt; constrains pick to
        modal if nonzero, and ignores \a ignore if nonzero. */
    Wnd* Pick(const Pt& pt, Wnd* modal, const std::set<Wnd*>* ignore = 0) const;
    //@}

    /** \name Mutators */ ///@{
    /** Add() places \a wnd in front of the list. */
    void Add(Wnd* wnd);

    bool Remove(Wnd* wnd);   ///< Removes \a wnd from z-order.

    /** Moves \a wnd from its current position to the beginning of list;
        updates wnd's z-value. */
    bool MoveUp(Wnd* wnd);

    /** Moves \a wnd from its current position to the end of list; updates
        wnd's z-value. */
    bool MoveDown(Wnd* wnd);
    //@}

private:
    Wnd*     PickWithinWindow(const Pt& pt, Wnd* wnd, const std::set<Wnd*>* ignore) const; ///< Returns pointer to the window under the point pt; constrains pick to wnd and its decendents, and ignores \a ignore if nonzero.
    iterator FirstNonOnTop();              ///< Returns iterator to first window in list that is non-on-top (returns end() if none found).
};

} // namespace GG

#endif
