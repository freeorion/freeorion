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

#include <GG/Wnd.h>

#include <GG/GUI.h>
#include <GG/BrowseInfoWnd.h>
#include <GG/DrawUtil.h>
#include <GG/EventPump.h>
#include <GG/Layout.h>
#include <GG/WndEvent.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

using namespace GG;

namespace {
    using namespace boost::multi_index;
    struct GridLayoutWnd
    {
        GridLayoutWnd() : wnd(0) {}
        GridLayoutWnd(Wnd* wnd_, const Pt& ul_, const Pt& lr_) : wnd(wnd_), ul(ul_), lr(lr_) {}
        Wnd* wnd;
        Pt ul;
        Pt lr;
    };
    struct IsLeft
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return lhs.x < rhs.x;}
        bool operator()(X x, const Pt& pt) const       {return x < pt.x;}
        bool operator()(const Pt& pt, X x) const       {return pt.x < x;}
    };
    struct IsTop
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return lhs.y < rhs.y;}
        bool operator()(Y y, const Pt& pt) const       {return y < pt.y;}
        bool operator()(const Pt& pt, Y y) const       {return pt.y < y;}
    };
    struct IsRight
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return rhs.x < lhs.x;}
        bool operator()(X x, const Pt& pt) const       {return pt.x < x;}
        bool operator()(const Pt& pt, X x) const       {return x < pt.x;}
    };
    struct IsBottom
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return rhs.y < lhs.y;}
        bool operator()(Y y, const Pt& pt) const       {return pt.y < y;}
        bool operator()(const Pt& pt, Y y) const       {return y < pt.y;}
    };
    struct Pointer {};
    struct LayoutLeft {};
    struct LayoutTop {};
    struct LayoutRight {};
    struct LayoutBottom {};
    typedef multi_index_container<
        GridLayoutWnd,
        indexed_by<
            ordered_unique<tag<Pointer>,            member<GridLayoutWnd, Wnd*, &GridLayoutWnd::wnd> >,
            ordered_non_unique<tag<LayoutLeft>,     member<GridLayoutWnd, Pt,   &GridLayoutWnd::ul>, IsLeft>,
            ordered_non_unique<tag<LayoutTop>,      member<GridLayoutWnd, Pt,   &GridLayoutWnd::ul>, IsTop>,
            ordered_non_unique<tag<LayoutRight>,    member<GridLayoutWnd, Pt,   &GridLayoutWnd::lr>, IsRight>,
            ordered_non_unique<tag<LayoutBottom>,   member<GridLayoutWnd, Pt,   &GridLayoutWnd::lr>, IsBottom>
        >
    > GridLayoutWndContainer;
    typedef GridLayoutWndContainer::index<Pointer>::type::iterator      PointerIter;
    typedef GridLayoutWndContainer::index<LayoutLeft>::type::iterator   LeftIter;
    typedef GridLayoutWndContainer::index<LayoutTop>::type::iterator    TopIter;
    typedef GridLayoutWndContainer::index<LayoutRight>::type::iterator  RightIter;
    typedef GridLayoutWndContainer::index<LayoutBottom>::type::iterator BottomIter;

    struct WndHorizontalLess
    {
        bool operator()(const Wnd* lhs, const Wnd* rhs) const
            {return lhs->Left() < rhs->Left();}
    };

    struct WndVerticalLess
    {
        bool operator()(const Wnd* lhs, const Wnd* rhs) const
            {return lhs->Top() < rhs->Top();}
    };

    const int DEFAULT_LAYOUT_BORDER_MARGIN = 0;
    const int DEFAULT_LAYOUT_CELL_MARGIN = 5;

    struct WndSizeFunctor
    {
        std::string operator()(const Wnd* wnd)
        {
            if (!wnd)
                return "";
            std::stringstream stream;
            stream << "(" << wnd->Width() << ", " << wnd->Height() << ")";
            return stream.str();
        }
    };

    struct WndClientSizeFunctor
    {
        std::string operator()(const Wnd* wnd)
        {
            if (!wnd)
                return "";
            std::stringstream stream;
            stream << "(" << wnd->ClientWidth() << ", " << wnd->ClientHeight() << ")";
            return stream.str();
        }
    };

    struct ForwardToParentException {};
}

///////////////////////////////////////
// WndFlags
///////////////////////////////////////
const WndFlag GG::NO_WND_FLAGS       (0);
const WndFlag GG::INTERACTIVE        (1 << 0);
const WndFlag GG::REPEAT_BUTTON_DOWN (1 << 1);
const WndFlag GG::DRAGABLE           (1 << 2);
const WndFlag GG::RESIZABLE          (1 << 3);
const WndFlag GG::ONTOP              (1 << 4);
const WndFlag GG::MODAL              (1 << 5);
const WndFlag GG::REPEAT_KEY_PRESS   (1 << 6);

