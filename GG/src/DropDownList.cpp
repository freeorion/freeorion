//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <iterator>
#include <memory>
#include <boost/optional/optional.hpp>
#include <GG/DrawUtil.h>
#include <GG/DropDownList.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Scroll.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

class ModalListPicker : public Control
{
public:
    typedef ListBox::iterator iterator;
    typedef boost::signals2::signal<void (iterator)> SelChangedSignalType;

    ModalListPicker(Clr color, const DropDownList* relative_to_wnd, std::size_t m_num_shown_rows);
    void CompleteConstruction() override;
    ~ModalListPicker();

    /** ModalListPicker is run then it returns true if it was not destroyed while running.*/
    bool RunAndCheckSelfDestruction();
    void EndRun() override;
    void Render() noexcept override {}

    [[nodiscard]] bool Dropped() const noexcept { return m_dropped.load(); }

    /** Adjust the m_lb_wnd size so that there are no more than m_num_shown_rows shown. It will
        not adjust a visible window, or if there is no relative to window. */
    void CorrectListSize();

    void LClick(Pt pt, Flags<ModKey> mod_keys) override;
    void ModalInit() override;

    [[nodiscard]] auto* LB() noexcept { return m_lb_wnd.get(); }
    [[nodiscard]] const auto* LB() const noexcept { return m_lb_wnd.get(); }

    /** The selection change signal while not running the modal drop down box.*/
    mutable SelChangedSignalType SelChangedSignal;
    /** The selection change signal while running the modal drop down box.*/
    mutable SelChangedSignalType SelChangedWhileDroppedSignal;

    [[nodiscard]] DropDownList::iterator CurrentItem() noexcept;

    /** If \p it is not none then select \p it in the LB().  Return the newly selected iterator or none if
        the selection did not change.*/
    boost::optional<DropDownList::iterator> Select(boost::optional<DropDownList::iterator> it);

    /** Call SelChangedSignal if \p it is not none. */
    void SignalChanged(boost::optional<DropDownList::iterator> it);

    /** A common KeyPress() for both ModalListPicker and its DropDownList.
        Examine \p key and return the new list iterator or none.*/
    [[nodiscard]] boost::optional<DropDownList::iterator> KeyPressCommon(
        Key key, uint32_t key_code_point, Flags<ModKey> mod_keys);

    /** A common MouseWheel() for both ModalListPicker and its DropDownList.
        Examine \p pt and \p move and then return the new list iterator or none.*/
    [[nodiscard]] boost::optional<DropDownList::iterator> MouseWheelCommon(
        Pt pt, int move, Flags<ModKey> mod_keys);

    /** Set the drop down list to only mouse scroll if it is dropped. */
    void SetOnlyMouseScrollWhenDropped(bool enable);

protected:
    /** ModalListPicker needs to process its own key press events because modal
        windows in GG can't have parents. */
    void KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys) override;

    /** ModalListPicker needs to process its own mouse events because modal windows in GG can't
        have parents.*/
    void MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys) override;

    /** Force the m_lb_wnd mouse wheel events to be forwarded. */
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;

private:
    void LBSelChangedSlot(ListBox::SelectionSet rows);

    void LBLeftClickSlot(ListBox::iterator it, GG::Pt pt, Flags<ModKey> modkeys);

    /** Close the drop down if the app resizes, to prevent the modal drop down
        list location not tracking the anchor.*/
    void WindowResizedSlot(X x, Y y);

    /** Used by CorrectListSize() to determine the list height from the current first shown row. */
    Pt DetermineListHeight(Pt drop_down_size);

    std::shared_ptr<ListBox> m_lb_wnd;
    const std::size_t        m_num_shown_rows = 1u;
    const DropDownList*      m_relative_to_wnd = nullptr;

    boost::signals2::scoped_connection m_lclick_connection;
    boost::signals2::scoped_connection m_sel_change_connection;
    boost::signals2::scoped_connection m_resize_connection;

    std::atomic<bool>        m_dropped = false; ///< Is the drop down list open.

    /** Should the list wnd scroll only when dropped? */
    bool                     m_only_mouse_scroll_when_dropped = false;
};


