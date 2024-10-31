//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <GG/BrowseInfoWnd.h>
#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/WndEvent.h>
#include <GG/Wnd.h>


using namespace GG;

namespace {
enum class TestEnum : int8_t { INVALID = -1, VALID = 0, MAX = 1, OUT_OF_RANGE, END, ZERO = 0, ONE, TWO, THREE };
constexpr auto* test_enum_text = "INVALID = -1, VALID = 0, MAX = 1, OUT_OF_RANGE, END";

constexpr auto split_result = GG::EnumMap<TestEnum>::Split(test_enum_text, ',');
static_assert(split_result.first == "INVALID = -1");
static_assert(split_result.second == " VALID = 0, MAX = 1, OUT_OF_RANGE, END");

constexpr auto trimmed_result = GG::EnumMap<TestEnum>::Trim(split_result.first);
static_assert(trimmed_result == "INVALID = -1");

constexpr auto split_apply_result = GG::EnumMap<TestEnum>::SplitApply(
    test_enum_text, GG::EnumMap<TestEnum>::Trim, ',');
static_assert(split_apply_result.first == 5);
static_assert(split_apply_result.second[0] == "INVALID = -1");
static_assert(split_apply_result.second[1] == "VALID = 0");
static_assert(split_apply_result.second[2] == "MAX = 1");
static_assert(split_apply_result.second[3] == "OUT_OF_RANGE");
static_assert(split_apply_result.second[4] == "END");

constexpr GG::EnumMap<TestEnum> cmap(test_enum_text);
static_assert(cmap["MAX"] == TestEnum::MAX);
static_assert(cmap["OUT_OF_RANGE"] == TestEnum::OUT_OF_RANGE);


constexpr auto em = GG::CGetEnumMap<GG::WndRegion>();
constexpr auto qq = em[GG::WndRegion::WR_TOPLEFT];
constexpr auto rr = GG::to_string(GG::WndRegion::WR_TOPLEFT);
static_assert(rr == "WR_TOPLEFT");
static_assert(rr == qq);


namespace mi = boost::multi_index;

struct GridLayoutWnd
{
    GridLayoutWnd() {}

    GridLayoutWnd(std::shared_ptr<Wnd>& wnd_, Pt ul_, Pt lr_) : wnd(wnd_), ul(ul_), lr(lr_) {}
    std::shared_ptr<Wnd> wnd;
    Pt ul;
    Pt lr;
};
struct IsLeft
{
    bool operator()(Pt lhs, Pt rhs) const {return lhs.x < rhs.x;}
    bool operator()(X x, Pt pt) const     {return x < pt.x;}
    bool operator()(Pt pt, X x) const     {return pt.x < x;}
};
struct IsTop
{
    bool operator()(Pt lhs, Pt rhs) const {return lhs.y < rhs.y;}
    bool operator()(Y y, Pt pt) const     {return y < pt.y;}
    bool operator()(Pt pt, Y y) const     {return pt.y < y;}
};
struct IsRight
{
    bool operator()(Pt lhs, Pt rhs) const {return rhs.x < lhs.x;}
    bool operator()(X x, Pt pt) const     {return pt.x < x;}
    bool operator()(Pt pt, X x) const     {return x < pt.x;}
};
struct IsBottom
{
    bool operator()(Pt lhs, Pt rhs) const {return rhs.y < lhs.y;}
    bool operator()(Y y, Pt pt) const     {return pt.y < y;}
    bool operator()(Pt pt, Y y) const     {return y < pt.y;}
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

constexpr int DEFAULT_LAYOUT_BORDER_MARGIN = 0;
constexpr int DEFAULT_LAYOUT_CELL_MARGIN = 5;

struct ForwardToParentException {};

}

///////////////////////////////////////
// WndFlags
///////////////////////////////////////
GG_FLAGSPEC_IMPL(WndFlag);

namespace {

bool RegisterWndFlags()
{
    FlagSpec<WndFlag>& spec = FlagSpec<WndFlag>::instance();
    spec.insert(NO_WND_FLAGS,       "NO_WND_FLAGS");
    spec.insert(INTERACTIVE,        "INTERACTIVE");
    spec.insert(REPEAT_BUTTON_DOWN, "REPEAT_BUTTON_DOWN");
    spec.insert(DRAGABLE,           "DRAGABLE");
    spec.insert(RESIZABLE,          "RESIZABLE");
    spec.insert(ONTOP,              "ONTOP");
    spec.insert(MODAL,              "MODAL");
    spec.insert(REPEAT_KEY_PRESS,   "REPEAT_KEY_PRESS");
    return true;
}
bool dummy = RegisterWndFlags();

}


///////////////////////////////////////
// class GG::Wnd
///////////////////////////////////////
unsigned int Wnd::s_default_browse_time = 1500;
std::shared_ptr<BrowseInfoWnd> Wnd::s_default_browse_info_wnd;

Wnd::Wnd(X x, Y y, X w, Y h, Flags<WndFlag> flags) :
    m_upperleft{x, y},
    m_lowerright{x + w, y + h},
    m_flags{flags}
{ ValidateFlags(); }

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
                good_filters.emplace_back(weak_filtering_wnd);
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

bool Wnd::PreRenderRequired() const
{
    if (m_needs_prerender)
        return true;

    auto layout = GetLayout();

    return (layout && layout->m_needs_prerender);
}

void Wnd::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                          Pt pt, Flags<ModKey> mod_keys) const
{
    // default reject all drops. derived classes can override to accept drops
    for (auto& it = first; it != last; ++it)
        it->second = false;
}