GG_FLAGSPEC_IMPL(WndFlag);

namespace {
    bool RegisterWndFlags()
    {
        FlagSpec<WndFlag>& spec = FlagSpec<WndFlag>::instance();
        spec.insert(NO_WND_FLAGS,       "NO_WND_FLAGS",         true);
        spec.insert(INTERACTIVE,        "INTERACTIVE",          true);
        spec.insert(REPEAT_BUTTON_DOWN, "REPEAT_BUTTON_DOWN",   true);
        spec.insert(DRAGABLE,           "DRAGABLE",             true);
        spec.insert(RESIZABLE,          "RESIZABLE",            true);
        spec.insert(ONTOP,              "ONTOP",                true);
        spec.insert(MODAL,              "MODAL",                true);
        spec.insert(REPEAT_KEY_PRESS,   "REPEAT_KEY_PRESS",     true);
        return true;
    }
    bool dummy = RegisterWndFlags();
}


///////////////////////////////////////
// class GG::Wnd
///////////////////////////////////////
// static(s)
unsigned int Wnd::s_default_browse_time = 1500;
boost::shared_ptr<BrowseInfoWnd> Wnd::s_default_browse_info_wnd;

Wnd::Wnd() :
    m_done(false),
    m_parent(0),
    m_zorder(0),
    m_visible(true),
    m_child_clipping_mode(DontClip),
    m_non_client_child(false),
    m_max_size(X(1 << 30), Y(1 << 30)),
    m_layout(0),
    m_containing_layout(0),
    m_flags()
{
    m_browse_modes.resize(1);
    m_browse_modes[0].time = s_default_browse_time;
    m_browse_modes[0].wnd = s_default_browse_info_wnd;
}

Wnd::Wnd(X x, Y y, X w, Y h, Flags<WndFlag> flags/* = INTERACTIVE | DRAGABLE*/) :
    m_done(false),
    m_parent(0),
    m_zorder(0),
    m_visible(true),
    m_child_clipping_mode(DontClip),
    m_non_client_child(false),
    m_upperleft(x, y),
    m_lowerright(x + w, y + h),
    m_max_size(X(1 << 30), Y(1 << 30)),
    m_layout(0),
    m_containing_layout(0),
    m_flags(flags)
{
    ValidateFlags();
    m_browse_modes.resize(1);
    m_browse_modes[0].time = s_default_browse_time;
    m_browse_modes[0].wnd = s_default_browse_info_wnd;
}

Wnd::~Wnd()
{
    // remove this-references from Wnds that this Wnd filters
    for (std::set<Wnd*>::iterator it1 = m_filtering.begin(); it1 != m_filtering.end(); ++it1) {
        std::vector<Wnd*>::iterator it2 = std::find((*it1)->m_filters.begin(), (*it1)->m_filters.end(), this);
        if (it2 != (*it1)->m_filters.end())
            (*it1)->m_filters.erase(it2);
    }

    // remove this-references from Wnds that filter this Wnd
    for (std::vector<Wnd*>::iterator it1 = m_filters.begin(); it1 != m_filters.end(); ++it1) {
        (*it1)->m_filtering.erase(this);
    }

    if (Wnd* parent = Parent())
        parent->DetachChild(this);

    GUI::GetGUI()->WndDying(this);

    DeleteChildren();
}

bool Wnd::Interactive() const
{ return m_flags & INTERACTIVE; }

bool Wnd::RepeatKeyPress() const
{ return m_flags & REPEAT_KEY_PRESS; }

bool Wnd::RepeatButtonDown() const
{ return m_flags & REPEAT_BUTTON_DOWN; }

bool Wnd::Dragable() const
{ return m_flags & DRAGABLE; }

bool Wnd::Resizable() const
{ return m_flags & RESIZABLE; }

bool Wnd::OnTop() const
{ return !m_parent && m_flags & ONTOP; }

bool Wnd::Modal() const
{ return !m_parent && m_flags & MODAL; }

Wnd::ChildClippingMode Wnd::GetChildClippingMode() const
{ return m_child_clipping_mode; }

bool Wnd::NonClientChild() const
{ return m_non_client_child; }

bool Wnd::Visible() const
{ return m_visible; }

const std::string& Wnd::Name() const
{ return m_name; }

const std::string& Wnd::DragDropDataType() const
{ return m_drag_drop_data_type; }

void Wnd::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                          const Pt& pt, Flags<ModKey> mod_keys) const
{
    // default reject all drops. derived classes can override to accept drops
    for (std::map<const Wnd*, bool>::iterator it = first; it != last; ++it)
        it->second = false;
}

Pt Wnd::UpperLeft() const
{
    Pt retval = m_upperleft;
    if (m_parent)
        retval += m_parent->ClientUpperLeft();
    return retval;
}

X Wnd::Left() const
{ return UpperLeft().x; }

Y Wnd::Top() const
{ return UpperLeft().y; }

