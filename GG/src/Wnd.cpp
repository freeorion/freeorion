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
    namespace mi = boost::multi_index;
    struct GridLayoutWnd
    {
        GridLayoutWnd() :
            wnd(nullptr)
        {}

        GridLayoutWnd(std::shared_ptr<Wnd>& wnd_, const Pt& ul_, const Pt& lr_) : wnd(wnd_), ul(ul_), lr(lr_) {}
        std::shared_ptr<Wnd> wnd;
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
    typedef mi::multi_index_container<
        GridLayoutWnd,
        mi::indexed_by<
            mi::ordered_unique<mi::tag<Pointer>,            mi::member<GridLayoutWnd, std::shared_ptr<Wnd>, &GridLayoutWnd::wnd>>,
            mi::ordered_non_unique<mi::tag<LayoutLeft>,     mi::member<GridLayoutWnd, Pt,   &GridLayoutWnd::ul>, IsLeft>,
            mi::ordered_non_unique<mi::tag<LayoutTop>,      mi::member<GridLayoutWnd, Pt,   &GridLayoutWnd::ul>, IsTop>,
            mi::ordered_non_unique<mi::tag<LayoutRight>,    mi::member<GridLayoutWnd, Pt,   &GridLayoutWnd::lr>, IsRight>,
            mi::ordered_non_unique<mi::tag<LayoutBottom>,   mi::member<GridLayoutWnd, Pt,   &GridLayoutWnd::lr>, IsBottom>
        >
    > GridLayoutWndContainer;
    typedef GridLayoutWndContainer::index<Pointer>::type::iterator      PointerIter;
    typedef GridLayoutWndContainer::index<LayoutLeft>::type::iterator   LeftIter;
    typedef GridLayoutWndContainer::index<LayoutTop>::type::iterator    TopIter;
    typedef GridLayoutWndContainer::index<LayoutRight>::type::iterator  RightIter;
    typedef GridLayoutWndContainer::index<LayoutBottom>::type::iterator BottomIter;

    struct WndHorizontalLess
    {
        bool operator()(const std::shared_ptr<Wnd>& lhs, const std::shared_ptr<Wnd>& rhs) const
            {return lhs->Left() < rhs->Left();}
    };

    struct WndVerticalLess
    {
        bool operator()(const std::shared_ptr<Wnd>& lhs, const std::shared_ptr<Wnd>& rhs) const
            {return lhs->Top() < rhs->Top();}
    };

    const int DEFAULT_LAYOUT_BORDER_MARGIN = 0;
    const int DEFAULT_LAYOUT_CELL_MARGIN = 5;

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
std::shared_ptr<BrowseInfoWnd> Wnd::s_default_browse_info_wnd;

Wnd::Wnd() :
    std::enable_shared_from_this<Wnd>(),
    m_parent(),
    m_child_clipping_mode(DontClip),
    m_upperleft(X0, Y0),
    m_lowerright(X1, Y1),
    m_max_size(X(1 << 30), Y(1 << 30)),
    m_layout(),
    m_containing_layout(),
    m_flags()
{
    m_browse_modes.resize(1);
    m_browse_modes[0].time = s_default_browse_time;
    m_browse_modes[0].wnd = s_default_browse_info_wnd;
}

Wnd::Wnd(X x, Y y, X w, Y h, Flags<WndFlag> flags/* = INTERACTIVE | DRAGABLE*/) :
    Wnd()
{
    m_upperleft = Pt(x, y);
    m_lowerright = Pt(x + w, y + h);

    m_flags = flags;
    ValidateFlags();
}

Wnd::~Wnd()
{
    // remove this-references from Wnds that this Wnd filters
    for (auto& weak_filtered_wnd : m_filtering) {
        auto filtering_wnd = weak_filtered_wnd.lock();
        if (!filtering_wnd)
            continue;

        // The weak pointer in the filtered window pointing to "this" will be
        // expired, since we are in ~Wnd().  Remove all expired weak_ptr from the
        // filtered window.
        std::vector<std::weak_ptr<Wnd>> good_filters;
        good_filters.reserve(filtering_wnd->m_filters.size());
        for (const auto& weak_filtering_wnd : filtering_wnd->m_filters)
            if (!weak_filtering_wnd.expired())
                good_filters.push_back(weak_filtering_wnd);
        good_filters.swap(filtering_wnd->m_filters);
    }

    // remove this-references from Wnds that filter this Wnd
    for (auto& weak_filter_wnd : m_filters) {
        auto filter_wnd = weak_filter_wnd.lock();
        if (!filter_wnd)
            continue;

        // The weak pointer in the filtering window pointing to "this" will be
        // expired, since we are in ~Wnd().  Remove all expired weak_ptr from the
        // filtered window.
        auto it = filter_wnd->m_filtering.begin();
        while (it != filter_wnd->m_filtering.end()) {
            if (it->expired())
                it = filter_wnd->m_filtering.erase(it);
            else
                ++it;
        }
    }
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
{ return !Parent() && m_flags & ONTOP; }

bool Wnd::Modal() const
{ return !Parent() && m_flags & MODAL; }

Wnd::ChildClippingMode Wnd::GetChildClippingMode() const
{ return m_child_clipping_mode; }

bool Wnd::NonClientChild() const
{ return m_non_client_child; }

bool Wnd::Visible() const
{ return m_visible; }

bool Wnd::PreRenderRequired() const
{
    if (m_needs_prerender)
        return true;

    auto&& layout = GetLayout();

    return (layout && layout->m_needs_prerender);
}

const std::string& Wnd::Name() const
{ return m_name; }

const std::string& Wnd::DragDropDataType() const
{ return m_drag_drop_data_type; }

void Wnd::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                          const Pt& pt, Flags<ModKey> mod_keys) const
{
    // default reject all drops. derived classes can override to accept drops
    for (auto& it = first; it != last; ++it)
        it->second = false;
}

Pt Wnd::UpperLeft() const
{
    Pt retval = m_upperleft;
    auto&& parent = Parent();
    if (parent)
        retval += parent->ClientUpperLeft();
    return retval;
}

X Wnd::Left() const
{ return UpperLeft().x; }

Y Wnd::Top() const
{ return UpperLeft().y; }

Pt Wnd::LowerRight() const
{
    Pt retval = m_lowerright;
    auto&& parent = Parent();
    if (parent)
        retval += parent->ClientUpperLeft();
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

Pt Wnd::Size() const
{ return Pt(m_lowerright.x - m_upperleft.x, m_lowerright.y - m_upperleft.y); }

Pt Wnd::MinSize() const
{ return m_min_size; }

Pt Wnd::MaxSize() const
{ return m_max_size; }

Pt Wnd::MinUsableSize() const
{
    auto&& layout = GetLayout();
    return layout ? layout->MinUsableSize() : Size();
}

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

const std::list<std::shared_ptr<Wnd>>& Wnd::Children() const
{ return m_children; }

std::shared_ptr<Wnd> Wnd::Parent() const
{ return LockAndResetIfExpired(m_parent); }

std::shared_ptr<Wnd> Wnd::RootParent() const
{
    auto&& parent = Parent();
    auto&& gparent = parent ? parent->Parent() : nullptr;
    while (gparent) {
        parent = std::move(gparent);
        gparent = parent->Parent();
    }
    return parent;
}

std::shared_ptr<Layout> Wnd::GetLayout() const
{ return LockAndResetIfExpired(m_layout); }

Layout* Wnd::ContainingLayout() const
{ return LockAndResetIfExpired(m_containing_layout).get(); }

const std::vector<Wnd::BrowseInfoMode>& Wnd::BrowseModes() const
{ return m_browse_modes; }

const std::string& Wnd::BrowseInfoText(std::size_t mode) const
{ return m_browse_modes.at(mode).text; }

const std::shared_ptr<StyleFactory>& Wnd::GetStyleFactory() const
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

void Wnd::ClampRectWithMinAndMaxSize(Pt& ul, Pt& lr) const
{
    Pt min_sz = MinSize();
    Pt max_sz = MaxSize();
    auto&& layout = GetLayout();
    if (layout) {
        Pt layout_min_sz = layout->MinSize() + (Size() - ClientSize());
        min_sz.x = std::max(min_sz.x, layout_min_sz.x);
        min_sz.y = std::max(min_sz.y, layout_min_sz.y);
    }
    if (lr.x - ul.x < min_sz.x) {
        if (ul.x != m_upperleft.x)
            ul.x = lr.x - min_sz.x;
        else
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
        else
            lr.y = ul.y + min_sz.y;
    } else if (max_sz.y < lr.y - ul.y) {
        if (lr.y != m_lowerright.y)
            lr.y = ul.y + max_sz.y;
        else
            ul.y = lr.y - max_sz.y;
    }
}

void Wnd::SetDragDropDataType(const std::string& data_type)
{ m_drag_drop_data_type = data_type; }

void Wnd::StartingChildDragDrop(const Wnd* wnd, const Pt& offset)
{}

void Wnd::AcceptDrops(const Pt& pt, std::vector<std::shared_ptr<Wnd>> wnds, Flags<ModKey> mod_keys)
{
    if (!Interactive() && Parent())
        ForwardEventToParent();
    // if dropped Wnds were accepted, but no handler take ownership they will be destroyed
}

void Wnd::CancellingChildDragDrop(const std::vector<const Wnd*>& wnds)
{}

void Wnd::ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination)
{
    for (auto& wnd : wnds) {
        DetachChild(wnd);
    }
}

void Wnd::SetName(const std::string& name)
{ m_name = name; }

void Wnd::Hide()
{
    m_visible = false;
    for (auto& child : m_children)
        child->Hide();
}

void Wnd::Show()
{
    m_visible = true;
    for (auto& child : m_children)
        child->Show();
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
    bool resized = (original_sz != (lr - ul));
    if (resized)
        ClampRectWithMinAndMaxSize(ul, lr);

    m_upperleft = ul;
    m_lowerright = lr;
    if (resized) {
        bool size_changed = Size() != original_sz;
        auto&& layout = GetLayout();
        if (layout && size_changed)
            layout->Resize(ClientSize());
        if (size_changed && !dynamic_cast<Layout*>(this))
            if (const auto&& containing_layout = LockAndResetIfExpired(m_containing_layout))
                containing_layout->ChildSizeOrMinSizeChanged();
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
    // The previous Resize() will call ChildSizeOrMinSizeChanged() itself if needed
    else if (min_size_changed && !dynamic_cast<Layout*>(this)) {
        if (auto&& containing_layout = LockAndResetIfExpired(m_containing_layout))
            containing_layout->ChildSizeOrMinSizeChanged();
    }
}

void Wnd::SetMaxSize(const Pt& sz)
{
    m_max_size = sz;
    if (m_max_size.x < Width() || m_max_size.y < Height())
        Resize(Pt(std::min(Width(), m_max_size.x), std::min(Height(), m_max_size.y)));
}

void Wnd::AttachChild(std::shared_ptr<Wnd> wnd)
{
    if (!wnd)
        return;

    try {
        // TODO use weak_from_this when converting to C++17
        auto my_shared = shared_from_this();

        // Remove from previous parent.
        if (auto&& parent = wnd->Parent())
            parent->DetachChild(wnd.get());

        GUI::GetGUI()->Remove(wnd);
        wnd->SetParent(my_shared);

        if (auto this_as_layout = std::dynamic_pointer_cast<Layout>(my_shared))
            wnd->m_containing_layout = this_as_layout;

        m_children.push_back(std::forward<std::shared_ptr<Wnd>>(wnd));

    } catch (const std::bad_weak_ptr&) {
        std::cerr << std::endl << "Wnd::AttachChild called either during the constructor "
                  << "or after the destructor has run. Not attaching child."
                  << std::endl << " parent = " << m_name << " child = " << wnd->m_name;
        // Soft failure:
        // Intentionally do nothing, to create minimal disruption to non-dev
        // players if a dev accidentally puts an AttachChild in its own constructor.
    }
}

void Wnd::MoveChildUp(const std::shared_ptr<Wnd>& wnd)
{ MoveChildUp(wnd.get()); }

void Wnd::MoveChildUp(Wnd* wnd)
{
    if (!wnd)
        return;
    const auto it = std::find_if(m_children.begin(), m_children.end(),
                                 [&wnd](const std::shared_ptr<Wnd>& x){ return x.get() == wnd; });
    if (it == m_children.end())
        return;
    m_children.push_back(*it);
    m_children.erase(it);
}

void Wnd::MoveChildDown(const std::shared_ptr<Wnd>& wnd)
{ MoveChildDown(wnd.get()); }

void Wnd::MoveChildDown(Wnd* wnd)
{
    if (!wnd)
        return;
    auto found = std::find_if(m_children.begin(), m_children.end(),
                              [&wnd](const std::shared_ptr<Wnd>& x){ return x.get() == wnd; });
    if (found == m_children.end())
        return;

    m_children.push_front(*found);
    m_children.erase(found);
}

void Wnd::DetachChild(const std::shared_ptr<Wnd>& wnd)
{ DetachChild(wnd.get()); }

void Wnd::DetachChild(Wnd* wnd)
{
    const auto it = std::find_if(m_children.begin(), m_children.end(),
                                 [&wnd](const std::shared_ptr<Wnd>& x){ return x.get() == wnd; });
    if (it == m_children.end())
        return;

    DetachChildCore(wnd);

    m_children.erase(it);
}

void Wnd::DetachChildCore(Wnd* wnd)
{
    if (!wnd)
        return;

    wnd->m_parent.reset();

    auto&& layout = GetLayout();
    if (layout && wnd == layout.get())
        m_layout.reset();

    if (auto this_as_layout = dynamic_cast<Layout*>(this)) {
        this_as_layout->Remove(wnd);
        wnd->m_containing_layout.reset();
    }
}

void Wnd::DetachChildren()
{
    m_layout.reset();

    for (auto& wnd : m_children) {
        DetachChildCore(wnd.get());
    }
    m_children.clear();
}

void Wnd::InstallEventFilter(const std::shared_ptr<Wnd>& wnd)
{
    if (!wnd)
        return;
    RemoveEventFilter(wnd);
    m_filters.push_back(std::weak_ptr<Wnd>(wnd));
    wnd->m_filtering.insert(shared_from_this());
}

void Wnd::RemoveEventFilter(const std::shared_ptr<Wnd>& wnd)
{
    if (!wnd)
        return;
    const auto& it = std::find_if(m_filters.begin(), m_filters.end(),
                                  [&wnd](const std::weak_ptr<Wnd>& x){ return x.lock() == wnd; });
    if (it != m_filters.end())
        m_filters.erase(it);
    wnd->m_filtering.erase(shared_from_this());
}

void Wnd::HorizontalLayout()
{
    RemoveLayout();

    std::multiset<std::shared_ptr<Wnd>, WndHorizontalLess> wnds;
    Pt client_sz = ClientSize();
    for (auto& child : m_children) {
        Pt wnd_ul = child->RelativeUpperLeft(), wnd_lr = child->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;
        wnds.insert(child);
    }

    auto layout = Wnd::Create<Layout>(X0, Y0, ClientSize().x, ClientSize().y,
                                      1, wnds.size(),
                                      DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    m_layout = layout;
    AttachChild(layout);

    int i = 0;
    for (const auto& wnd : wnds) {
        layout->Add(wnd, 0, i++);
    }
}

void Wnd::VerticalLayout()
{
    RemoveLayout();

    std::multiset<std::shared_ptr<Wnd>, WndVerticalLess> wnds;
    Pt client_sz = ClientSize();
    for (auto& child : m_children) {
        Pt wnd_ul = child->RelativeUpperLeft(), wnd_lr = child->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;
        wnds.insert(child);
    }

    auto layout = Wnd::Create<Layout>(X0, Y0, ClientSize().x, ClientSize().y,
                                      wnds.size(), 1,
                                      DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    m_layout = layout;
    AttachChild(layout);

    int i = 0;
    for (auto& wnd : wnds) {
        layout->Add(wnd, i++, 0);
    }
}

void Wnd::GridLayout()
{
    RemoveLayout();

    Pt client_sz = ClientSize();

    GridLayoutWndContainer grid_layout;

    // validate existing children and place them in a grid with one cell per pixel
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        auto& wnd = *it;
        Pt wnd_ul = wnd->RelativeUpperLeft(), wnd_lr = wnd->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;

        auto it2 = it;
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
            if (grid_layout.get<LayoutRight>().count(x + 1, IsRight())) {
                break;
            } else if (grid_layout.get<LayoutLeft>().count(x, IsLeft())) {
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
            if (grid_layout.get<LayoutLeft>().count(x - 1, IsLeft())) {
                break;
            } else if (grid_layout.get<LayoutRight>().count(x, IsRight())) {
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
            if (grid_layout.get<LayoutBottom>().count(y + 1, IsBottom())) {
                break;
            } else if (grid_layout.get<LayoutTop>().count(y, IsTop())) {
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
            if (grid_layout.get<LayoutTop>().count(y - 1, IsTop())) {
                break;
            } else if (grid_layout.get<LayoutBottom>().count(y, IsBottom())) {
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
    for (const GridLayoutWnd& layout_wnd : grid_layout.get<LayoutLeft>()) {
        unique_lefts.insert(layout_wnd.ul.x);
    }
    for (const GridLayoutWnd& layout_wnd : grid_layout.get<LayoutTop>()) {
        unique_tops.insert(layout_wnd.ul.y);
    }

    if (unique_lefts.empty() || unique_tops.empty())
        return;

    auto layout = Wnd::Create<Layout>(X0, Y0, ClientSize().x, ClientSize().y,
                                      unique_tops.size(), unique_lefts.size(),
                                      DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    m_layout = layout;
    AttachChild(layout);

    // populate this new layout with the child windows, based on their placements in the pixel-grid layout
    for (const GridLayoutWnd& layout_wnd : grid_layout.get<Pointer>()) {
        auto& wnd = layout_wnd.wnd;
        Pt ul = layout_wnd.ul;
        Pt lr = layout_wnd.lr;
        int left = std::distance(unique_lefts.begin(), unique_lefts.find(ul.x));
        int top = std::distance(unique_tops.begin(), unique_tops.find(ul.y));
        int right = std::distance(unique_lefts.begin(), unique_lefts.lower_bound(lr.x));
        int bottom = std::distance(unique_tops.begin(), unique_tops.lower_bound(lr.y));
        layout->Add(wnd, top, left, bottom - top, right - left);
    }
}

void Wnd::SetLayout(const std::shared_ptr<Layout>& layout)
{
    auto&& mm_layout = GetLayout();
    if (layout == mm_layout || layout == LockAndResetIfExpired(m_containing_layout))
        throw BadLayout("Wnd::SetLayout() : Attempted to set a Wnd's layout to be its current layout or the layout that contains the Wnd");
    RemoveLayout();
    auto children = m_children;
    DetachChildren();
    Pt client_sz = ClientSize();
    for (auto& wnd : children) {
        Pt wnd_ul = wnd->RelativeUpperLeft(), wnd_lr = wnd->RelativeLowerRight();
        if (wnd_ul.x < 0 || wnd_ul.y < 0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            AttachChild(wnd);
    }
    AttachChild(layout);
    m_layout = layout;
    layout->SizeMove(Pt(), Pt(ClientWidth(), ClientHeight()));
}

void Wnd::RemoveLayout()
{
    auto&& layout = GetLayout();
    m_layout.reset();
    if (!layout)
        return;

    auto layout_children = layout->Children();
    layout->DetachAndResetChildren();
    for (auto& wnd : layout_children) {
        AttachChild(wnd);
    }
}

std::shared_ptr<Layout> Wnd::DetachLayout()
{
    auto&& layout = GetLayout();
    DetachChild(layout.get());
    return layout;
}

void Wnd::SetLayoutBorderMargin(unsigned int margin)
{
    if (auto&& layout = GetLayout())
        layout->SetBorderMargin(margin);
}

void Wnd::SetLayoutCellMargin(unsigned int margin)
{
    if (auto&& layout = GetLayout())
        layout->SetCellMargin(margin);
}

void Wnd::PreRender()
{
    m_needs_prerender = false;
    auto&& layout = GetLayout();
    if (!layout)
        return;
    if (layout->m_needs_prerender)
        layout->PreRender();
}

void Wnd::RequirePreRender()
{ m_needs_prerender = true; }

void Wnd::Render() {}

bool Wnd::Run()
{
    bool retval = false;
    auto&& parent = Parent();
    if (!parent && m_flags & MODAL) {
        GUI* gui = GUI::GetGUI();
        gui->RegisterModal(shared_from_this());
        ModalInit();
        m_done = false;
        std::shared_ptr<ModalEventPump> pump = gui->CreateModalEventPump(m_done);
        (*pump)();
        gui->Remove(shared_from_this());
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

void Wnd::SetBrowseInfoWnd(const std::shared_ptr<BrowseInfoWnd>& wnd, std::size_t mode/* = 0*/)
{ m_browse_modes.at(mode).wnd = wnd; }

void Wnd::ClearBrowseInfoWnd(std::size_t mode/* = 0*/)
{ m_browse_modes.at(mode).wnd.reset(); }

void Wnd::SetBrowseText(const std::string& text, std::size_t mode/* = 0*/)
{ m_browse_modes.at(mode).text = text; }

void Wnd::SetBrowseModes(const std::vector<BrowseInfoMode>& modes)
{ m_browse_modes = modes; }

void Wnd::SetStyleFactory(const std::shared_ptr<StyleFactory>& factory)
{ m_style_factory = factory; }

unsigned int Wnd::DefaultBrowseTime()
{ return s_default_browse_time; }

void Wnd::SetDefaultBrowseTime(unsigned int time)
{ s_default_browse_time = time; }

const std::shared_ptr<BrowseInfoWnd>& Wnd::DefaultBrowseInfoWnd()
{ return s_default_browse_info_wnd; }

void Wnd::SetDefaultBrowseInfoWnd(const std::shared_ptr<BrowseInfoWnd>& browse_info_wnd)
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
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::DragDropHere(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys)
{
    if (!Interactive())
        ForwardEventToParent();
    this->DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);
}

void Wnd::CheckDrops(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                     Flags<ModKey> mod_keys)
{
    if (!Interactive())
        ForwardEventToParent();
    this->DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);
}

void Wnd::DragDropLeave()
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::KeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
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
    // Check if any of the filters block this event
    bool filtered = false;
    ProcessThenRemoveExpiredPtrs(
        m_filters,
        [&filtered, this, &event](std::shared_ptr<Wnd>& wnd)
        {
            if (!filtered)
                filtered = wnd->EventFilter(this, event);
        });

    if (filtered)
        return;

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
        case WndEvent::CheckDrops:
            CheckDrops(event.Point(), event.GetAcceptableDropWnds(), event.ModKeys());
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
        if (auto&& p = Parent())
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

void Wnd::SetParent(const std::shared_ptr<Wnd>& wnd)
{ m_parent = wnd; }