namespace {

struct DropDownListSelChangedEcho
{
    DropDownListSelChangedEcho(const DropDownList& drop_list) :
        m_drop_list(drop_list)
    {}
    void operator()(const DropDownList::iterator it)
    {
        std::cerr << "GG SIGNAL : DropDownList::SelChangedSignal(row="
                  << m_drop_list.IteratorToIndex(it)
                  << ")" << std::endl;
    }
    const DropDownList& m_drop_list;
};

struct ModalListPickerSelChangedEcho
{
    ModalListPickerSelChangedEcho(ModalListPicker& picker) :
        m_picker(picker)
    {}
    void operator()(const ListBox::iterator it)
    {
        std::cerr << "GG SIGNAL : ModalListPicker::SelChangedSignal(row="
                  << std::distance(m_picker.LB()->begin(), it)
                  << ")" << std::endl;
    }
    ModalListPicker& m_picker;
};

}

////////////////////////////////////////////////
// ModalListPicker
////////////////////////////////////////////////
ModalListPicker::ModalListPicker(Clr color, const DropDownList* relative_to_wnd, std::size_t num_rows) :
    Control(X0, Y0, GUI::GetGUI()->AppWidth(), GUI::GetGUI()->AppHeight(), INTERACTIVE | MODAL),
    m_lb_wnd(GetStyleFactory().NewDropDownListListBox(color, color)),
    m_num_shown_rows(std::max(std::size_t{1u}, num_rows)),
    m_relative_to_wnd(relative_to_wnd)
{}

void ModalListPicker::CompleteConstruction()
{
    namespace ph = boost::placeholders;

    m_lclick_connection = m_lb_wnd->SelRowsChangedSignal.connect(
        boost::bind(&ModalListPicker::LBSelChangedSlot, this, ph::_1));
    m_sel_change_connection = m_lb_wnd->LeftClickedRowSignal.connect(
        boost::bind(&ModalListPicker::LBLeftClickSlot, this, ph::_1, ph::_2, ph::_3));

    m_resize_connection = GUI::GetGUI()->WindowResizedSignal.connect(
        boost::bind(&ModalListPicker::WindowResizedSlot, this, ph::_1, ph::_2));

    AttachChild(m_lb_wnd);
    m_lb_wnd->InstallEventFilter(shared_from_this());

    if (INSTRUMENT_ALL_SIGNALS)
        SelChangedSignal.connect(ModalListPickerSelChangedEcho(*this));

    if (m_relative_to_wnd)
        m_lb_wnd->MoveTo(Pt(m_relative_to_wnd->Left(), m_relative_to_wnd->Bottom()));

    m_lb_wnd->Hide();
}

ModalListPicker::~ModalListPicker()
{ ModalListPicker::EndRun(); }

bool ModalListPicker::RunAndCheckSelfDestruction()
{
    //    const std::shared_ptr<ModalListPicker> leash_holder_prevents_destruction = leash;
    const auto keep_alive = shared_from_this();
    const auto old_current_item = CurrentItem();
    Wnd::Run();
    const auto new_current_item = CurrentItem();
    m_dropped.store(false);
    if (keep_alive.use_count() < 2)
        return false;
    if (old_current_item != new_current_item)
        SignalChanged(new_current_item);
    return true;
}

void ModalListPicker::ModalInit()
{
    m_modal_done.store(true);
    m_dropped.store(true);

    // Try to center the current item unless within half the number of
    // shown rows from the top or bottom
    const auto current_item = CurrentItem();
    if (current_item != m_lb_wnd->end() && !m_lb_wnd->Empty()) {
        const std::size_t current_ii(std::distance(m_lb_wnd->begin(), current_item));
        const std::size_t half_shown((m_num_shown_rows / 2));
        const std::size_t even_extra_one((m_num_shown_rows % 2 == 0) ? 1 : 0);

        m_lb_wnd->SetFirstRowShown(m_lb_wnd->begin());
        if (current_ii >= (m_lb_wnd->NumRows() - 1 - half_shown)) {
            m_lb_wnd->BringRowIntoView(--m_lb_wnd->end());
        } else if (current_ii >= half_shown) {
            m_lb_wnd->SetFirstRowShown(
                std::next(m_lb_wnd->begin(),
                          current_ii - half_shown + even_extra_one));
        }
    }

    m_lb_wnd->Hide(); // to enable CorrectListSize() to work
    CorrectListSize();
    Show();
}

