//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Wnd.h>
#include <GG/ZList.h>


using namespace GG;

namespace {
    std::shared_ptr<Wnd> PickWithinWindow(Pt pt, std::shared_ptr<Wnd> wnd,
                                          const std::set<Wnd*>* ignore) // TODO: ignore as reference
    {
        if (!wnd)
            return nullptr;

        // look through all the children of wnd, and determine whether pt lies in
        // any of them (or their children)
        const auto& children = wnd->Children();
        const auto end_it = children.rend();
        for (auto it = children.rbegin(); it != end_it; ++it) {
            if (!(*it)->Visible())
                continue;
            if (!(*it)->InWindow(pt))
                continue;
            if (auto temp = PickWithinWindow(pt, *it, ignore))
                return temp;
        }

        // if wnd is visible and clickable, return it if no child windows also catch pt
        if (!wnd->Visible() || !wnd->Interactive() || (ignore && ignore->count(wnd.get())))
            return nullptr;

        return wnd;
    }

    std::shared_ptr<Wnd> ContainsPt(Pt pt, std::shared_ptr<Wnd> locked, const std::set<Wnd*>* ignore)
    {
        if (!locked || !locked->Visible() || !locked->InWindow(pt))
            return nullptr;

        if (auto temp = PickWithinWindow(pt, std::move(locked), ignore))
            return temp;
        return nullptr;
    };

    /** Return the pair (iterator, bool) for the first wnd where \p predicate(wnd) != boost::none.
    * Remove any nullptr found before the first satisying wnd from the list. */
    template <typename L, typename P>
    auto Find(L& list, P predicate)
    {
        using iterator = ZList::iterator;
        using list_value_t = ZList::container_t::value_type;
        using predicate_result_t = decltype(predicate(std::declval<list_value_t>()));
        using retval_t = boost::optional<std::pair<iterator, predicate_result_t>>;

        auto wnd_it = list.begin();
        while (wnd_it != list.end()) {
            if (!*wnd_it) {
                // remove this Wnd and move to next...
                wnd_it = list.erase(wnd_it);
                continue;
            }

            // does this Wnd pass predicate?
            if (auto result = predicate(*wnd_it))
                return retval_t(std::pair{wnd_it, std::move(result)});

            ++wnd_it;
        }

        return retval_t(boost::none);
    }

}

///////////////////////////////////////
// class GG::ZList
///////////////////////////////////////
std::shared_ptr<Wnd> ZList::Pick(Pt pt, std::shared_ptr<Wnd> modal, const std::set<Wnd*>* ignore) const
{
    if (modal) { // if a modal window is active, only look there
        // NOTE: We have to check Visible() separately, because in the
        // rendering code an invisble parent's children are never rendered.
        if (modal->Visible() && modal->InWindow(pt))
            return PickWithinWindow(pt, std::move(modal), ignore);
        return nullptr;

    } else {
        auto found = Find(m_list, [pt, ignore](auto locked) { return ContainsPt(pt, std::move(locked), ignore); });
        if (found)
            return found->second;
    }
    return nullptr;
}

void ZList::Add(std::shared_ptr<Wnd> wnd)
{
    if (!wnd)
        return;

    auto is_wnd = [&wnd](const auto& test_wnd) { return wnd == test_wnd; };
    if (std::any_of(m_list.begin(), m_list.end(), is_wnd))
        return;

    auto wnd_raw = wnd.get();
    m_list.push_back(std::move(wnd));   // add wnd to the end of the list...
    MoveUp(wnd_raw);                    // move it up to the top
}

bool ZList::Remove(const std::shared_ptr<Wnd>& wnd)
{ return Remove(wnd.get()); }

bool ZList::Remove(const Wnd* const wnd)
{
    if (!wnd)
        return false;

    auto is_wnd = [wnd](const auto& test_wnd) { return wnd == test_wnd.get(); };
    auto it = std::find_if(m_list.begin(), m_list.end(), is_wnd);

    if (it == m_list.end())
        return false;

    m_list.erase(it);
    return true;
}

bool ZList::MoveUp(const std::shared_ptr<Wnd>& wnd)
{ return MoveUp(wnd.get()) ;}

bool ZList::MoveUp(Wnd const* const wnd)
{
   if (!wnd)
        return false;

    auto is_wnd = [&wnd](const auto& test_wnd) { return wnd == test_wnd.get(); };
    auto it = std::find_if(m_list.begin(), m_list.end(), is_wnd);

    if (it == m_list.end())
        return false;

    const auto& front = m_list.front();
    if (!front || !front->OnTop() || wnd->OnTop()) {
        // if there are no on-top windows, or wnd is an on-top window just
        // slap wnd on top of the topmost element
        m_list.splice(m_list.begin(), m_list, it);
    } else {
        // front()->OnTop() && !wnd->OnTop(), so only move wnd down to just
        // below the bottom of the on-top range
        m_list.splice(FirstNonOnTop(), m_list, it);
    }
    return true;
}

bool ZList::MoveDown(const std::shared_ptr<Wnd>& wnd)
{ return MoveDown(wnd.get()) ;}

bool ZList::MoveDown(const Wnd* const wnd)
{
    if (!wnd)
        return false;

    auto is_wnd = [wnd](const auto& test_wnd) { return wnd == test_wnd.get(); };
    auto it = std::find_if(m_list.begin(), m_list.end(), is_wnd);

    if (it == m_list.end())
        return false;

    const auto& back = m_list.back();
    if ((back && back->OnTop()) || !wnd->OnTop()) {
        // if there are only on-top windows, or wnd is not an on-top
        // window just put wnd below the bottom element
        m_list.splice(m_list.end(), m_list, it);
    } else {
        // !back()->OnTop() && wnd->OnTop(), so only move wnd up to just
        // below the bottom of the on-top range
        m_list.splice(FirstNonOnTop(), m_list, it);
    }
    return true;
}

ZList::iterator ZList::FirstNonOnTop()
{
    const auto not_on_top = [](const std::shared_ptr<Wnd>& wnd) { return wnd && !wnd->OnTop(); };
    return std::find_if(m_list.begin(), m_list.end(), not_on_top);
}
