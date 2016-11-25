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

#include <GG/DropDownList.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/Scroll.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>

#include <boost/optional/optional.hpp>

using namespace GG;

class ModalListPicker : public Control
{
public:
    typedef ListBox::iterator iterator;
    typedef boost::signals2::signal<void (iterator)>   SelChangedSignalType;

    ModalListPicker(Clr color, const Wnd* relative_to_wnd, size_t m_num_shown_rows);

    virtual bool   Run();
    virtual void   EndRun();
    bool           Dropped() const;
    virtual void   Render() {};

    /** Adjust the m_lb_wnd size so that there are no more than m_num_shown_rows shown. It will
        not adjust a visible window, or if there is no relative to window. */
    void CorrectListSize();

    virtual void LClick(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void ModalInit();

    ListBox* LB()
    { return m_lb_wnd; }

    const ListBox* LB() const
    { return m_lb_wnd; }

    /** The selection change signal while not running the modal drop down box.*/
    mutable SelChangedSignalType SelChangedSignal;
    /** The selection change signal while running the modal drop down box.*/
    mutable SelChangedSignalType SelChangedWhileDroppedSignal;

    DropDownList::iterator CurrentItem();

    /** If \p it is not none then select \p it in the LB().  Return the newly selected iterator or none if
        the selection did not change.*/
    boost::optional<DropDownList::iterator> Select(boost::optional<DropDownList::iterator> it);

    /** Call SelChangedSignal if \p it is not none. */
    void SignalChanged(boost::optional<DropDownList::iterator> it);

    /** A common KeyPress() for both ModalListPicker and its DropDownList.
        Examine \p key and return the new list iterator or none.*/
    boost::optional<DropDownList::iterator> KeyPressCommon(
        Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);

    /** A common MouseWheel() for both ModalListPicker and its DropDownList.
        Examine \p pt and \p move and then return the new list iterator or none.*/
    boost::optional<DropDownList::iterator> MouseWheelCommon(
        const Pt& pt, int move, Flags<ModKey> mod_keys);

protected:
    /** ModalListPicker needs to process its own key press events because modal windows in GG
        can't have parents. */
    virtual void    KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);
    /** ModalListPicker needs to process its own mouse events because modal windows in GG can't
        have parents.*/
    virtual void    MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys);

private:
    void LBSelChangedSlot(const ListBox::SelectionSet& rows);

    void LBLeftClickSlot(ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);

    ListBox*     m_lb_wnd;
    const size_t m_num_shown_rows;
    const Wnd*   m_relative_to_wnd;
    bool         m_dropped;  ///< Is the drop down list open.
};

namespace {
    struct DropDownListSelChangedEcho
    {
        DropDownListSelChangedEcho(const DropDownList& drop_list) :
            m_drop_list(drop_list)
        {}
        void operator()(const DropDownList::iterator& it)
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
        void operator()(const ListBox::iterator& it)
        {
            std::cerr << "GG SIGNAL : ModalListPicker::SelChangedSignal(row="
                      << std::distance(m_picker.LB()->begin(), it)
                      << ")" << std::endl;
        }
        ModalListPicker& m_picker;
    };

    const int BORDER_THICK = 2; // should be the same as the BORDER_THICK value in GGListBox.h
}

////////////////////////////////////////////////
// ModalListPicker
////////////////////////////////////////////////
ModalListPicker::ModalListPicker(Clr color, const Wnd* relative_to_wnd, size_t num_rows) :
    Control(X0, Y0, GUI::GetGUI()->AppWidth(), GUI::GetGUI()->AppHeight(), INTERACTIVE | MODAL),
    m_lb_wnd(GetStyleFactory()->NewDropDownListListBox(color, color)),
    m_num_shown_rows(num_rows),
    m_relative_to_wnd(relative_to_wnd),
    m_dropped(false)
{
    Connect(m_lb_wnd->SelChangedSignal,     &ModalListPicker::LBSelChangedSlot, this);
    Connect(m_lb_wnd->LeftClickedSignal,    &ModalListPicker::LBLeftClickSlot,  this);
    AttachChild(m_lb_wnd);

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(SelChangedSignal, ModalListPickerSelChangedEcho(*this));

    if (m_relative_to_wnd)
        m_lb_wnd->MoveTo(Pt(m_relative_to_wnd->Left(), m_relative_to_wnd->Bottom()));

    m_lb_wnd->Hide();
}


bool ModalListPicker::Run() {
    DropDownList::iterator old_current_item = CurrentItem();
    bool retval = Wnd::Run();
    m_dropped = false;
    if (old_current_item != CurrentItem())
        SignalChanged(CurrentItem());
    return retval;
}