Pt Wnd::UpperLeft() const noexcept
{
    Pt retval = m_upperleft;
    if (auto parent = Parent())
        retval += parent->ClientUpperLeft();
    return retval;
}

Pt Wnd::LowerRight() const noexcept
{
    Pt retval = m_lowerright;
    if (auto&& parent = Parent())
        retval += parent->ClientUpperLeft();
    return retval;
}

Pt Wnd::MinUsableSize() const
{
    auto layout = GetLayout();
    return layout ? layout->MinUsableSize() : Size();
}

std::shared_ptr<Wnd> Wnd::Parent() const noexcept
{ return LockAndResetIfExpired(m_parent); }

bool Wnd::IsAncestorOf(const std::shared_ptr<Wnd>& wnd) const noexcept
{
    // Is this Wnd one of wnd's (direct or indirect) parents?
    if (!wnd)
        return false;
    auto&& parent_of_wnd = wnd->Parent();
    while (parent_of_wnd) {
        if (parent_of_wnd.get() == this)
            return true;
        parent_of_wnd = parent_of_wnd->Parent();
    }
    return false;
}

std::shared_ptr<Wnd> Wnd::RootParent() const noexcept
{
    auto parent{Parent()};
    auto gparent{parent ? parent->Parent() : nullptr};
    while (gparent) {
        parent = std::move(gparent);
        gparent = parent->Parent();
    }
    return parent;
}

std::shared_ptr<Layout> Wnd::GetLayout() const noexcept
{ return LockAndResetIfExpired(m_layout); }

Layout* Wnd::ContainingLayout() const noexcept
{ return LockAndResetIfExpired(m_containing_layout).get(); }

const StyleFactory& Wnd::GetStyleFactory() const noexcept
{ return m_style_factory ? std::as_const(*m_style_factory) : GUI::GetGUI()->GetStyleFactory(); }

WndRegion Wnd::WindowRegion(Pt pt) const
{
    static constexpr int LEFT = 0;
    static constexpr int MIDDLE = 1;
    static constexpr int RIGHT = 2;
    static constexpr int TOP = 0;
    static constexpr int BOTTOM = 2;

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

    static_assert(std::is_signed_v<std::underlying_type_t<WndRegion>>);
    return (Resizable() ? WndRegion(x_pos + 3 * y_pos) : WndRegion::WR_NONE);
}

