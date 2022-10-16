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
    /** Returns \a wnd or one of its (nested) children if they are visible,
      * \a pt is within the returned Wnd and it is not in \a ignore. If no
      * such Wnd is found, returns nullptr. */
    template <typename Wnds>
    std::shared_ptr<Wnd> PickWithinWindow(Pt pt, const std::shared_ptr<Wnd>& wnd, const Wnds& ignore)
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
        if (!wnd->Visible() || !wnd->Interactive())
            return nullptr;
        if (!ignore.empty() && std::find(ignore.begin(), ignore.end(), wnd.get()) != ignore.end())
            return nullptr;
        return wnd;
    }
}

///////////////////////////////////////
// class GG::ZList
///////////////////////////////////////
std::shared_ptr<Wnd> ZList::Pick(Pt pt, std::shared_ptr<Wnd> modal) const
{
    static const std::vector<const Wnd*> NO_WNDS{};
    return Pick(pt, std::move(modal), NO_WNDS);
}

std::shared_ptr<Wnd> ZList::Pick(Pt pt, std::shared_ptr<Wnd> modal,
                                 const std::vector<const Wnd*>& ignore) const
{
    if (modal) { // if a modal window is active, only look there
        // NOTE: We have to check Visible() separately, because in the
        // rendering code an invisble parent's children are never rendered.
        if (modal->Visible() && modal->InWindow(pt))
            return PickWithinWindow(pt, std::move(modal), ignore);

    } else {
        for (const auto& list_wnd : m_list) {
            if (list_wnd && list_wnd->Visible() && list_wnd->InWindow(pt))
                return PickWithinWindow(pt, list_wnd, ignore);
        }
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