void ModalListPicker::ModalInit()
{
    m_dropped = true;
    m_lb_wnd->Hide(); // to enable CorrectListSize() to work
    CorrectListSize();
    Show();
}

void ModalListPicker::EndRun() {
    Wnd::EndRun();
    m_lb_wnd->Hide();
}

bool ModalListPicker::Dropped() const
{ return m_dropped; }

DropDownList::iterator ModalListPicker::CurrentItem()
{ return (LB()->Selections().empty()) ?  LB()->end() : (*LB()->Selections().begin()); }

boost::optional<DropDownList::iterator>  ModalListPicker::Select(boost::optional<DropDownList::iterator> it)
{
    if (!it)
        return boost::none;

    DropDownList::iterator old_m_current_item = CurrentItem();
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

    if (Dropped())
        SelChangedWhileDroppedSignal(*it);
    else
        SelChangedSignal(*it);
}

void ModalListPicker::CorrectListSize() {
    // reset size of displayed drop list based on number of shown rows set.
    // assumes that all rows have the same height.
    // adds some magic padding for now to prevent the scroll bars showing up.

    if (!m_relative_to_wnd)
        return;

    if (LB()->Visible())
        return;

    LB()->MoveTo(Pt(m_relative_to_wnd->Left(), m_relative_to_wnd->Bottom()));

    Pt drop_down_size(m_relative_to_wnd->ClientWidth(), m_relative_to_wnd->ClientHeight());

    if (LB()->Empty()) {
        LB()->Resize(drop_down_size);
    } else {
        LB()->Show();

        // Resize the rows, once to pick up the correct height and a second
        // time to use the height to size the drop down list
        drop_down_size.y = (*LB()->FirstRowShown())->Height() * std::min<int>(m_num_shown_rows, LB()->NumRows()) + 4;
        LB()->Resize(drop_down_size);
        GUI::GetGUI()->PreRenderWindow(LB());

        drop_down_size.y = (*LB()->FirstRowShown())->Height() * std::min<int>(m_num_shown_rows, LB()->NumRows()) + 4;
        LB()->Resize(drop_down_size);
        GUI::GetGUI()->PreRenderWindow(LB());

        LB()->Hide();
    }
}

boost::optional<DropDownList::iterator> ModalListPicker::KeyPressCommon(
    Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    switch (key) {
    case GGK_UP: // arrow-up (not numpad arrow)
        if (CurrentItem() != LB()->end() && CurrentItem() != LB()->begin())
            return boost::prior(CurrentItem());
        break;
    case GGK_DOWN: // arrow-down (not numpad arrow)
        if (CurrentItem() != LB()->end() && CurrentItem() != --LB()->end())
            return boost::next(CurrentItem());
        break;
    case GGK_PAGEUP: // page up key (not numpad key)
        if (LB()->NumRows() && CurrentItem() != LB()->end()) {
            std::size_t i = 10;
            DropDownList::iterator it = CurrentItem();
            while (i && it != LB()->begin()) {
                --it;
                --i;
            }
            return it;
        }
        break;
    case GGK_PAGEDOWN: // page down key (not numpad key)
        if (LB()->NumRows()) {
            std::size_t i = 10;
            DropDownList::iterator it = CurrentItem();
            while (i && it != --LB()->end()) {
                ++it;
                ++i;
            }
            return it;
        }
        break;
    case GGK_HOME: // home key (not numpad)
        if (LB()->NumRows())
            return LB()->begin();
        break;
    case GGK_END: // end key (not numpad)
        if (LB()->NumRows() && !LB()->Empty())
            return --LB()->end();
        break;
    case GGK_RETURN:
    case GGK_KP_ENTER:
    case GGK_ESCAPE:
        EndRun();
        return boost::none;
        break;
    default:
        return boost::none;
    }
    return boost::none;
}

boost::optional<DropDownList::iterator> ModalListPicker::MouseWheelCommon(
    const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    DropDownList::iterator cur_it = CurrentItem();
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
        return cur_it;
    }
    return boost::none;
}

void ModalListPicker::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ EndRun(); }

void ModalListPicker::LBSelChangedSlot(const ListBox::SelectionSet& rows)
{
    if (rows.empty()) {
        SignalChanged(m_lb_wnd->end());
    } else {
        ListBox::iterator sel_it = *rows.begin();
        SignalChanged(sel_it);
    }
}