Pt Wnd::LowerRight() const
{
    Pt retval = m_lowerright;
    if (m_parent)
        retval += m_parent->ClientUpperLeft();
    return retval;
}

X Wnd::Right() const
{ return LowerRight().x; }

Y Wnd::Bottom() const
{ return LowerRight().y; }

Pt Wnd::RelativeUpperLeft() const
{ return m_upperleft; }

Pt Wnd::RelativeLowerRight() const
{ return m_lowerright; }

X Wnd::Width() const
{ return m_lowerright.x - m_upperleft.x; }

Y Wnd::Height() const
{ return m_lowerright.y - m_upperleft.y; }

int Wnd::ZOrder() const
{ return m_zorder; }

Pt Wnd::Size() const
{ return Pt(m_lowerright.x - m_upperleft.x, m_lowerright.y - m_upperleft.y); }

Pt Wnd::MinSize() const
{ return m_min_size; }

Pt Wnd::MaxSize() const
{ return m_max_size; }

Pt Wnd::MinUsableSize() const
{ return Size(); }

Pt Wnd::ClientUpperLeft() const
{ return UpperLeft(); }

Pt Wnd::ClientLowerRight() const
{ return LowerRight(); }

Pt Wnd::ClientSize() const
{ return ClientLowerRight() - ClientUpperLeft(); }

X Wnd::ClientWidth() const
{ return ClientLowerRight().x - ClientUpperLeft().x; }

Y Wnd::ClientHeight() const
{ return ClientLowerRight().y - ClientUpperLeft().y; }

Pt Wnd::ScreenToWindow(const Pt& pt) const
{ return pt - UpperLeft(); }

Pt Wnd::ScreenToClient(const Pt& pt) const
{ return pt - ClientUpperLeft(); }

bool Wnd::InWindow(const Pt& pt) const
{ return pt >= UpperLeft() && pt < LowerRight(); }

bool Wnd::InClient(const Pt& pt) const
{ return pt >= ClientUpperLeft() && pt < ClientLowerRight(); }

const std::list<Wnd*>& Wnd::Children() const
{ return m_children; }

Wnd* Wnd::Parent() const
{ return m_parent; }

Wnd* Wnd::RootParent() const
{
    Wnd* retval = m_parent;
    while (retval && retval->Parent()) {
        retval = retval->Parent();
    }
    return retval;
}

Layout* Wnd::GetLayout() const
{ return m_layout; }

Layout* Wnd::ContainingLayout() const
{ return m_containing_layout; }

const std::vector<Wnd::BrowseInfoMode>& Wnd::BrowseModes() const
{ return m_browse_modes; }

const std::string& Wnd::BrowseInfoText(std::size_t mode) const
{ return m_browse_modes.at(mode).text; }

const boost::shared_ptr<StyleFactory>& Wnd::GetStyleFactory() const
{ return m_style_factory ? m_style_factory : GUI::GetGUI()->GetStyleFactory(); }

WndRegion Wnd::WindowRegion(const Pt& pt) const
{
    enum {LEFT = 0, MIDDLE = 1, RIGHT = 2};
    enum {TOP = 0, BOTTOM = 2};

    // window regions look like this:
    // 0111112
    // 3444445   // 4 is client area, 0,2,6,8 are corners
    // 3444445
    // 6777778

    int x_pos = MIDDLE;   // default & typical case is that the mouse is over the (non-border) client area
    int y_pos = MIDDLE;

    if (pt.x < ClientUpperLeft().x)
        x_pos = LEFT;
    else if (pt.x > ClientLowerRight().x)
        x_pos = RIGHT;

    if (pt.y < ClientUpperLeft().y)
        y_pos = TOP;
    else if (pt.y > ClientLowerRight().y)
        y_pos = BOTTOM;

    return (Resizable() ? WndRegion(x_pos + 3 * y_pos) : WR_NONE);
}

void Wnd::SetDragDropDataType(const std::string& data_type)
{ m_drag_drop_data_type = data_type; }

void Wnd::StartingChildDragDrop(const Wnd* wnd, const Pt& offset)
{}

void Wnd::AcceptDrops(const Pt& pt, const std::vector<Wnd*>& wnds, Flags<ModKey> mod_keys)
{
    if (!Interactive() && Parent())
        ForwardEventToParent();
    // if dropped Wnds were accepted, but no handler or parent exists to handle them, delete them
    for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it)
        delete *it;
}

void Wnd::CancellingChildDragDrop(const std::vector<const Wnd*>& wnds)
{}

void Wnd::ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination)
{
    for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
        DetachChild(*it);
    }
}

void Wnd::SetName(const std::string& name)
{ m_name = name; }

void Wnd::Hide(bool children/* = true*/)
{
    m_visible = false;
    if (children) {
        std::list<Wnd*>::iterator it = m_children.begin();
        for (; it != m_children.end(); ++it)
            (*it)->Hide(children);
    }
}