void ModalListPicker::EndRun()
{
    m_modal_done.store(true);
    m_dropped.store(false);
    m_lb_wnd->Hide();
    Hide();
}

void ModalListPicker::WindowResizedSlot(X x, Y y)
{
    // Keep the ModalListPicker full app sized so that the drop down list
    // can be placed anywhere.
    Control::Resize(Pt(x, y));
    if (m_dropped.load())
        EndRun();
}

DropDownList::iterator ModalListPicker::CurrentItem() noexcept
{
    const auto start = m_lb_wnd->begin(), end = m_lb_wnd->end();
    if (start == end)
        return end;
    if (m_lb_wnd->Selections().empty())
        return end;
    const auto sel_it{*m_lb_wnd->Selections().begin()};
    for (auto find_it = start; find_it != end; ++find_it)
        if (find_it == sel_it)
            return find_it;
    return end;
}

boost::optional<DropDownList::iterator> ModalListPicker::Select(boost::optional<DropDownList::iterator> it)
{
    if (!it)
        return boost::none;

    auto old_m_current_item = CurrentItem();
    if (*it == LB()->end()) {
        LB()->DeselectAll();
    } else {
        LB()->SelectRow(*it);
    }

    return (CurrentItem() != old_m_current_item) ? boost::optional<DropDownList::iterator>(CurrentItem()) : boost::none;
}

void ModalListPicker::SignalChanged(boost::optional<DropDownList::iterator> it)
{
    if (!it)
        return;

    const auto weak_this(weak_from_this());

    if (m_dropped.load()) {
        // There will be at least 2 shared_ptr, one held by parent and one by Run(), if the parent
        // has not already destroyed itself.  This can happen while in the modal event pump.
        if (weak_this.use_count() < 2)
            return;

        SelChangedWhileDroppedSignal(*it);
    } else {
        // There will be at least 1 shared_ptr, one held by parent and one by Run(), if the parent
        // has not already tried to destroy this.  This should never happen.
        if (weak_this.use_count() < 1)
            return;

        SelChangedSignal(*it);
    }
}

Pt ModalListPicker::DetermineListHeight(Pt drop_down_size) {
    auto* const lb = LB();
    if (!lb)
        return drop_down_size;

    // Determine the expected height
    auto border_thick = 2 * GG::Y(ListBox::BORDER_THICK);
    auto num_rows = std::min<int>(m_num_shown_rows, lb->NumRows());

    const auto first_shown_row_it = lb->FirstRowShown();
    auto row_height = (*first_shown_row_it)->Height();
    auto expected_height = num_rows * row_height + border_thick;

    const auto* const gui = GUI::GetGUI();

    // Shrink the height if too near app edge.
    const auto dist_to_app_edge = gui->AppHeight() - m_relative_to_wnd->Bottom();
    if (expected_height > dist_to_app_edge && row_height > Y0) {
        auto reduced_num_rows = std::max<int>(1, (dist_to_app_edge-border_thick) / row_height);
        expected_height = reduced_num_rows*row_height + border_thick;
    }

    lb->Resize(GG::Pt{drop_down_size.x, expected_height});

    const auto& sels = lb->Selections();
    if (!sels.empty())
        lb->BringRowIntoView(*(sels.begin()));
    gui->PreRenderWindow(lb);

    return drop_down_size;
}

