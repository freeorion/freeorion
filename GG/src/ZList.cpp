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
    std::shared_ptr<Wnd> PickWithinWindow(const Pt& pt, std::shared_ptr<Wnd> wnd,
                                          const std::set<Wnd*>* ignore)
    {
        // look through all the children of wnd, and determine whether pt lies in
        // any of them (or their children)
        const auto& end_it = wnd->Children().rend();
        for (auto it = wnd->Children().rbegin(); it != end_it; ++it) {
            if (!(*it)->Visible())
                continue;
            if (!(*it)->InWindow(pt))
                continue;
            if (auto temp = PickWithinWindow(pt, std::move(*it), ignore))
                return temp;
        }

        // if wnd is visible and clickable, return it if no child windows also catch pt
        if (!wnd->Visible() || !wnd->Interactive()  || (ignore && ignore->count(wnd.get())))
            return nullptr;

        return wnd;
    }
}

///////////////////////////////////////
// class GG::ZList
///////////////////////////////////////


ZList::~ZList()
{}

ZList::RenderOrderIterable ZList::RenderOrder() const
{ return RenderOrderIterable(m_list); }

std::shared_ptr<Wnd> ZList::Pick(const Pt& pt, const std::shared_ptr<Wnd>& modal, const std::set<Wnd*>* ignore/* = 0*/) const
{
    if (modal) { // if a modal window is active, only look there
        // NOTE: We have to check Visible() separately, because in the
        // rendering code an invisble parent's children are never rendered.
        return (modal->Visible() && modal->InWindow(pt)) ? PickWithinWindow(pt, modal, ignore) : nullptr;
    } else {
        // otherwise, look in the z-list for the first visible Wnd containg pt.
        std::function<boost::optional<std::shared_ptr<Wnd>> (const std::shared_ptr<Wnd>&)> contains_pt =
            [&pt, &ignore](const std::shared_ptr<Wnd>& locked) {
            if (!locked->Visible() || !locked->InWindow(pt))
                return boost::optional<std::shared_ptr<Wnd>>(boost::none);

            if (auto temp = PickWithinWindow(pt, locked, ignore))
                return boost::optional<std::shared_ptr<Wnd>>(temp);
            return boost::optional<std::shared_ptr<Wnd>>(boost::none);
        };

        if (auto found = Find(contains_pt))
            return found->second;
    }
    return nullptr;
}

void ZList::Add(std::shared_ptr<Wnd> wnd)
{
    if (!wnd)
        return;

    std::function<boost::optional<bool> (const std::shared_ptr<Wnd>&)> equals_wnd =
        [&wnd](const std::shared_ptr<Wnd>& locked) {
        return wnd == locked ? boost::optional<bool>(true) : boost::optional<bool>(boost::none); };
    auto found = Find(equals_wnd);

    if (!found) {
        auto wndp = wnd.get();
        // add wnd to the end of the list...
        m_list.emplace_back(std::move(wnd));
        // then move it up to its proper place
        MoveUp(wndp);
    }
}

bool ZList::Remove(const std::shared_ptr<Wnd>& wnd)
{ return Remove(wnd.get()); }

bool ZList::Remove(const Wnd* const wnd)
{
    if (!wnd)
        return false;

    std::function<boost::optional<bool> (const std::shared_ptr<Wnd>&)> equals_wnd =
        [&wnd](const std::shared_ptr<Wnd>& locked) {
        return wnd == locked.get() ? boost::optional<bool>(true) : boost::optional<bool>(boost::none); };
    auto found = Find(equals_wnd);

    if (found)
        m_list.erase(found->first);
    return bool(found);
}

bool ZList::MoveUp(const std::shared_ptr<Wnd>& wnd)
{ return MoveUp(wnd.get()) ;}

bool ZList::MoveUp(Wnd const * const wnd)
{
   if (!wnd)
        return false;

    std::function<boost::optional<bool> (const std::shared_ptr<Wnd>&)> equals_wnd =
        [&wnd](const std::shared_ptr<Wnd>& locked) {
        return wnd == locked.get() ? boost::optional<bool>(true) : boost::optional<bool>(boost::none); };
    auto found = Find(equals_wnd);

    if (found) { // note that this also implies !empty()..

        auto front = m_list.front();
        if (!front || !front->OnTop() || (wnd && wnd->OnTop())) {
            // if there are no on-top windows, or wnd is an on-top window just
            // slap wnd on top of the topmost element
            m_list.splice(m_list.begin(), m_list, found->first);
        } else {
            // front()->OnTop() && !wnd->OnTop(), so only move wnd up to just
            // below the bottom of the on-top range
            m_list.splice(FirstNonOnTop(), m_list, found->first);
        }
    }
    return bool(found);
}

bool ZList::MoveDown(const std::shared_ptr<Wnd>& wnd)
{ return MoveDown(wnd.get()) ;}

bool ZList::MoveDown(const Wnd* const wnd)
{
    if (!wnd)
        return false;

    std::function<boost::optional<bool> (const std::shared_ptr<Wnd>&)> equals_wnd =
        [&wnd](const std::shared_ptr<Wnd>& locked) {
        return wnd == locked.get() ? boost::optional<bool>(true) : boost::optional<bool>(boost::none); };
    auto found = Find(equals_wnd);

    if (found) {
        auto back = m_list.back();
        if ((back && back->OnTop()) || !wnd || !wnd->OnTop()) {
            // if there are only on-top windows, or wnd is not an on-top
            // window just put wnd below the bottom element
            m_list.splice(m_list.end(), m_list, found->first);
        } else {
            // !back()->OnTop() && wnd->OnTop(), so only move wnd up to just
            // below the bottom of the on-top range
            m_list.splice(FirstNonOnTop(), m_list, found->first);
        }
    }
    return bool(found);
}

namespace {
    const std::function<boost::optional<bool> (const std::shared_ptr<Wnd>&)> IsNotOnTop =
        [](const std::shared_ptr<Wnd>& wnd)
    {
        return !wnd->OnTop() ? boost::optional<bool>(true) : boost::optional<bool>(boost::none);
    };
}

ZList::iterator ZList::FirstNonOnTop()
{
    auto retval = Find(IsNotOnTop);
    return retval ? retval->first : m_list.end();
}

template <typename T> boost::optional<std::pair<ZList::iterator, T>> ZList::Find(
    const std::function<boost::optional<T> (const std::shared_ptr<Wnd>&)>& predicate) const
{
    auto retval = m_list.begin();
    while (retval != m_list.end()) {
        if (! *retval) {
            retval = m_list.erase(retval);
            continue;
        }

        if (auto result = predicate(*retval))
            return std::make_pair(retval, *result);

        ++retval;
    }
    return boost::none;
}