void Wnd::ClampRectWithMinAndMaxSize(Pt& ul, Pt& lr) const
{
    Pt min_sz = MinSize();
    Pt max_sz = MaxSize();

    if (auto layout = GetLayout()) {
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

void Wnd::AcceptDrops(Pt pt, std::vector<std::shared_ptr<Wnd>> wnds, Flags<ModKey> mod_keys)
{
    if (!Interactive() && Parent())
        ForwardEventToParent();
    // if dropped Wnds were accepted, but no handler take ownership they will be destroyed
}

void Wnd::ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination)
{
    for (auto& wnd : wnds)
        DetachChild(wnd);
}

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

void Wnd::MoveTo(Pt pt)
{ SizeMove(pt, pt + Size()); }

void Wnd::OffsetMove(Pt pt)
{ SizeMove(m_upperleft + pt, m_lowerright + pt); }

void Wnd::SizeMove(Pt ul_, Pt lr_)
{
    auto ul = ul_, lr = lr_;
    const auto original_sz = Size();
    const bool resized = (original_sz != (lr - ul));
    if (resized)
        ClampRectWithMinAndMaxSize(ul, lr);

    m_upperleft = ul;
    m_lowerright = lr;
    if (resized) {
        const bool size_changed = Size() != original_sz;
        auto layout = GetLayout();
        if (layout && size_changed)
            layout->Resize(ClientSize());
        if (size_changed && !dynamic_cast<Layout*>(this))
            if (const auto&& containing_layout = LockAndResetIfExpired(m_containing_layout))
                containing_layout->ChildSizeOrMinSizeChanged();
    }
}

void Wnd::Resize(Pt sz)
{ SizeMove(m_upperleft, m_upperleft + sz); }

void Wnd::SetMinSize(Pt sz)
{
    const bool min_size_changed = m_min_size != sz;
    m_min_size = sz;
    if (Width() < m_min_size.x || Height() < m_min_size.y) {
        Resize(Pt(std::max(Width(), m_min_size.x), std::max(Height(), m_min_size.y)));
    // The previous Resize() will call ChildSizeOrMinSizeChanged() itself if needed
    } else if (min_size_changed && !dynamic_cast<Layout*>(this)) {
        if (auto&& containing_layout = LockAndResetIfExpired(m_containing_layout))
            containing_layout->ChildSizeOrMinSizeChanged();
    }
}

void Wnd::SetMaxSize(Pt sz)
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

        auto this_as_layout = std::dynamic_pointer_cast<Layout>(my_shared);

        GUI::GetGUI()->Remove(wnd);
        wnd->SetParent(std::move(my_shared));

        if (this_as_layout)
            wnd->m_containing_layout = this_as_layout;

        m_children.push_back(std::move(wnd));

    } catch (const std::bad_weak_ptr&) {
        std::cerr << "\nWnd::AttachChild called either during the constructor "
                  << "or after the destructor has run. Not attaching child.\n"
                  << " parent = " << m_name << " child = " << (wnd ? wnd->m_name : "???");
        // Soft failure:
        // Intentionally do nothing, to create minimal disruption to non-dev
        // players if a dev accidentally puts an AttachChild in its own constructor.
    }
}

void Wnd::MoveChildUp(const std::shared_ptr<Wnd>& wnd)
{ MoveChildUp(wnd.get()); }

void Wnd::MoveChildUp(Wnd* wnd)
{
    if (!wnd || m_children.empty() || m_children.back().get() == wnd)
        return;
    const auto found_it = std::find_if(m_children.begin(), m_children.end(),
                                      [&wnd](const std::shared_ptr<Wnd>& x){ return x.get() == wnd; });
    if (found_it == m_children.end())
        return;

    auto found{std::move(*found_it)};
    m_children.erase(found_it);
    m_children.push_back(std::move(found));
}

void Wnd::MoveChildDown(const std::shared_ptr<Wnd>& wnd)
{ MoveChildDown(wnd.get()); }

void Wnd::MoveChildDown(Wnd* wnd)
{
    if (!wnd)
        return;
    auto found_it = std::find_if(m_children.begin(), m_children.end(),
                                 [&wnd](const std::shared_ptr<Wnd>& x){ return x.get() == wnd; });
    if (found_it == m_children.end())
        return;

    auto found{std::move(*found_it)};
    m_children.erase(found_it);
    m_children.insert(m_children.begin(), std::move(found));
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
    wnd->m_containing_layout.reset();

    auto&& layout = GetLayout();
    if (layout && wnd == layout.get())
        m_layout.reset();
}

void Wnd::DetachChildren()
{
    m_layout.reset();

    for (auto& wnd : m_children)
        DetachChildCore(wnd.get());

    m_children.clear();
}

void Wnd::InstallEventFilter(std::shared_ptr<Wnd> wnd)
{
    if (!wnd)
        return;
    RemoveEventFilter(wnd);

    m_filters.push_back(wnd);
    wnd->m_filtering.insert(shared_from_this());
}

void Wnd::RemoveEventFilter(const std::shared_ptr<Wnd>& wnd)
{
    if (!wnd)
        return;
    const auto it = std::find_if(m_filters.begin(), m_filters.end(),
                                 [&wnd](const std::weak_ptr<Wnd>& x){ return x.lock() == wnd; });
    if (it != m_filters.end())
        m_filters.erase(it);
    wnd->m_filtering.erase(shared_from_this());
}