void ModalListPicker::CorrectListSize() {
    // reset size of displayed drop list based on number of shown rows set.
    // assumes that all rows have the same height.
    // adds some magic padding for now to prevent the scroll bars showing up.

    if (!m_relative_to_wnd)
        return;

    auto* const lb = LB();

    if (!lb || lb->Visible())
        return;

    lb->MoveTo(Pt(m_relative_to_wnd->Left(), m_relative_to_wnd->Bottom()));

    const Pt drop_down_size_initial_guess(m_relative_to_wnd->DroppedRowWidth(),
                                          m_relative_to_wnd->ClientHeight());

    if (lb->Empty()) {
        lb->Resize(drop_down_size_initial_guess);
        return;
    }

    lb->Show();

    // The purpose of this code is to produce a drop down list that
    // will be exactly m_num_shown_rows high and make sure that the
    // selected row is prerendered in the same way when the drop down
    // list is open or closed.

    // The list needs to be resized twice.  The first resize with an
    // estimated row height will add any list box chrome, like scroll
    // bars to the list and may change the height of the row.  The
    // second resize uses the corrected row height to finalize the drop
    // down list size.

    // Note:  Placing a tighter constraint on valid DropDownList rows
    // of always returning the same fixed height regardless of status
    // (width, prerender etc.) would mean this code could be reduced to
    // check height and resize list just once.
    const auto drop_down_size_updated = DetermineListHeight(drop_down_size_initial_guess);
    DetermineListHeight(drop_down_size_updated);

    lb->Hide();
}

boost::optional<DropDownList::iterator> ModalListPicker::KeyPressCommon(
    Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    const bool numlock_on = mod_keys & MOD_KEY_NUM;
    if (!numlock_on) {
        // convert keypad keys into corresponding non-number keys
        switch (key) {
        case Key::GGK_KP0:       key = Key::GGK_INSERT;   break;
        case Key::GGK_KP1:       key = Key::GGK_END;      break;
        case Key::GGK_KP2:       key = Key::GGK_DOWN;     break;
        case Key::GGK_KP3:       key = Key::GGK_PAGEDOWN; break;
        case Key::GGK_KP4:       key = Key::GGK_LEFT;     break;
        case Key::GGK_KP5:                                break;
        case Key::GGK_KP6:       key = Key::GGK_RIGHT;    break;
        case Key::GGK_KP7:       key = Key::GGK_HOME;     break;
        case Key::GGK_KP8:       key = Key::GGK_UP;       break;
        case Key::GGK_KP9:       key = Key::GGK_PAGEUP;   break;
        case Key::GGK_KP_PERIOD: key = Key::GGK_DELETE;   break;
        default:                                          break;
        }
    }

    switch (key) {
    case Key::GGK_UP: // arrow-up (not numpad arrow)
        if (CurrentItem() != LB()->end() && CurrentItem() != LB()->begin()) {
            auto prev_it{std::prev(CurrentItem())};
            LB()->BringRowIntoView(prev_it);
            return prev_it;
        }
        break;
    case Key::GGK_DOWN: // arrow-down (not numpad arrow)
        if (CurrentItem() != LB()->end() && CurrentItem() != --LB()->end()) {
            auto next_it(std::next(CurrentItem()));
            LB()->BringRowIntoView(next_it);
            return next_it;
        }
        break;
    case Key::GGK_PAGEUP: // page up key (not numpad key)
        if (!LB()->Empty() && CurrentItem() != LB()->end()) {
            std::size_t i = std::max(std::size_t{1}, m_num_shown_rows - 1);
            auto it = CurrentItem();
            while (i && it != LB()->begin()) {
                --it;
                --i;
            }
            LB()->BringRowIntoView(it);
            return it;
        }
        break;
    case Key::GGK_PAGEDOWN: // page down key (not numpad key)
        if (!LB()->Empty()) {
            std::size_t i = std::max(std::size_t{1u}, m_num_shown_rows - 1);
            auto it = CurrentItem();
            while (i && it != --LB()->end()) {
                ++it;
                --i;
            }
            LB()->BringRowIntoView(it);
            return it;
        }
        break;
    case Key::GGK_HOME: // home key (not numpad)
        if (!LB()->Empty()) {
            DropDownList::iterator it(LB()->begin());
            LB()->BringRowIntoView(it);
            return it;
        }
        break;
    case Key::GGK_END: // end key (not numpad)
        if (!LB()->Empty()) {
            DropDownList::iterator it(--LB()->end());
            LB()->BringRowIntoView(it);
            return it;
        }
        break;
    case Key::GGK_RETURN:
    case Key::GGK_KP_ENTER:
    case Key::GGK_ESCAPE:
        EndRun();
        return boost::none;
        break;
    default:
        return boost::none;
    }
    return boost::none;
}

