//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/ZList.h
//!
//! Contains the ZList class, which maintains the Z-/depth-position of Wnds for
//! GUI.

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
            m_list(list.rbegin(), list.rend())
        {}

        std::vector<std::shared_ptr<Wnd>>::const_iterator begin() const
        { return m_list.begin(); }

        std::vector<std::shared_ptr<Wnd>>::const_iterator end() const
        { return m_list.end(); }

        private:
        const std::vector<std::shared_ptr<Wnd>> m_list;
    };

    /** Return a RenderOrderIterable in back to front render order. */
    RenderOrderIterable RenderOrder() const;

    /** Returns pointer to the window under the point pt; constrains pick to
        modal if nonzero, and ignores \a ignore if nonzero. */
    std::shared_ptr<Wnd> Pick(const Pt& pt, const std::shared_ptr<Wnd>& modal, const std::set<Wnd*>* ignore = nullptr) const;

    /** Add() places \a wnd in front of the list. */
    void Add(std::shared_ptr<Wnd> wnd);

    /** Remove \p wnd from the z-ordered list. */
    bool Remove(const std::shared_ptr<Wnd>& wnd);
    bool Remove(const Wnd* const wnd);

    /** Moves \a wnd from its current position to the beginning of list;
        updates wnd's z-value. */
    bool MoveUp(const std::shared_ptr<Wnd>& wnd);
    bool MoveUp(const Wnd* const wnd);

    /** Moves \a wnd from its current position to the end of list; updates
        wnd's z-value. */
    bool MoveDown(const std::shared_ptr<Wnd>& wnd);
    bool MoveDown(const Wnd* const wnd);

private:
    using iterator = std::list<std::shared_ptr<Wnd>>::iterator;

    iterator FirstNonOnTop();              ///< Returns iterator to first window in list that is non-on-top (returns end() if none found).

    template <typename T>
    using FunctionOfWndReturningMaybeT = std::function<boost::optional<T> (const std::shared_ptr<Wnd>&)>;

    /** Return the pair (iterator, T) for the first satisying wnd where \p predicate(wnd) !=
        boost::none.  Remove any nullptr found before the first satisying wnd from the list. */
    template <typename T>
    boost::optional<std::pair<iterator, T>> Find(const FunctionOfWndReturningMaybeT<T>& predicate) const;

    mutable std::list<std::shared_ptr<Wnd>> m_list;
};

}

#endif