void Wnd::HorizontalLayout()
{
    RemoveLayout();

    std::vector<std::shared_ptr<Wnd>> wnds;
    wnds.reserve(m_children.size());
    Pt client_sz = ClientSize();
    for (auto& child : m_children) {
        Pt wnd_ul = child->RelativeUpperLeft(), wnd_lr = child->RelativeLowerRight();
        if (wnd_ul.x < X0 || wnd_ul.y < Y0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;
        wnds.push_back(child);
    }
    std::sort(wnds.begin(), wnds.end(), [](const auto& l, const auto& r) { return l->Left() < r->Left(); });

    auto layout = Wnd::Create<Layout>(X0, Y0, ClientSize().x, ClientSize().y,
                                      1, wnds.size(),
                                      DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    m_layout = layout;
    AttachChild(layout);

    int i = 0;
    for (auto& wnd : wnds)
        layout->Add(wnd, 0, i++);
}

void Wnd::VerticalLayout()
{
    RemoveLayout();

    std::vector<std::shared_ptr<Wnd>> wnds;
    wnds.reserve(m_children.size());
    Pt client_sz = ClientSize();
    for (auto& child : m_children) {
        Pt wnd_ul = child->RelativeUpperLeft(), wnd_lr = child->RelativeLowerRight();
        if (wnd_ul.x < X0 || wnd_ul.y < Y0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            continue;
        wnds.push_back(child);
    }
    std::sort(wnds.begin(), wnds.end(), [](const auto& l, const auto& r) { return l->Top() < r->Top(); });

    auto layout = Wnd::Create<Layout>(X0, Y0, ClientSize().x, ClientSize().y,
                                      wnds.size(), 1,
                                      DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    m_layout = layout;
    AttachChild(layout);

    int i = 0;
    for (auto& wnd : wnds)
        layout->Add(wnd, i++, 0);
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
        if (wnd_ul.x < X0 || wnd_ul.y < Y0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
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
        for (X x = ul.x - 1; x >= X0; --x) {
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
        for (Y y = ul.y - Y1; y >= Y0; --y) {
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
    for (const GridLayoutWnd& layout_wnd : grid_layout.get<LayoutLeft>())
        unique_lefts.emplace(layout_wnd.ul.x);
    for (const GridLayoutWnd& layout_wnd : grid_layout.get<LayoutTop>())
        unique_tops.emplace(layout_wnd.ul.y);

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
        const auto ul = layout_wnd.ul;
        const auto lr = layout_wnd.lr;
        const auto left = std::distance(unique_lefts.begin(), unique_lefts.find(ul.x));
        const auto top = std::distance(unique_tops.begin(), unique_tops.find(ul.y));
        const auto right = std::distance(unique_lefts.begin(), unique_lefts.lower_bound(lr.x));
        const auto bottom = std::distance(unique_tops.begin(), unique_tops.lower_bound(lr.y));
        const auto height_sz = static_cast<size_t>((bottom >= top) ? (bottom - top) : 0u);
        const auto width_sz = static_cast<size_t>((right >= left) ? (right - left) : 0u);
        const auto top_sz = top > 0 ? static_cast<size_t>(top) : 0u;
        const auto left_sz = left > 0 ? static_cast<size_t>(left) : 0u;
        layout->Add(wnd, top_sz, left_sz, height_sz, width_sz);
    }
}

void Wnd::SetLayout(const std::shared_ptr<Layout>& layout)
{
    auto&& mm_layout = GetLayout();
    if (layout == mm_layout || layout == LockAndResetIfExpired(m_containing_layout))
        throw BadLayout("Wnd::SetLayout() : Attempted to set a Wnd's layout to be its current layout or the layout that contains the Wnd");
    RemoveLayout();
    const auto children{m_children};
    DetachChildren();
    Pt client_sz = ClientSize();
    for (auto& wnd : children) {
        Pt wnd_ul = wnd->RelativeUpperLeft(), wnd_lr = wnd->RelativeLowerRight();
        if (wnd_ul.x < X0 || wnd_ul.y < Y0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            AttachChild(wnd);
    }
    AttachChild(layout);
    m_layout = layout;
    layout->SizeMove(Pt(), Pt(ClientWidth(), ClientHeight()));
}

void Wnd::SetLayout(std::shared_ptr<Layout>&& layout)
{
    auto mm_layout = GetLayout();
    if (layout == mm_layout || layout == LockAndResetIfExpired(m_containing_layout))
        throw BadLayout("Wnd::SetLayout() : Attempted to set a Wnd's layout to be its current layout or the layout that contains the Wnd");
    RemoveLayout();
    const auto children{m_children};
    DetachChildren();
    const Pt client_sz = ClientSize();
    for (auto& wnd : children) {
        Pt wnd_ul = wnd->RelativeUpperLeft(), wnd_lr = wnd->RelativeLowerRight();
        if (wnd_ul.x < X0 || wnd_ul.y < Y0 || client_sz.x < wnd_lr.x || client_sz.y < wnd_lr.y)
            AttachChild(wnd);
    }
    AttachChild(layout);
    m_layout = std::move(layout);
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
    for (auto& wnd : layout_children)
        AttachChild(wnd);
}

std::shared_ptr<Layout> Wnd::DetachLayout()
{
    auto layout{GetLayout()};
    DetachChild(layout.get());
    return layout;
}

void Wnd::SetLayoutBorderMargin(unsigned int margin)
{
    if (auto layout{GetLayout()})
        layout->SetBorderMargin(margin);
}

void Wnd::SetLayoutCellMargin(unsigned int margin)
{
    if (auto layout{GetLayout()})
        layout->SetCellMargin(margin);
}

void Wnd::PreRender()
{
    m_needs_prerender = false;
    auto layout{GetLayout()};
    if (!layout)
        return;
    if (layout->m_needs_prerender)
        layout->PreRender();
}

bool Wnd::Run()
{
    if ((m_flags & MODAL) && !Parent()) {
        GUI* gui = GUI::GetGUI();
        gui->RegisterModal(shared_from_this());
        ModalInit();
        m_modal_done.store(false);
        gui->RunModal(shared_from_this());
        gui->Remove(shared_from_this());
        return true;
    }
    return false;
}

void Wnd::SetBrowseModeTime(unsigned int time, std::size_t mode)
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

void Wnd::SetBrowseInfoWnd(std::shared_ptr<BrowseInfoWnd> wnd, std::size_t mode)
{ m_browse_modes.at(mode).wnd = std::move(wnd); }

void Wnd::ClearBrowseInfoWnd(std::size_t mode)
{ m_browse_modes.at(mode).wnd.reset(); }

void Wnd::SetBrowseText(std::string text, std::size_t mode)
{ m_browse_modes.at(mode).text = std::move(text); }

Wnd::DragDropRenderingState Wnd::GetDragDropRenderingState() const
{
    DragDropRenderingState retval = DragDropRenderingState::NOT_DRAGGED;
    if (GUI::GetGUI()->DragDropWnd(this)) {
        if (!Dragable() && !GUI::GetGUI()->RenderingDragDropWnds())
             retval = DragDropRenderingState::IN_PLACE_COPY;
        else if (GUI::GetGUI()->AcceptedDragDropWnd(this))
            retval = DragDropRenderingState::DRAGGED_OVER_ACCEPTING_DROP_TARGET;
        else
            retval = DragDropRenderingState::DRAGGED_OVER_UNACCEPTING_DROP_TARGET;
    }
    return retval;
}

void Wnd::LButtonDown(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys)
{
    if (Dragable())
        OffsetMove(move);
    else if (!Interactive())
        ForwardEventToParent();
}

void Wnd::LButtonUp(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::LClick(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::LDoubleClick(Pt pt, Flags<ModKey> mod_keys)
{ LClick(pt, mod_keys); }

void Wnd::MButtonDown(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MDrag(Pt pt, Pt move, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MButtonUp(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MClick(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MDoubleClick(Pt pt, Flags<ModKey> mod_keys)
{ MClick(pt, mod_keys); }

void Wnd::RButtonDown(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RDrag(Pt pt, Pt move, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RButtonUp(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RClick(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::RDoubleClick(Pt pt, Flags<ModKey> mod_keys)
{ RClick(pt, mod_keys); }

void Wnd::MouseEnter(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MouseHere(Pt pt, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MouseLeave()
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::DragDropEnter(Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::DragDropHere(Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys)
{
    if (!Interactive())
        ForwardEventToParent();
    this->DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);
}

void Wnd::CheckDrops(Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                     Flags<ModKey> mod_keys)
{
    if (!Interactive())
        ForwardEventToParent();
    this->DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);
}

void Wnd::DragDropLeave()
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::KeyRelease(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{ if (!Interactive()) ForwardEventToParent(); }

void Wnd::TextInput(const std::string&)
{ if (!Interactive()) ForwardEventToParent(); }

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
        case WndEvent::EventType::LButtonDown:
            LButtonDown(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::LDrag:
            LDrag(event.Point(), event.DragMove(), event.ModKeys());
            break;
        case WndEvent::EventType::LButtonUp:
            LButtonUp(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::LClick:
            LClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::LDoubleClick:
            LDoubleClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::MButtonDown:
            MButtonDown(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::MDrag:
            MDrag(event.Point(), event.DragMove(), event.ModKeys());
            break;
        case WndEvent::EventType::MButtonUp:
            MButtonUp(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::MClick:
            MClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::MDoubleClick:
            MDoubleClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::RButtonDown:
            RButtonDown(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::RDrag:
            RDrag(event.Point(), event.DragMove(), event.ModKeys());
            break;
        case WndEvent::EventType::RButtonUp:
            RButtonUp(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::RClick:
            RClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::RDoubleClick:
            RDoubleClick(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::MouseEnter:
            MouseEnter(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::MouseHere:
            MouseHere(event.Point(), event.ModKeys());
            break;
        case WndEvent::EventType::MouseLeave:
            MouseLeave();
            break;
        case WndEvent::EventType::DragDropEnter:
            DragDropEnter(event.Point(), event.GetAcceptableDropWnds(), event.ModKeys());
            break;
        case WndEvent::EventType::DragDropHere:
            DragDropHere(event.Point(), event.GetAcceptableDropWnds(), event.ModKeys());
            break;
        case WndEvent::EventType::CheckDrops:
            CheckDrops(event.Point(), event.GetAcceptableDropWnds(), event.ModKeys());
            break;
        case WndEvent::EventType::DragDropLeave:
            DragDropLeave();
            break;
        case WndEvent::EventType::DragDroppedOn:
            AcceptDrops(event.Point(), event.GetDragDropWnds(), event.ModKeys());
            break;
        case WndEvent::EventType::MouseWheel:
            MouseWheel(event.Point(), event.WheelMove(), event.ModKeys());
            break;
        case WndEvent::EventType::KeyPress:
            KeyPress(event.GetKey(), event.KeyCodePoint(), event.ModKeys());
            break;
        case WndEvent::EventType::KeyRelease:
            KeyRelease(event.GetKey(), event.KeyCodePoint(), event.ModKeys());
            break;
        case WndEvent::EventType::TextInput:
            TextInput(event.GetText());
            break;
        case WndEvent::EventType::GainingFocus:
            GainingFocus();
            break;
        case WndEvent::EventType::LosingFocus:
            LosingFocus();
            break;
        case WndEvent::EventType::TimerFiring:
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
    if (m_child_clipping_mode != ChildClippingMode::DontClip)
        BeginClippingImpl(m_child_clipping_mode);
}

void Wnd::EndClipping()
{
    if (m_child_clipping_mode != ChildClippingMode::DontClip)
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
    case ChildClippingMode::DontClip:
        assert(!"Wnd::BeginClippingImpl() called with mode == DontClip; this should never happen.");
        break;
    case ChildClippingMode::ClipToClient:
    case ChildClippingMode::ClipToClientAndWindowSeparately:
        BeginScissorClipping(ClientUpperLeft(), ClientLowerRight());
        break;
    case ChildClippingMode::ClipToWindow:
        BeginScissorClipping(UpperLeft(), LowerRight());
        break;
    case Wnd::ChildClippingMode::ClipToAncestorClient:
        break;
    }
}

void Wnd::EndClippingImpl(ChildClippingMode mode)
{
    switch (mode) {
    case ChildClippingMode::DontClip:
        assert(!"Wnd::EndClippingImpl() called with mode == DontClip; this should never happen.");
        break;
    case ChildClippingMode::ClipToClient:
    case ChildClippingMode::ClipToWindow:
    case ChildClippingMode::ClipToClientAndWindowSeparately:
        EndScissorClipping();
        break;
    case Wnd::ChildClippingMode::ClipToAncestorClient:
        break;
    }
}

void Wnd::BeginNonclientClippingImpl()
{ BeginStencilClipping(ClientUpperLeft(), ClientLowerRight(), UpperLeft(), LowerRight()); }

void Wnd::EndNonclientClippingImpl()
{ EndStencilClipping(); }

