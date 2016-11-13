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


using namespace GG;

class ModalListPicker : public Wnd
{
public:
    typedef ListBox::iterator iterator;
    typedef boost::signals2::signal<void (iterator)>   SelChangedSignalType;

    ModalListPicker(Clr color, const Wnd* relative_to_wnd);

    virtual void LClick(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void ModalInit();

    ListBox* LB()
    { return m_lb_wnd; }

    const ListBox* LB() const
    { return m_lb_wnd; }

    mutable SelChangedSignalType SelChangedSignal; ///< the selection change signal object for this DropDownList

private:
    void LBSelChangedSlot(const ListBox::SelectionSet& rows);

    void LBLeftClickSlot(ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);

    ListBox*    m_lb_wnd;
    const Wnd*  m_relative_to_wnd;
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
ModalListPicker::ModalListPicker(Clr color, const Wnd* relative_to_wnd) :
    Wnd(X0, Y0, GUI::GetGUI()->AppWidth(), GUI::GetGUI()->AppHeight(), INTERACTIVE | MODAL),
    m_lb_wnd(GetStyleFactory()->NewDropDownListListBox(color, color)),
    m_relative_to_wnd(relative_to_wnd)
{
    Connect(m_lb_wnd->SelChangedSignal,     &ModalListPicker::LBSelChangedSlot, this);
    Connect(m_lb_wnd->LeftClickedSignal,    &ModalListPicker::LBLeftClickSlot,  this);
    AttachChild(m_lb_wnd);

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(SelChangedSignal, ModalListPickerSelChangedEcho(*this));
}

void ModalListPicker::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ m_done = true; }

void ModalListPicker::ModalInit()
{
    if (m_relative_to_wnd)
        m_lb_wnd->MoveTo(Pt(m_relative_to_wnd->Left(), m_relative_to_wnd->Bottom()));
    Show();
}

void ModalListPicker::LBSelChangedSlot(const ListBox::SelectionSet& rows)
{
    Hide();
    if (rows.empty()) {
        SelChangedSignal(m_lb_wnd->end());
    } else {
        ListBox::iterator sel_it = *rows.begin();
        SelChangedSignal(sel_it);
    }
    m_done = true;
}

void ModalListPicker::LBLeftClickSlot(ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
{
    Hide();
    m_done = true;
}


////////////////////////////////////////////////
// GG::DropDownList
////////////////////////////////////////////////
DropDownList::DropDownList(size_t num_shown_elements, Clr color) :
    Control(X0, Y0, X1, Y1, INTERACTIVE),
    m_modal_picker(new ModalListPicker(color, this)),
    m_num_shown_elements(num_shown_elements)
{
    SetStyle(LIST_SINGLESEL);

    Connect(m_modal_picker->SelChangedSignal, SelChangedSignal);

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(SelChangedSignal, DropDownListSelChangedEcho(*this));

    InitBuffer();
}

DropDownList::~DropDownList() {
    if (m_modal_picker)
        m_modal_picker->EndRun();
    DetachChild(m_modal_picker);
    delete m_modal_picker;

    m_buffer.clear();
}

DropDownList::iterator DropDownList::CurrentItem() const
{
    if (m_modal_picker->LB()->Selections().empty())
        return m_modal_picker->LB()->end();
    else
        return *m_modal_picker->LB()->Selections().begin();
}

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
    if (!sel_visible) {
        // The following is necessary because neither LB() nor the selected row may be visible and
        // prerendered.
        if (!lb_visible) {
            LB()->Show();
            GUI::GetGUI()->PreRenderWindow(LB());
            LB()->Hide();
        }

        current_item->Show();
        GUI::GetGUI()->PreRenderWindow(current_item);
    }

    Pt offset = ClientUpperLeft() - current_item->UpperLeft();
    current_item->OffsetMove(offset);

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
    GG::Pt sz = Size();
    Wnd::SizeMove(ul, lr);
    Pt drop_down_size(ClientWidth(), ClientHeight());

    // reset size of displayed drop list based on number of shown rows set.
    // assumes that all rows have the same height.
    // adds some magic padding for now to prevent the scroll bars showing up.
    if (LB()->NumRows() > 0)
        drop_down_size.y = LB()->GetRow(0).Height() * std::min<int>(m_num_shown_elements, LB()->NumRows()) + 4;
    LB()->SizeMove(Pt(X0, Height()), Pt(X0, Height()) + drop_down_size);

    if (sz != Size())
        InitBuffer();
}

void DropDownList::SetColor(Clr c)
{ LB()->SetColor(c); }

DropDownList::iterator DropDownList::Insert(Row* row, iterator it, bool signal/* = true*/)
{
    row->SetDragDropDataType("");
    DropDownList::iterator ret = LB()->Insert(row, it, signal);
    Resize(Size());
    return ret;
}

DropDownList::iterator DropDownList::Insert(Row* row, bool signal/* = true*/)
{
    row->SetDragDropDataType("");
    DropDownList::iterator ret = LB()->Insert(row, signal);
    Resize(Size());
    return ret;
}

void DropDownList::Insert(const std::vector<Row*>& rows, iterator it, bool signal/* = true*/)
{
    for (std::vector<Row*>::const_iterator rows_it = rows.begin();
         rows_it != rows.end(); ++rows_it)
    { (*rows_it)->SetDragDropDataType(""); }
    LB()->Insert(rows, it, signal);
    Resize(Size());
}

void DropDownList::Insert(const std::vector<Row*>& rows, bool signal/* = true*/)
{
    for (std::vector<Row*>::const_iterator rows_it = rows.begin();
         rows_it != rows.end(); ++rows_it)
    { (*rows_it)->SetDragDropDataType(""); }
    LB()->Insert(rows, signal);
    Resize(Size());
}

DropDownList::Row* DropDownList::Erase(iterator it, bool signal/* = false*/)
{ return LB()->Erase(it, signal); }

void DropDownList::Clear()
{
    m_modal_picker->EndRun();
    LB()->Clear();
}

DropDownList::iterator DropDownList::begin()
{ return LB()->begin(); }

DropDownList::iterator DropDownList::end()
{ return LB()->end(); }

DropDownList::Row& DropDownList::GetRow(std::size_t n)
{ return LB()->GetRow(n); }

void DropDownList::Select(iterator it)
{ SelectImpl(it, false); }

void DropDownList::Select(std::size_t n)
{ SelectImpl(n < LB()->NumRows() ? boost::next(LB()->begin(), n) : LB()->end(), false); }

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