void ModalListPicker::SetOnlyMouseScrollWhenDropped(bool enable)
{ m_only_mouse_scroll_when_dropped = enable; }

boost::optional<DropDownList::iterator> ModalListPicker::MouseWheelCommon(
    Pt pt, int move, Flags<ModKey> mod_keys)
{
    if (m_only_mouse_scroll_when_dropped && !Dropped())
        return boost::none;

    auto cur_it = CurrentItem();
    if (cur_it == LB()->end())
        return boost::none;
    if (move == 0)
        return boost::none;

    if (move > 0) {
        int dist_to_last = std::distance(cur_it, LB()->end()) - 1; // end is one past last valid item
        if (move > dist_to_last)
            move = dist_to_last;
    } else {
        int dist_from_first = std::distance(LB()->begin(), cur_it);// begin is the first valid item
        if (-move > dist_from_first)
            move = -dist_from_first;
    }
    if (move != 0) {
        std::advance(cur_it, move);
        LB()->BringRowIntoView(cur_it);
        return cur_it;
    }
    return boost::none;
}

bool ModalListPicker::EventFilter(Wnd* w, const WndEvent& event) {
    if (w != m_lb_wnd.get())
        return false;

    switch (event.Type()) {
    case WndEvent::EventType::MouseWheel:
        MouseWheel(event.Point(), -event.WheelMove(), event.ModKeys());
        return true;
    default:
        break;
    };
    return false;
}

void ModalListPicker::LClick(Pt pt, Flags<ModKey> mod_keys)
{ EndRun(); }

void ModalListPicker::LBSelChangedSlot(ListBox::SelectionSet rows)
{
    if (rows.empty())
        SignalChanged(m_lb_wnd->end());
    else
        SignalChanged(*rows.begin());
}

void ModalListPicker::LBLeftClickSlot(ListBox::iterator it, GG::Pt pt, Flags<ModKey> modkeys)
{ EndRun(); }

void ModalListPicker::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{ SignalChanged(Select(KeyPressCommon(key, key_code_point, mod_keys))); }

void ModalListPicker::MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys)
{
    const bool in_anchor_or_dropped_list = (LB()->InWindow(pt) ||
                                            (m_relative_to_wnd && m_relative_to_wnd->InWindow(pt)));
    if (!in_anchor_or_dropped_list)
        return;

    auto corrected_move = (LB()->InWindow(pt)? move : -move);
    SignalChanged(Select(MouseWheelCommon(pt, corrected_move, mod_keys)));
}


////////////////////////////////////////////////
// GG::DropDownList
////////////////////////////////////////////////
DropDownList::DropDownList(std::size_t num_shown_elements, Clr color) :
    Control(X0, Y0, X(1 + 2 * ListBox::BORDER_THICK),
            Y(1 + 2 * ListBox::BORDER_THICK), INTERACTIVE),
    m_modal_picker(Wnd::Create<ModalListPicker>(color, this, num_shown_elements)),
    m_sel_changed_con(m_modal_picker->SelChangedSignal.connect(SelChangedSignal)),
    m_sel_changed_dropped_con(m_modal_picker->SelChangedWhileDroppedSignal.connect(SelChangedWhileDroppedSignal))
{
    SetStyle(LIST_SINGLESEL);

    if (INSTRUMENT_ALL_SIGNALS)
        SelChangedSignal.connect(DropDownListSelChangedEcho(*this));

    // InitBuffer here prevents a crash if DropDownList is constructed in
    // the prerender phase.
    DropDownList::InitBuffer();

    // Set a non zero client min size.
    SetMinSize(Pt(X(1 + 2 * ListBox::BORDER_THICK), Y(1 + 2 * ListBox::BORDER_THICK)));

    RequirePreRender();
}