void ModalListPicker::LBLeftClickSlot(ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
{ EndRun(); }

void ModalListPicker::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    SignalChanged(Select(KeyPressCommon(key, key_code_point, mod_keys)));
}

void ModalListPicker::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    SignalChanged(Select(MouseWheelCommon(pt, move, mod_keys)));
}

////////////////////////////////////////////////
// GG::DropDownList
////////////////////////////////////////////////
DropDownList::DropDownList(size_t num_shown_elements, Clr color) :
    Control(X0, Y0, X(1 + 2 * BORDER_THICK), Y(1 + 2 * BORDER_THICK), INTERACTIVE),
    m_modal_picker(new ModalListPicker(color, this, num_shown_elements))
{
    SetStyle(LIST_SINGLESEL);

    Connect(m_modal_picker->SelChangedSignal, SelChangedSignal);
    Connect(m_modal_picker->SelChangedWhileDroppedSignal, SelChangedWhileDroppedSignal);

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(SelChangedSignal, DropDownListSelChangedEcho(*this));

    // InitBuffer here prevents a crash if DropDownList is constructed in
    // the prerender phase.
    InitBuffer();

    // Set a non zero client min size.
    SetMinSize(Pt(X(1 + 2 * BORDER_THICK), Y(1 + 2 * BORDER_THICK)));

    RequirePreRender();
}

DropDownList::~DropDownList() {
    if (m_modal_picker)
        m_modal_picker->EndRun();

    m_buffer.clear();
}

DropDownList::iterator DropDownList::CurrentItem() const
{ return m_modal_picker->CurrentItem(); }

std::size_t DropDownList::CurrentItemIndex() const
{ return IteratorToIndex(CurrentItem()); }

std::size_t DropDownList::IteratorToIndex(iterator it) const
{ return it == m_modal_picker->LB()->end() ? -1 : std::distance(m_modal_picker->LB()->begin(), it); }

DropDownList::iterator DropDownList::IndexToIterator(std::size_t n) const
{ return n < LB()->NumRows() ? boost::next(m_modal_picker->LB()->begin(), n) : m_modal_picker->LB()->end(); }

bool DropDownList::Empty() const
{ return LB()->Empty(); }

DropDownList::const_iterator DropDownList::begin() const
{ return LB()->begin(); }

DropDownList::const_iterator DropDownList::end() const
{ return LB()->end(); }

const DropDownList::Row& DropDownList::GetRow(std::size_t n) const
{ return LB()->GetRow(n); }

bool DropDownList::Selected(iterator it) const
{ return LB()->Selected(it); }

bool DropDownList::Selected(std::size_t n) const
{ return n < LB()->NumRows() ? LB()->Selected(boost::next(m_modal_picker->LB()->begin(), n)) : false; }

Clr DropDownList::InteriorColor() const
{ return LB()->InteriorColor(); }

bool DropDownList::Dropped() const
{ return m_modal_picker->Dropped(); }

Y DropDownList::DropHeight() const
{ return LB()->Height(); }

Flags<ListBoxStyle> DropDownList::Style() const
{ return LB()->Style(); }

std::size_t DropDownList::NumRows() const
{ return LB()->NumRows(); }

std::size_t DropDownList::NumCols() const
{ return LB()->NumCols(); }

std::size_t DropDownList::SortCol() const
{ return LB()->SortCol(); }

X DropDownList::ColWidth(std::size_t n) const
{ return LB()->ColWidth(n); }

Alignment DropDownList::ColAlignment(std::size_t n) const
{ return LB()->ColAlignment(n); }

Alignment DropDownList::RowAlignment(iterator it) const
{ return LB()->RowAlignment(it); }

Pt DropDownList::ClientUpperLeft() const
{ return UpperLeft() + Pt(X(BORDER_THICK), Y(BORDER_THICK)); }

Pt DropDownList::ClientLowerRight() const
{ return LowerRight() - Pt(X(BORDER_THICK), Y(BORDER_THICK)); }

