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


#include <functional>
#include <list>
#include <memory>
#include <set>
#include <vector>
#include <boost/optional/optional.hpp>
#include <GG/Base.h>


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
    using container_t = std::list<std::shared_ptr<Wnd>>;
    using iterator = container_t::iterator;

    /** Return a RenderOrderIterable in back to front render order. */
    std::vector<std::shared_ptr<Wnd>> RenderOrder() const { return {m_list.crbegin(), m_list.crend()}; }

    /** Returns pointer to the window under the point pt; constrains pick to
        modal if nonzero, and ignores \a ignore if nonzero. */
    std::shared_ptr<Wnd> Pick(Pt pt, std::shared_ptr<Wnd> modal) const;
    std::shared_ptr<Wnd> Pick(Pt pt, std::shared_ptr<Wnd> modal,
                              const std::vector<const Wnd*>& ignore) const;

    /** Add() places \a wnd in front of the list. */
    void Add(std::shared_ptr<Wnd> wnd);

    /** Remove \p wnd from the z-ordered list. */
    bool Remove(const std::shared_ptr<Wnd>& wnd);
    bool Remove(const Wnd* const wnd);

    /** Moves \a wnd from its current position to the beginning of list, or
      * the clostes to the beginning if there are other OnTop Wnds.
      * Updates wnd's z-value. */
    bool MoveUp(const std::shared_ptr<Wnd>& wnd);
    bool MoveUp(const Wnd* const wnd);

    /** Moves \a wnd from its current position to the end of list.
    * * Updates wnd's z-value. */
    bool MoveDown(const std::shared_ptr<Wnd>& wnd);
    bool MoveDown(const Wnd* const wnd);

private:
    iterator FirstNonOnTop(); ///< iterator to first window in list that is non-on-top (returns end() if none found).

    container_t m_list;
};

}


#endif