DropDownList::~DropDownList()
{ m_modal_picker->ModalListPicker::EndRun(); }

DropDownList::iterator DropDownList::CurrentItem() const noexcept
{ return m_modal_picker->CurrentItem(); }

std::size_t DropDownList::CurrentItemIndex() const noexcept
{ return IteratorToIndex(CurrentItem()); }

std::size_t DropDownList::IteratorToIndex(iterator it) const noexcept
{
    static constexpr std::size_t neg1 = static_cast<std::size_t>(-1);
    const auto* lb = m_modal_picker->LB();
    if (!lb)
        return neg1;
    const auto start = lb->begin(), end = lb->end();
    if (it == end)
        return neg1;
    std::size_t dist = 0;
    for (auto find_it = start; find_it != end; ++find_it) {
        if (find_it == it)
            return dist;
        ++dist;
    }
    return neg1;
}

DropDownList::iterator DropDownList::IndexToIterator(std::size_t n) const
{ return n < LB()->NumRows() ? std::next(m_modal_picker->LB()->begin(), n) : m_modal_picker->LB()->end(); }

const DropDownList::Row& DropDownList::GetRow(std::size_t n) const
{ return LB()->GetRow(n); }

bool DropDownList::Selected(iterator it) const
{ return LB()->Selected(it); }

bool DropDownList::Selected(std::size_t n) const
{ return n < LB()->NumRows() ? LB()->Selected(std::next(m_modal_picker->LB()->begin(), n)) : false; }

Clr DropDownList::InteriorColor() const noexcept
{ return LB()->InteriorColor(); }

bool DropDownList::Dropped() const noexcept
{ return m_modal_picker->Dropped(); }

Y DropDownList::DropHeight() const noexcept
{ return LB()->Height(); }

Flags<ListBoxStyle> DropDownList::Style() const noexcept
{ return LB()->Style(); }

std::size_t DropDownList::NumRows() const noexcept
{ return LB()->NumRows(); }

std::size_t DropDownList::NumCols() const noexcept
{ return LB()->NumCols(); }

std::size_t DropDownList::SortCol() const noexcept
{ return LB()->SortCol(); }

X DropDownList::ColWidth(std::size_t n) const
{ return LB()->ColWidth(n); }

Alignment DropDownList::ColAlignment(std::size_t n) const
{ return LB()->ColAlignment(n); }

Alignment DropDownList::RowAlignment(iterator it) const
{ return LB()->RowAlignment(it); }

void DropDownList::InitBuffer()
{
    m_buffer.clear();

    const auto lr = Size();
    const auto inner_ul = GG::Pt(GG::X(ListBox::BORDER_THICK), GG::Y(ListBox::BORDER_THICK));
    const auto inner_lr = lr - inner_ul;

    // outer border
    m_buffer.store(0.0f,    0.0f);
    m_buffer.store(lr.x,    0.0f);
    m_buffer.store(lr.x,    lr.y);
    m_buffer.store(0.0f,    lr.y);

    // inner bevel quad strip
    m_buffer.store(inner_lr.x,  inner_ul.y);
    m_buffer.store(lr.x,        0.0f);
    m_buffer.store(inner_ul.x,  inner_ul.y);
    m_buffer.store(0.0f,        0.0f);
    m_buffer.store(inner_ul.x,  inner_lr.y);
    m_buffer.store(0.0f,        lr.y);
    m_buffer.store(inner_lr.x,  inner_lr.y);
    m_buffer.store(lr.x,        lr.y);
    m_buffer.store(inner_lr.x,  inner_ul.y);
    m_buffer.store(lr.x,        0.0f);

    m_buffer.createServerBuffer();
}

void DropDownList::PreRender()
{
    GG::Control::PreRender();

    InitBuffer();

    m_modal_picker->CorrectListSize();
}