void DropDownList::InitBuffer()
{
    m_buffer.clear();

    GG::Pt lr = Size();
    GG::Pt inner_ul = GG::Pt(GG::X(BORDER_THICK), GG::Y(BORDER_THICK));
    GG::Pt inner_lr = lr - inner_ul;

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
    Clr border_color1 = DarkColor(border_color);
    Clr border_color2 = LightColor(border_color);
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
    if (BORDER_THICK && (border_color1 != CLR_ZERO || border_color2 != CLR_ZERO)) {
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

GG::X DropDownList::DisplayedRowWidth() const
{ return ClientWidth(); }

void DropDownList::RenderDisplayedRow()
{
    // Draw the ListBox::Row of currently displayed item, if any.
    if (CurrentItem() == LB()->end())
        return;

    /** The following code possibly renders the selected row twice.  Once in the selected area and
        also in the drop down list if it is visible.*/
    Row* current_item = *CurrentItem();
    bool sel_visible = current_item->Visible();
    bool lb_visible = LB()->Visible();

    // The following is necessary because neither LB() nor the selected row may be visible and
    // prerendered.
    if (!lb_visible)
        LB()->Show();
    GUI::GetGUI()->PreRenderWindow(LB());
    if (!lb_visible)
        LB()->Hide();

    if (!sel_visible)
        current_item->Show();

    // Vertically center the selected row in the box.
    Pt offset = GG::Pt(ClientUpperLeft().x - current_item->ClientUpperLeft().x,
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

void DropDownList::SizeMove(const Pt& ul, const Pt& lr)
{
    // adjust size to keep correct height based on row height, etc.
    GG::Pt old_ul = RelativeUpperLeft();
    GG::Pt old_lr = RelativeLowerRight();

    Wnd::SizeMove(ul, lr);

    if ((old_ul != RelativeUpperLeft()) || (old_lr != RelativeLowerRight()))
        RequirePreRender();
}

void DropDownList::SetColor(Clr c)
{ LB()->SetColor(c); }

DropDownList::iterator DropDownList::Insert(Row* row, iterator it, bool signal/* = true*/)
{
    row->SetDragDropDataType("");
    DropDownList::iterator ret = LB()->Insert(row, it, signal);
    Resize(Size());
    RequirePreRender();
    return ret;
}

DropDownList::iterator DropDownList::Insert(Row* row, bool signal/* = true*/)
{
    row->SetDragDropDataType("");
    DropDownList::iterator ret = LB()->Insert(row, signal);
    Resize(Size());
    RequirePreRender();
    return ret;
}

void DropDownList::Insert(const std::vector<Row*>& rows, iterator it, bool signal/* = true*/)
{
    for (std::vector<Row*>::const_iterator rows_it = rows.begin();
         rows_it != rows.end(); ++rows_it)
    { (*rows_it)->SetDragDropDataType(""); }
    LB()->Insert(rows, it, signal);
    Resize(Size());
    RequirePreRender();
}

void DropDownList::Insert(const std::vector<Row*>& rows, bool signal/* = true*/)
{
    for (std::vector<Row*>::const_iterator rows_it = rows.begin();
         rows_it != rows.end(); ++rows_it)
    { (*rows_it)->SetDragDropDataType(""); }
    LB()->Insert(rows, signal);
    Resize(Size());
    RequirePreRender();
}

DropDownList::Row* DropDownList::Erase(iterator it, bool signal/* = false*/)
{ return LB()->Erase(it, signal); }

void DropDownList::Clear()
{
    m_modal_picker->EndRun();
    LB()->Clear();
    RequirePreRender();
}

DropDownList::iterator DropDownList::begin()
{ return LB()->begin(); }

DropDownList::iterator DropDownList::end()
{ return LB()->end(); }

DropDownList::Row& DropDownList::GetRow(std::size_t n)
{ return LB()->GetRow(n); }

void DropDownList::Select(iterator it)
{ m_modal_picker->Select(it); }

void DropDownList::Select(std::size_t n)
{ m_modal_picker->Select(n < LB()->NumRows() ? boost::next(LB()->begin(), n) : LB()->end()); }

void DropDownList::SetInteriorColor(Clr c)
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

void DropDownList::NormalizeRowsOnInsert(bool enable /*= true*/)
{ LB()->NormalizeRowsOnInsert(enable); }

void DropDownList::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    const ListBox::SelectionSet& LB_sels = LB()->Selections();
    if (!LB_sels.empty()) {
        if (LB()->m_vscroll) {
            LB()->m_vscroll->ScrollTo(0);
            SignalScroll(*LB()->m_vscroll, true);
        }
    }
    LB()->m_first_col_shown = 0;

    DropDownOpenedSignal(true);
    m_modal_picker->Run();
    DropDownOpenedSignal(false);
}

void DropDownList::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
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

void DropDownList::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_modal_picker->SignalChanged(m_modal_picker->Select(m_modal_picker->MouseWheelCommon(pt, move, mod_keys)));
    } else {
        Control::MouseWheel(pt, move, mod_keys);
    }
}

ListBox* DropDownList::LB()
{ return m_modal_picker->LB(); }

const ListBox* DropDownList::LB() const
{ return m_modal_picker->LB(); }