    m_modal_picker->Run();
}

void DropDownList::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        switch (key) {
        case GGK_UP: // arrow-up (not numpad arrow)
            if (CurrentItem() != LB()->end() && CurrentItem() != LB()->begin())
                SelectImpl(boost::prior(CurrentItem()), true);
            break;
        case GGK_DOWN: // arrow-down (not numpad arrow)
            if (CurrentItem() != LB()->end() && CurrentItem() != --LB()->end())
                SelectImpl(boost::next(CurrentItem()), true);
            break;
        case GGK_PAGEUP: // page up key (not numpad key)
            if (LB()->NumRows() && CurrentItem() != LB()->end()) {
                std::size_t i = 10;
                iterator it = CurrentItem();
                while (i && it != LB()->begin()) {
                    --it;
                    --i;
                }
                SelectImpl(it, true);
            }
            break;
        case GGK_PAGEDOWN: // page down key (not numpad key)
            if (LB()->NumRows()) {
                std::size_t i = 10;
                iterator it = CurrentItem();
                while (i && it != --LB()->end()) {
                    ++it;
                    ++i;
                }
                SelectImpl(it, true);
            }
            break;
        case GGK_HOME: // home key (not numpad)
            if (LB()->NumRows())
                SelectImpl(LB()->begin(), true);
            break;
        case GGK_END: // end key (not numpad)
            if (LB()->NumRows() && !LB()->Empty())
                SelectImpl(--LB()->end(), true);
            break;
        default:
            Control::KeyPress(key, key_code_point, mod_keys);
        }
    } else {
        Control::KeyPress(key, key_code_point, mod_keys);
    }
}

void DropDownList::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        if (CurrentItem() == LB()->end())
            return;
        if (move == 0)
            return;

        //std::cout << "DropDownList::MouseWheel move: " << move << std::endl;

        DropDownList::iterator cur_it = CurrentItem();

        if (move > 0) {
            int dist_to_last = std::distance(cur_it, end()) - 1; // end is one past last valid item
            if (move > dist_to_last)
                move = dist_to_last;
        } else {
            int dist_from_first = std::distance(begin(), cur_it);// begin is the first valid item
            if (-move > dist_from_first)
                move = -dist_from_first;
        }
        if (move != 0) {
            std::advance(cur_it, move);
            SelectImpl(cur_it, true);
        }

    } else {
        Control::MouseWheel(pt, move, mod_keys);
    }
}

ListBox* DropDownList::LB()
{ return m_modal_picker->LB(); }

const ListBox* DropDownList::LB() const
{ return m_modal_picker->LB(); }

void DropDownList::SelectImpl(iterator it, bool signal)
{
    iterator old_m_current_item = CurrentItem();
    if (it == LB()->end()) {
        LB()->DeselectAll();
    } else {
        LB()->SelectRow(it);
    }

    if (signal && CurrentItem() != old_m_current_item)
        SelChangedSignal(CurrentItem());
}