void DropDownList::Render()
{
    // draw beveled-down rectangle around client area
    Pt ul = UpperLeft();

    Clr border_color = Disabled() ? DisabledColor(LB()->Color()) : LB()->Color();
    Clr border_color1 = DarkenClr(border_color);
    Clr border_color2 = LightenClr(border_color);
    Clr interior_color = Disabled() ? DisabledColor(LB()->m_int_color) : LB()->m_int_color;


    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);


    m_buffer.activate();

    // draw interior of rectangle
    if (interior_color != CLR_ZERO) {
        glColor(interior_color);
        glDrawArrays(GL_TRIANGLE_FAN,   0, 4);
    }

    // draw beveled edges
    if (ListBox::BORDER_THICK && (border_color1 != CLR_ZERO || border_color2 != CLR_ZERO)) {
        // top left shadowed bevel
        glColor(border_color1);
        glDrawArrays(GL_QUAD_STRIP,     4, 6);

        // bottom right brightened bevel
        glColor(border_color2);
        glDrawArrays(GL_QUAD_STRIP,     8, 6);
    }

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);


    RenderDisplayedRow();
}

GG::X DropDownList::DroppedRowWidth() const
{ return ClientWidth(); }

GG::X DropDownList::DisplayedRowWidth() const
{ return ClientWidth(); }

void DropDownList::RenderDisplayedRow()
{
    const auto current_item_it = CurrentItem();
    // Draw the ListBox::Row of currently displayed item, if any.
    if (current_item_it == LB()->end())
        return;
    auto* current_item = current_item_it->get();
    if (!current_item)
        return;

    /** The following code possibly renders the selected row twice.  Once in the selected area and
      * also in the drop down list if it is visible.*/
    const bool sel_visible = current_item->Visible();

    // neither LB() nor the selected row may be visible and prerendered.
    GUI::GetGUI()->PreRenderWindow(LB(), true);

    if (!sel_visible)
        current_item->Show();

    // Vertically center the selected row in the box.
    const Pt offset = GG::Pt(ClientUpperLeft().x - current_item->ClientUpperLeft().x,
                             Top() + Height() / 2 - (current_item->Top() + current_item->Height() / 2));
    current_item->OffsetMove(offset);

    GUI::GetGUI()->PreRenderWindow(current_item);

    BeginClipping();
    GUI::GetGUI()->RenderWindow(current_item);
    EndClipping();

    current_item->OffsetMove(-offset);

    if (!sel_visible)
        current_item->Hide();
}

void DropDownList::SizeMove(Pt ul, Pt lr)
{
    // adjust size to keep correct height based on row height, etc.
    GG::Pt old_ul = RelativeUpperLeft();
    GG::Pt old_lr = RelativeLowerRight();

    Wnd::SizeMove(ul, lr);

    if ((old_ul != RelativeUpperLeft()) || (old_lr != RelativeLowerRight()))
        RequirePreRender();
}

DropDownList::iterator DropDownList::Insert(std::shared_ptr<Row> row, iterator it)
{
    row->SetDragDropDataType("");
    auto ret = LB()->Insert(std::move(row), it);
    Resize(Size());
    RequirePreRender();
    return ret;
}

DropDownList::iterator DropDownList::Insert(std::shared_ptr<Row> row)
{
    row->SetDragDropDataType("");
    auto ret = LB()->Insert(std::move(row));
    Resize(Size());
    RequirePreRender();
    return ret;
}

void DropDownList::Insert(const std::vector<std::shared_ptr<Row>>& rows, iterator it)
{
    for (auto& row : rows)
        row->SetDragDropDataType("");
    LB()->Insert(rows, it);
    Resize(Size());
    RequirePreRender();
}

void DropDownList::Insert(std::vector<std::shared_ptr<Row>>&& rows, iterator it)
{
    for (auto& row : rows)
        row->SetDragDropDataType("");
    LB()->Insert(std::move(rows), it);
    Resize(Size());
    RequirePreRender();
}

void DropDownList::Insert(const std::vector<std::shared_ptr<Row>>& rows)
{
    for (auto& row : rows)
        row->SetDragDropDataType("");
    LB()->Insert(rows);
    Resize(Size());
    RequirePreRender();
}

void DropDownList::Insert(std::vector<std::shared_ptr<Row>>&& rows)
{
    for (auto& row : rows)
        row->SetDragDropDataType("");
    LB()->Insert(std::move(rows));
    Resize(Size());
    RequirePreRender();
}

