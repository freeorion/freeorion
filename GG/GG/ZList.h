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
#include <boost/optional/optional.hpp>

#include <vector>
#include <list>
#include <set>
#include <memory>
#include <functional>


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
class GG_API ZList
{
public:
    ~ZList();

    /** RenderOrderIterable can be iterated in back to front render order. */
    struct RenderOrderIterable {
        RenderOrderIterable(const std::list<std::shared_ptr<Wnd>>& list) :
            m_list(list)
        {}

        std::list<std::shared_ptr<Wnd>>::const_reverse_iterator begin()
        { return m_list.crbegin(); }

        std::list<std::shared_ptr<Wnd>>::const_reverse_iterator end()
        { return m_list.crend(); }

        private:
        const std::list<std::shared_ptr<Wnd>>& m_list;
    };

    /** \name Accessors */ ///@{
    /** Return a RenderOrderIterable in back to front render order. */
    RenderOrderIterable RenderOrder() const;

    /** Returns pointer to the window under the point pt; constrains pick to
        modal if nonzero, and ignores \a ignore if nonzero. */
    std::shared_ptr<Wnd> Pick(const Pt& pt, const std::shared_ptr<Wnd>& modal, const std::set<Wnd*>* ignore = nullptr) const;
    std::shared_ptr<Wnd> Pick(const Pt& pt, Wnd* modal, const std::set<Wnd*>* ignore = nullptr) const;
    //@}

    /** \name Mutators */ ///@{
    /** Add() places \a wnd in front of the list. */
    void Add(const std::shared_ptr<Wnd>& wnd);

    bool Remove(const std::shared_ptr<Wnd>& wnd);   ///< Removes \a wnd from z-order.

    /** Moves \a wnd from its current position to the beginning of list;
        updates wnd's z-value. */
    bool MoveUp(const std::shared_ptr<Wnd>& wnd);

    /** Moves \a wnd from its current position to the end of list; updates
        wnd's z-value. */
    bool MoveDown(const std::shared_ptr<Wnd>& wnd);
    //@}

private:
    using iterator = std::list<std::shared_ptr<Wnd>>::iterator;

    iterator FirstNonOnTop();              ///< Returns iterator to first window in list that is non-on-top (returns end() if none found).

    /** Return the first candidate that satisfies \p predicate with a non none result along with
        the result. Remove any non-existent Wnd examined. */
    template <typename T>
    boost::optional<std::pair<iterator, T>> Find(
        const std::function<boost::optional<T> (const std::shared_ptr<Wnd>&)>& predicate) const;

    mutable std::list<std::shared_ptr<Wnd>> m_list;
};

} // namespace GG

#endif
