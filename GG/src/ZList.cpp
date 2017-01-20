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

#include <GG/ZList.h>
#include <GG/Wnd.h>

using namespace GG;

///////////////////////////////////////
// class GG::ZList
///////////////////////////////////////
Wnd* ZList::Pick(const Pt& pt, Wnd* modal, const std::set<Wnd*>* ignore/* = 0*/) const
{
    Wnd* retval = nullptr;
    if (modal) { // if a modal window is active, only look there
        // NOTE: We have to check Visible() separately, because in the
        // rendering code an invisble parent's children are never rendered.
        retval =
            modal->Visible() &&
            modal->InWindow(pt) ?
            PickWithinWindow(pt, modal, ignore) : nullptr;
    } else { // otherwise, look in the z-list
        for (Wnd* wnd : *this) {
            Wnd* temp = nullptr;
            if (wnd->Visible() &&
                wnd->InWindow(pt) &&
                (temp = PickWithinWindow(pt, wnd, ignore))) {
                retval = temp;
                break;
            }
        }
    }
    return retval;
}

void ZList::Add(Wnd* wnd)
{
    if (end() == std::find(begin(), end(), wnd)) {
        // add wnd to the end of the list...
        insert(end(), wnd);
        // then move it up to its proper place
        MoveUp(wnd);
    }
}

bool ZList::Remove(Wnd* wnd)
{
    bool retval = false;
    iterator it = std::find(begin(), end(), wnd);
    if (it != end()) {
        erase(it);
        retval = true;
    }
    return retval;
}

bool ZList::MoveUp(Wnd* wnd)
{
    bool retval = false;
    iterator it = std::find(begin(), end(), wnd);
    if (it != end()) { // note that this also implies !empty()..
        if (!front()->OnTop() || wnd->OnTop()) {
            // if there are no on-top windows, or wnd is an on-top window just
            // slap wnd on top of the topmost element
            splice(begin(), *this, it);
        } else {
            // front()->OnTop() && !wnd->OnTop(), so only move wnd up to just
            // below the bottom of the on-top range
            splice(FirstNonOnTop(), *this, it);
        }
        retval = true;
    }
    return retval;
}

bool ZList::MoveDown(Wnd* wnd)
{
    bool retval = false;
    iterator it = std::find(begin(), end(), wnd);
    if (it != end()) {
        if (back()->OnTop() || !wnd->OnTop()) {
            // if there are only on-top windows, or wnd is not an on-top
            // window just put wnd below the bottom element
            splice(end(), *this, it);
        } else {
            // !back()->OnTop() && wnd->OnTop(), so only move wnd up to just
            // below the bottom of the on-top range
            splice(FirstNonOnTop(), *this, it);
        }
        retval = true;
    }
    return retval;
}

Wnd* ZList::PickWithinWindow(const Pt& pt, Wnd* wnd, const std::set<Wnd*>* ignore) const
{
    // if wnd is visible and clickable, return it if no child windows also catch pt
    Wnd* retval =
        (wnd->Visible() &&
         wnd->Interactive() &&
         (!ignore || ignore->find(wnd) == ignore->end())) ?
        wnd : nullptr;
    // look through all the children of wnd, and determine whether pt lies in
    // any of them (or their children)
    std::list<Wnd*>::reverse_iterator end_it = wnd->m_children.rend();
    for (std::list<Wnd*>::reverse_iterator it = wnd->m_children.rbegin(); it != end_it; ++it) {
        if (!(*it)->Visible())
            continue;
        Wnd* temp = nullptr;
        if ((*it)->InWindow(pt) && (temp = PickWithinWindow(pt, *it, ignore))) {
            retval = temp;
            break;
        }
    }
    return retval;
}

ZList::iterator ZList::FirstNonOnTop()
{
    iterator retval = begin();
    for (; retval != end(); ++retval)
        if (!(*retval)->OnTop())
            break;
    return retval;
}