std::shared_ptr<DropDownList::Row> DropDownList::Erase(iterator it, bool signal)
{ return LB()->Erase(it, signal); }

void DropDownList::Clear()
{
    m_modal_picker->EndRun();
    LB()->Clear();
    RequirePreRender();
}

DropDownList::iterator DropDownList::begin() noexcept
{ return LB()->begin(); }

DropDownList::iterator DropDownList::end() noexcept
{ return LB()->end(); }

DropDownList::Row& DropDownList::GetRow(std::size_t n)
{ return LB()->GetRow(n); }

void DropDownList::Select(iterator it)
{ m_modal_picker->Select(it); }

void DropDownList::Select(std::size_t n)
{ m_modal_picker->Select(n < LB()->NumRows() ? std::next(LB()->begin(), n) : LB()->end()); }

void DropDownList::SetInteriorColor(Clr c) noexcept
{ LB()->SetInteriorColor(c); }

void DropDownList::SetStyle(Flags<ListBoxStyle> s)
{
    s &= ~(LIST_NOSEL | LIST_QUICKSEL | LIST_USERDELETE | LIST_BROWSEUPDATES);
    s |= LIST_SINGLESEL;
    LB()->SetStyle(s);
}

void DropDownList::SetNumCols(std::size_t n)
{ LB()->SetNumCols(n); }

void DropDownList::SetSortCol(std::size_t n)
{ LB()->SetSortCol(n); }

void DropDownList::SetColWidth(std::size_t n, X w)
{ LB()->SetColWidth(n, w); }

void DropDownList::LockColWidths()
{ LB()->LockColWidths(); }

void DropDownList::UnLockColWidths()
{ LB()->UnLockColWidths(); }

void DropDownList::ManuallyManageColProps()
{ LB()->ManuallyManageColProps(); }

void DropDownList::SetColAlignment(std::size_t n, Alignment align) 
{ LB()->SetColAlignment(n, align); }

void DropDownList::SetRowAlignment(iterator it, Alignment align)
{ LB()->SetRowAlignment(it, align); }

void DropDownList::SetColStretch(std::size_t n, double stretch)
{ LB()->SetColStretch(n, stretch); }

void DropDownList::NormalizeRowsOnInsert(bool enable)
{ LB()->NormalizeRowsOnInsert(enable); }

void DropDownList::Close()
{ m_modal_picker->EndRun(); }

void DropDownList::LButtonDown(Pt pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    const auto& LB_sels = LB()->Selections();
    if (!LB_sels.empty()) {
        if (LB()->m_vscroll) {
            LB()->m_vscroll->ScrollTo(0);
            SignalScroll(*LB()->m_vscroll, true);
        }
    }
    LB()->m_first_col_shown = 0;

    DropDownOpenedSignal(true);
    if (!m_modal_picker->RunAndCheckSelfDestruction())
        return;
    DropDownOpenedSignal(false);
}

void DropDownList::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        boost::optional<DropDownList::iterator> key_selected = m_modal_picker->KeyPressCommon(key, key_code_point, mod_keys);
        if (key_selected)
            m_modal_picker->SignalChanged(m_modal_picker->Select(key_selected));
        else
            Control::KeyPress(key, key_code_point, mod_keys);
    } else {
        Control::KeyPress(key, key_code_point, mod_keys);
    }
}

void DropDownList::MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        auto corrected_move = (LB()->InWindow(pt)? move : -move);
        m_modal_picker->SignalChanged(m_modal_picker->Select(
            m_modal_picker->MouseWheelCommon(pt, corrected_move, mod_keys)));
    } else {
        Control::MouseWheel(pt, move, mod_keys);
    }
}

void DropDownList::SetOnlyMouseScrollWhenDropped(bool enable)
{ m_modal_picker->SetOnlyMouseScrollWhenDropped(enable); }

ListBox* DropDownList::LB() noexcept
{ return m_modal_picker->LB(); }

const ListBox* DropDownList::LB() const noexcept
{ return m_modal_picker->LB(); }