void Wnd::Show(bool children/* = true*/)
{
    m_visible = true;
    if (children) {
        std::list<Wnd*>::iterator it = m_children.begin();
        for (; it != m_children.end(); ++it)
            (*it)->Show(children);
    }
}

void Wnd::ModalInit()
{}

void Wnd::SetChildClippingMode(ChildClippingMode mode)
{ m_child_clipping_mode = mode; }

void Wnd::NonClientChild(bool b)
{ m_non_client_child = b; }

void Wnd::MoveTo(const Pt& pt)
{ SizeMove(pt, pt + Size()); }

void Wnd::OffsetMove(const Pt& pt)
{ SizeMove(m_upperleft + pt, m_lowerright + pt); }

void Wnd::SizeMove(const Pt& ul_, const Pt& lr_)
{
    Pt ul = ul_, lr = lr_;
    Pt original_sz = Size();
    bool resized = (original_sz.x != (lr.x - ul.x)) || (original_sz.y != (lr.y - ul.y));
    if (resized) {
        Pt min_sz = MinSize();
        Pt max_sz = MaxSize();
        if (m_layout) {
            Pt layout_min_sz = m_layout->MinSize() + (Size() - ClientSize());
            min_sz.x = std::max(min_sz.x, layout_min_sz.x);
            min_sz.y = std::max(min_sz.y, layout_min_sz.y);
        }
        if (lr.x - ul.x < min_sz.x) {
            if (ul.x != m_upperleft.x)
                ul.x = lr.x - min_sz.x;
            else if (lr.x != m_lowerright.x)
                lr.x = ul.x + min_sz.x;
        } else if (max_sz.x < lr.x - ul.x) {
            if (lr.x != m_lowerright.x)
                lr.x = ul.x + max_sz.x;
            else
                ul.x = lr.x - max_sz.x;
        }
        if (lr.y - ul.y < min_sz.y) {
            if (ul.y != m_upperleft.y)
                ul.y = lr.y - min_sz.y;
            else if (lr.y != m_lowerright.y)
                lr.y = ul.y + min_sz.y;
        } else if (max_sz.y < lr.y - ul.y) {
            if (lr.y != m_lowerright.y)
                lr.y = ul.y + max_sz.y;
            else
                ul.y = lr.y - max_sz.y;
        }
    }
    m_upperleft = ul;
    m_lowerright = lr;
    if (resized) {
        bool size_changed = Size() != original_sz;
        if (m_layout && size_changed)
            m_layout->Resize(ClientSize());
        if (m_containing_layout && size_changed && !dynamic_cast<Layout*>(this))
            m_containing_layout->ChildSizeOrMinSizeOrMaxSizeChanged();
    }
}

void Wnd::Resize(const Pt& sz)
{ SizeMove(m_upperleft, m_upperleft + sz); }

void Wnd::SetMinSize(const Pt& sz)
{
    bool min_size_changed = m_min_size != sz;
    m_min_size = sz;
    if (Width() < m_min_size.x || Height() < m_min_size.y)
        Resize(Pt(std::max(Width(), m_min_size.x), std::max(Height(), m_min_size.y)));
    else if (m_containing_layout && min_size_changed && !dynamic_cast<Layout*>(this))
        m_containing_layout->ChildSizeOrMinSizeOrMaxSizeChanged();
}

void Wnd::SetMaxSize(const Pt& sz)
{
    m_max_size = sz;
    if (m_max_size.x < Width() || m_max_size.y < Height())
        Resize(Pt(std::min(Width(), m_max_size.x), std::min(Height(), m_max_size.y)));
}

void Wnd::AttachChild(Wnd* wnd)
{
    if (wnd) {
        // remove from previous parent, if any
        if (wnd->Parent())
            wnd->Parent()->DetachChild(wnd);
        GUI::GetGUI()->Remove(wnd);
        m_children.push_back(wnd);
        wnd->SetParent(this);
        if (Layout* this_as_layout = dynamic_cast<Layout*>(this))
            wnd->m_containing_layout = this_as_layout;
    }
}

void Wnd::MoveChildUp(Wnd* wnd)
{
    if (wnd) {
        if (std::find(m_children.begin(), m_children.end(), wnd) != m_children.end()) {
            m_children.remove(wnd);
            m_children.push_back(wnd);
        }
    }
}
 
void Wnd::MoveChildDown(Wnd* wnd)
{
    if (wnd) {
        if (std::find(m_children.begin(), m_children.end(), wnd) != m_children.end()) {
            m_children.remove(wnd);
            m_children.push_front(wnd);
        }
    }
}

void Wnd::DetachChild(Wnd* wnd)
{
    if (wnd) {
        std::list<Wnd*>::iterator it = std::find(m_children.begin(), m_children.end(), wnd);
        if (it != m_children.end()) {
            m_children.erase(it);
            wnd->SetParent(0);
            if (Layout* this_as_layout = dynamic_cast<Layout*>(this)) {
                this_as_layout->Remove(wnd);
                wnd->m_containing_layout = 0;
            }
        }
    }
}

void Wnd::DetachChildren()
{
    for (std::list<Wnd*>::iterator it = m_children.begin(); it != m_children.end();) {
        std::list<Wnd*>::iterator temp = it;
        ++it;
        DetachChild(*temp);
    }
}

void Wnd::DeleteChild(Wnd* wnd)
{
    if (wnd && std::find(m_children.begin(), m_children.end(), wnd) != m_children.end()) {
        delete wnd;
    }
}

void Wnd::DeleteChildren()
{
    for (std::list<Wnd*>::iterator it = m_children.begin(); it != m_children.end();) {
        Wnd* wnd = *it++;
        delete wnd;
    }
}

void Wnd::InstallEventFilter(Wnd* wnd)
{
    if (wnd) {
        RemoveEventFilter(wnd);
        m_filters.push_back(wnd);
        wnd->m_filtering.insert(this);
    }
}

void Wnd::RemoveEventFilter(Wnd* wnd)
{
    if (wnd) {
        std::vector<Wnd*>::iterator it = std::find(m_filters.begin(), m_filters.end(), wnd);
        if (it != m_filters.end())
            m_filters.erase(it);
        wnd->m_filtering.erase(this);
    }
}

void Wnd::HorizontalLayout()
{
    RemoveLayout();

    std::multiset<Wnd*, WndHorizontalLess> wnds;
    Pt client_sz = ClientSize();
    for (std::list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        Pt wnd_ul = (*it)->RelativeUpperLeft(), wnd_lr = (*it)->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;
        wnds.insert(*it);
    }

    m_layout = new Layout(X0, Y0, ClientSize().x, ClientSize().y,
                          1, wnds.size(),
                          DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    AttachChild(m_layout);

    int i = 0;
    for (std::multiset<Wnd*, WndHorizontalLess>::iterator it = wnds.begin(); it != wnds.end(); ++it) {
        m_layout->Add(*it, 0, i++);
    }
}

void Wnd::VerticalLayout()
{
    RemoveLayout();

    std::multiset<Wnd*, WndVerticalLess> wnds;
    Pt client_sz = ClientSize();
    for (std::list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        Pt wnd_ul = (*it)->RelativeUpperLeft(), wnd_lr = (*it)->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;
        wnds.insert(*it);
    }

    m_layout = new Layout(X0, Y0, ClientSize().x, ClientSize().y,
                          wnds.size(), 1,
                          DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    AttachChild(m_layout);

    int i = 0;
    for (std::multiset<Wnd*, WndVerticalLess>::iterator it = wnds.begin(); it != wnds.end(); ++it) {
        m_layout->Add(*it, i++, 0);
    }
}

void Wnd::GridLayout()
{
    RemoveLayout();

    Pt client_sz = ClientSize();

    GridLayoutWndContainer grid_layout;

    // validate existing children and place them in a grid with one cell per pixel
    for (std::list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        Wnd* wnd = *it;
        Pt wnd_ul = wnd->RelativeUpperLeft(), wnd_lr = wnd->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;

        std::list<Wnd*>::const_iterator it2 = it;
        ++it2;
        for (; it2 != m_children.end(); ++it2) {
            Rect other_wnd_rect((*it2)->RelativeUpperLeft(), (*it2)->RelativeLowerRight());
            if (other_wnd_rect.Contains(wnd_ul) || other_wnd_rect.Contains(wnd_lr - Pt(X1, Y1)))
                throw BadLayout("Wnd::GridLayout() : Two or more child windows overlap");
        }

        grid_layout.insert(GridLayoutWnd(wnd, wnd_ul, wnd_lr));
    }


    // align left sides of windows
    for (LeftIter it = grid_layout.get<LayoutLeft>().begin();
         it != grid_layout.get<LayoutLeft>().end(); ++it)
    {
        Pt ul = it->ul;
        for (X x = ul.x - 1; x >= 0; --x) {
            if (grid_layout.get<LayoutRight>().find(x + 1, IsRight()) != grid_layout.get<LayoutRight>().end()) {
                break;
            } else if (grid_layout.get<LayoutLeft>().find(x, IsLeft()) != grid_layout.get<LayoutLeft>().end()) {
                GridLayoutWnd grid_wnd = *it;
                grid_wnd.ul.x = x;
                grid_layout.get<LayoutLeft>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // align right sides of windows
    for (RightIter it = grid_layout.get<LayoutRight>().begin(); it != grid_layout.get<LayoutRight>().end(); ++it) {
        Pt lr = it->lr;
        for (X x = lr.x + 1; x < client_sz.x; ++x) {
            if (grid_layout.get<LayoutLeft>().find(x - 1, IsLeft()) != grid_layout.get<LayoutLeft>().end()) {
                break;
            } else if (grid_layout.get<LayoutRight>().find(x, IsRight()) != grid_layout.get<LayoutRight>().end()) {
                GridLayoutWnd grid_wnd = *it;
                grid_wnd.lr.x = x;
                grid_layout.get<LayoutRight>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // align tops of windows
    for (TopIter it = grid_layout.get<LayoutTop>().begin(); it != grid_layout.get<LayoutTop>().end(); ++it) {
        Pt ul = it->ul;
        for (Y y = ul.y - 1; y >= 0; --y) {
            if (grid_layout.get<LayoutBottom>().find(y + 1, IsBottom()) != grid_layout.get<LayoutBottom>().end()) {
                break;
            } else if (grid_layout.get<LayoutTop>().find(y, IsTop()) != grid_layout.get<LayoutTop>().end()) {
                GridLayoutWnd grid_wnd = *it;
                grid_wnd.ul.y = y;
                grid_layout.get<LayoutTop>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // align bottoms of windows
    for (BottomIter it = grid_layout.get<LayoutBottom>().begin(); it != grid_layout.get<LayoutBottom>().end(); ++it) {
        Pt lr = it->lr;
        for (Y y = lr.y + 1; y < client_sz.y; ++y) {
            if (grid_layout.get<LayoutTop>().find(y - 1, IsTop()) != grid_layout.get<LayoutTop>().end()) {
                break;
            } else if (grid_layout.get<LayoutBottom>().find(y, IsBottom()) != grid_layout.get<LayoutBottom>().end()) {
                GridLayoutWnd grid_wnd = *it;
                grid_wnd.lr.y = y;
                grid_layout.get<LayoutBottom>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // create an actual layout with a more reasonable number of cells from the pixel-grid layout
    std::set<X> unique_lefts;
    std::set<Y> unique_tops;
    for (LeftIter it = grid_layout.get<LayoutLeft>().begin(); it != grid_layout.get<LayoutLeft>().end(); ++it) {
        unique_lefts.insert(it->ul.x);
    }
    for (TopIter it = grid_layout.get<LayoutTop>().begin(); it != grid_layout.get<LayoutTop>().end(); ++it) {
        unique_tops.insert(it->ul.y);
    }

    if (unique_lefts.empty() || unique_tops.empty())
        return;

    m_layout = new Layout(X0, Y0, ClientSize().x, ClientSize().y,
                          unique_tops.size(), unique_lefts.size(),
                          DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    AttachChild(m_layout);

    // populate this new layout with the child windows, based on their placements in the pixel-grid layout
    for (PointerIter it = grid_layout.get<Pointer>().begin(); it != grid_layout.get<Pointer>().end(); ++it) {
        Wnd* wnd = it->wnd;
        Pt ul = it->ul;
        Pt lr = it->lr;
        int left = std::distance(unique_lefts.begin(), unique_lefts.find(ul.x));
        int top = std::distance(unique_tops.begin(), unique_tops.find(ul.y));
        int right = std::distance(unique_lefts.begin(), unique_lefts.lower_bound(lr.x));
        int bottom = std::distance(unique_tops.begin(), unique_tops.lower_bound(lr.y));
        m_layout->Add(wnd, top, left, bottom - top, right - left);
    }
}

void Wnd::SetLayout(Layout* layout)
{
    if (layout == m_layout && layout == m_containing_layout)
        throw BadLayout("Wnd::SetLayout() : Attempted to set a Wnd's layout to be its current layout or the layout that contains the Wnd");
    RemoveLayout();
    std::list<Wnd*> children = m_children;
    DetachChildren();
    Pt client_sz = ClientSize();
    for (std::list<Wnd*>::const_iterator it = children.begin(); it != children.end(); ++it) {
        Pt wnd_ul = (*it)->RelativeUpperLeft(), wnd_lr = (*it)->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            AttachChild(*it);
        else
            delete *it;
    }
    AttachChild(layout);
    m_layout = layout;
    m_layout->SizeMove(Pt(), Pt(ClientWidth(), ClientHeight()));
}

void Wnd::RemoveLayout()
{
    if (m_layout) {
        std::list<Wnd*> layout_children = m_layout->Children();
        m_layout->DetachAndResetChildren();
        for (std::list<Wnd*>::iterator it = layout_children.begin(); it != layout_children.end(); ++it) {
            AttachChild(*it);
        }
        DeleteChild(m_layout);
        m_layout = 0;
    }
}

Layout* Wnd::DetachLayout()
{
    Layout* retval = m_layout;
    DetachChild(m_layout);
    m_layout = 0;
    return retval;
}

void Wnd::SetLayoutBorderMargin(unsigned int margin)
{
    if (m_layout)
        m_layout->SetBorderMargin(margin);
}

void Wnd::SetLayoutCellMargin(unsigned int margin)
{
    if (m_layout)
        m_layout->SetCellMargin(margin);
}

void Wnd::Render() {}

bool Wnd::Run()
{
    bool retval = false;
    if (!m_parent && m_flags & MODAL) {
        GUI* gui = GUI::GetGUI();
        gui->RegisterModal(this);
        ModalInit();
        m_done = false;
        boost::shared_ptr<ModalEventPump> pump = gui->CreateModalEventPump(m_done);
        (*pump)();
        gui->Remove(this);
        retval = true;
    }
    return retval;
}

void Wnd::EndRun()
{ m_done = true; }

void Wnd::SetBrowseModeTime(unsigned int time, std::size_t mode/* = 0*/)
{
    if (m_browse_modes.size() <= mode) {
        if (m_browse_modes.empty()) {
            m_browse_modes.resize(mode + 1);
            for (std::size_t i = 0; i < m_browse_modes.size() - 1; ++i) {
                m_browse_modes[i].time = time;
            }
        } else {
            std::size_t original_size = m_browse_modes.size();
            m_browse_modes.resize(mode + 1);
            for (std::size_t i = original_size; i < m_browse_modes.size() - 1; ++i) {
                m_browse_modes[i].time = m_browse_modes[original_size - 1].time;
            }
        }
    }
    m_browse_modes[mode].time = time;
}

void Wnd::SetBrowseInfoWnd(const boost::shared_ptr<BrowseInfoWnd>& wnd, std::size_t mode/* = 0*/)
{ m_browse_modes.at(mode).wnd = wnd; }

void Wnd::ClearBrowseInfoWnd(std::size_t mode/* = 0*/)
{ m_browse_modes.at(mode).wnd.reset(); }

void Wnd::SetBrowseText(const std::string& text, std::size_t mode/* = 0*/)
{ m_browse_modes.at(mode).text = text; }

void Wnd::SetBrowseModes(const std::vector<BrowseInfoMode>& modes)
{ m_browse_modes = modes; }

void Wnd::SetStyleFactory(const boost::shared_ptr<StyleFactory>& factory)
{ m_style_factory = factory; }

unsigned int Wnd::DefaultBrowseTime()
{ return s_default_browse_time; }

void Wnd::SetDefaultBrowseTime(unsigned int time)
{ s_default_browse_time = time; }

const boost::shared_ptr<BrowseInfoWnd>& Wnd::DefaultBrowseInfoWnd()
{ return s_default_browse_info_wnd; }

void Wnd::SetDefaultBrowseInfoWnd(const boost::shared_ptr<BrowseInfoWnd>& browse_info_wnd)
{ s_default_browse_info_wnd = browse_info_wnd; }

Wnd::DragDropRenderingState Wnd::GetDragDropRenderingState() const
{
    DragDropRenderingState retval = NOT_DRAGGED;
    if (GUI::GetGUI()->DragDropWnd(this)) {
        if (!Dragable() && !GUI::GetGUI()->RenderingDragDropWnds())
             retval = IN_PLACE_COPY;
        else if (GUI::GetGUI()->AcceptedDragDropWnd(this))
            retval = DRAGGED_OVER_ACCEPTING_DROP_TARGET;
        else
            retval = DRAGGED_OVER_UNACCEPTING_DROP_TARGET;
    }
    return retval;
}

void Wnd::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (Dragable())
        OffsetMove(move);
    else if (!Interactive())
        ForwardEventToParent();
}

void Wnd::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::LDoubleClick(const Pt& pt, Flags<ModKey> mod_keys)
{ LClick(pt, mod_keys); }

void Wnd::MButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MClick(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MDoubleClick(const Pt& pt, Flags<ModKey> mod_keys)
{ MClick(pt, mod_keys); }

void Wnd::RButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RClick(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RDoubleClick(const Pt& pt, Flags<ModKey> mod_keys)
{ RClick(pt, mod_keys); }

void Wnd::MouseEnter(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MouseLeave()
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::DragDropEnter(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys)
{ this->DragDropHere(pt, drop_wnds_acceptable, mod_keys); }

void Wnd::DragDropHere(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys)
{
    if (!Interactive())
        ForwardEventToParent();
    this->DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);
}

void Wnd::DragDropLeave()
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::KeyRelease(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::TextInput(const std::string* text)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::GainingFocus()
{}

void Wnd::LosingFocus()
{}

void Wnd::TimerFiring(unsigned int ticks, Timer* timer)
{}

bool Wnd::EventFilter(Wnd* w, const WndEvent& event)
{ return false; }

void Wnd::HandleEvent(const WndEvent& event)
{
    for (std::vector<Wnd*>::reverse_iterator it = m_filters.rbegin(); it != m_filters.rend(); ++it) {
        if ((*it)->EventFilter(this, event))
            return;
    }

    try {
        switch (event.Type()) {
        case WndEvent::LButtonDown:
            LButtonDown(event.Point(), event.ModKeys());
            break;
        case WndEvent::LDrag:
            LDrag(event.Point(), event.DragMove(), event.ModKeys());
            break;
        case WndEvent::LButtonUp:
            LButtonUp(event.Point(), event.ModKeys());
            break;
        case WndEvent::LClick:
            LClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::LDoubleClick:
            LDoubleClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::MButtonDown:
            MButtonDown(event.Point(), event.ModKeys());
            break;
        case WndEvent::MDrag:
            MDrag(event.Point(), event.DragMove(), event.ModKeys());
            break;
        case WndEvent::MButtonUp:
            MButtonUp(event.Point(), event.ModKeys());
            break;
        case WndEvent::MClick:
            MClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::MDoubleClick:
            MDoubleClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::RButtonDown:
            RButtonDown(event.Point(), event.ModKeys());
            break;
        case WndEvent::RDrag:
            RDrag(event.Point(), event.DragMove(), event.ModKeys());
            break;
        case WndEvent::RButtonUp:
            RButtonUp(event.Point(), event.ModKeys());
            break;
        case WndEvent::RClick:
            RClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::RDoubleClick:
            RDoubleClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::MouseEnter:
            MouseEnter(event.Point(), event.ModKeys());
            break;
        case WndEvent::MouseHere:
            MouseHere(event.Point(), event.ModKeys());
            break;
        case WndEvent::MouseLeave:
            MouseLeave();
            break;
        case WndEvent::DragDropEnter:
            DragDropEnter(event.Point(), event.GetAcceptableDropWnds(), event.ModKeys());
            break;
        case WndEvent::DragDropHere:
            DragDropHere(event.Point(), event.GetAcceptableDropWnds(), event.ModKeys());
            break;
        case WndEvent::DragDropLeave:
            DragDropLeave();
            break;
        case WndEvent::DragDroppedOn:
            AcceptDrops(event.Point(), event.GetDragDropWnds(), event.ModKeys());
            break;
        case WndEvent::MouseWheel:
            MouseWheel(event.Point(), event.WheelMove(), event.ModKeys());
            break;
        case WndEvent::KeyPress:
            KeyPress(event.GetKey(), event.KeyCodePoint(), event.ModKeys());
            break;
        case WndEvent::KeyRelease:
            KeyRelease(event.GetKey(), event.KeyCodePoint(), event.ModKeys());
            break;
        case WndEvent::TextInput:
            TextInput(event.GetText());
            break;
        case WndEvent::GainingFocus:
            GainingFocus();
            break;
        case WndEvent::LosingFocus:
            LosingFocus();
            break;
        case WndEvent::TimerFiring:
            TimerFiring(event.Ticks(), event.GetTimer());
            break;
        default:
            break;
        }
    } catch (const ForwardToParentException&) {
        if (Wnd* p = Parent())
            p->HandleEvent(event);
    }
}

void Wnd::ForwardEventToParent()
{ throw ForwardToParentException(); }

void Wnd::BeginClipping()
{
    if (m_child_clipping_mode != DontClip)
        BeginClippingImpl(m_child_clipping_mode);
}

void Wnd::EndClipping()
{
    if (m_child_clipping_mode != DontClip)
        EndClippingImpl(m_child_clipping_mode);
}

void Wnd::BeginNonclientClipping()
{ BeginNonclientClippingImpl(); }

void Wnd::EndNonclientClipping()
{ EndNonclientClippingImpl(); }

void Wnd::ValidateFlags()
{
    if ((m_flags & MODAL) && (m_flags & ONTOP))
        m_flags &= ~ONTOP;
}

void Wnd::BeginClippingImpl(ChildClippingMode mode)
{
    switch (mode) {
    case DontClip:
        assert(!"Wnd::BeginClippingImpl() called with mode == DontClip; this should never happen.");
        break;
    case ClipToClient:
    case ClipToClientAndWindowSeparately:
        BeginScissorClipping(ClientUpperLeft(), ClientLowerRight());
        break;
    case ClipToWindow:
        BeginScissorClipping(UpperLeft(), LowerRight());
        break;
    }
}

void Wnd::EndClippingImpl(ChildClippingMode mode)
{
    switch (mode) {
    case DontClip:
        assert(!"Wnd::EndClippingImpl() called with mode == DontClip; this should never happen.");
        break;
    case ClipToClient:
    case ClipToWindow:
    case ClipToClientAndWindowSeparately:
        EndScissorClipping();
        break;
    }
}

void Wnd::BeginNonclientClippingImpl()
{ BeginStencilClipping(ClientUpperLeft(), ClientLowerRight(), UpperLeft(), LowerRight()); }

void Wnd::EndNonclientClippingImpl()
{ EndStencilClipping(); }

void Wnd::SetParent(Wnd* wnd)
{ m_parent = wnd; }
