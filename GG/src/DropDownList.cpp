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
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    struct SelChangedEcho
    {
        SelChangedEcho(const DropDownList& drop_list) :
            m_drop_list(drop_list)
            {}
        void operator()(const DropDownList::iterator& it)
            {
                std::cerr << "GG SIGNAL : DropDownList::SelChangedSignal(row="
                          << m_drop_list.IteratorToIndex(it)
                          << ")\n";
            }
        const DropDownList& m_drop_list;
    };

    const int BORDER_THICK = 2; // should be the same as the BORDER_THICK value in GGListBox.h

    class ModalListPicker : public Wnd
    {
    public:
        ModalListPicker(DropDownList* drop_wnd, ListBox* lb_wnd) :
            Wnd(X0, Y0, GUI::GetGUI()->AppWidth(), GUI::GetGUI()->AppHeight(),
                INTERACTIVE | MODAL),
            m_drop_wnd(drop_wnd),
            m_lb_wnd(lb_wnd),
            m_old_lb_ul(m_lb_wnd->UpperLeft())
        {
            m_connection_1 =
                Connect(m_lb_wnd->SelChangedSignal, &ModalListPicker::LBSelChangedSlot, this);
            m_connection_2 =
                Connect(m_lb_wnd->LeftClickedSignal, &ModalListPicker::LBLeftClickSlot, this);
            m_lb_ul = m_old_lb_ul + m_drop_wnd->UpperLeft();
            AttachChild(m_lb_wnd);
        }

        virtual void Render()
        { m_lb_wnd->MoveTo(m_lb_ul); }

        ~ModalListPicker()
        {
            m_lb_wnd->MoveTo(m_old_lb_ul);
            DetachChild(m_lb_wnd);
        }

    protected:
        virtual void LClick(const Pt& pt, Flags<ModKey> mod_keys)
        { m_done = true; }

    private:
        void LBSelChangedSlot(const ListBox::SelectionSet& rows)
        {
            if (!rows.empty()) {
                m_drop_wnd->Select(*rows.begin());
                m_drop_wnd->SelChangedSignal(m_drop_wnd->CurrentItem());
                m_done = true;
            }
        }

        void LBLeftClickSlot(ListBox::iterator it, const Pt&)
        { m_done = true; }

        DropDownList*  m_drop_wnd;
        ListBox*       m_lb_wnd;
        Pt             m_old_lb_ul;
        Pt             m_lb_ul;

        boost::signals2::scoped_connection m_connection_1;
        boost::signals2::scoped_connection m_connection_2;
    };
}

////////////////////////////////////////////////
// GG::DropDownList
////////////////////////////////////////////////
DropDownList::DropDownList() :
    Control(),
    m_current_item(),
    m_LB(0),
    m_modal_picker(0)
{}

DropDownList::DropDownList(X x, Y y, X w, Y h, Y drop_ht, Clr color,
                           Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Control(x, y, w, h, flags),
    m_current_item(),
    m_LB(GetStyleFactory()->NewDropDownListListBox(x, y, w, drop_ht, color, color, flags)),
    m_modal_picker(0)
{
    SetStyle(LIST_SINGLESEL);
    // adjust size to keep correct height based on row height, etc.
    Wnd::SizeMove(Pt(x, y), Pt(x + Size().x, y + h + 2 * static_cast<int>(m_LB->CellMargin()) + 2 * BORDER_THICK));
    m_LB->SizeMove(Pt(X0, Height()), Pt(Width(), Height() + m_LB->Height()));
    m_current_item = m_LB->end();

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(SelChangedSignal, SelChangedEcho(*this));
}

DropDownList::~DropDownList() {
    if (m_modal_picker)
        m_modal_picker->EndRun();
    m_modal_picker = 0;
    delete m_LB;
}

DropDownList::iterator DropDownList::CurrentItem() const
{ return m_current_item; }

std::size_t DropDownList::CurrentItemIndex() const
{ return IteratorToIndex(m_current_item); }

std::size_t DropDownList::IteratorToIndex(iterator it) const
{ return it == m_LB->end() ? -1 : std::distance(m_LB->begin(), it); }

DropDownList::iterator DropDownList::IndexToIterator(std::size_t n) const
{ return n < m_LB->NumRows() ? boost::next(m_LB->begin(), n) : m_LB->end(); }

bool DropDownList::Empty() const
{ return m_LB->Empty(); }

DropDownList::const_iterator DropDownList::begin() const
{ return m_LB->begin(); }

DropDownList::const_iterator DropDownList::end() const
{ return m_LB->end(); }

DropDownList::const_reverse_iterator DropDownList::rbegin() const
{ return m_LB->rbegin(); }

DropDownList::const_reverse_iterator DropDownList::rend() const
{ return m_LB->rend(); }

const DropDownList::Row& DropDownList::GetRow(std::size_t n) const
{ return m_LB->GetRow(n); }

bool DropDownList::Selected(iterator it) const
{ return m_LB->Selected(it); }

bool DropDownList::Selected(std::size_t n) const
{ return n < m_LB->NumRows() ? m_LB->Selected(boost::next(m_LB->begin(), n)) : false; }

Clr DropDownList::InteriorColor() const
{ return m_LB->InteriorColor(); }

Y DropDownList::DropHeight() const
{ return m_LB->Height(); }

Flags<ListBoxStyle> DropDownList::Style() const
{ return m_LB->Style(); }

std::size_t DropDownList::NumRows() const
{ return m_LB->NumRows(); }

std::size_t DropDownList::NumCols() const
{ return m_LB->NumCols(); }

std::size_t DropDownList::SortCol() const
{ return m_LB->SortCol(); }

X DropDownList::ColWidth(std::size_t n) const
{ return m_LB->ColWidth(n); }

Alignment DropDownList::ColAlignment(std::size_t n) const
{ return m_LB->ColAlignment(n); }

Alignment DropDownList::RowAlignment(iterator it) const
{ return m_LB->RowAlignment(it); }

Pt DropDownList::ClientUpperLeft() const
{ return UpperLeft() + Pt(X(BORDER_THICK), Y(BORDER_THICK)); }

Pt DropDownList::ClientLowerRight() const
{ return LowerRight() - Pt(X(BORDER_THICK), Y(BORDER_THICK)); }

void DropDownList::Render()
{
    // draw beveled rectangle around client area
    Pt ul = UpperLeft(), lr = LowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(m_LB->Color()) : m_LB->Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_LB->m_int_color) : m_LB->m_int_color;

    BeveledRectangle(ul, lr, int_color_to_use, color_to_use, false, BORDER_THICK);

    // Draw the ListBox::Row of currently displayed item, if any.
    if (m_current_item != m_LB->end()) {
        Row* current_item = *m_current_item;
        Pt offset = ClientUpperLeft() - current_item->UpperLeft();
        bool visible = current_item->Visible();
        current_item->OffsetMove(offset);
        if (!visible)
            current_item->Show();
        BeginClipping();
        GUI::GetGUI()->RenderWindow(current_item);
        EndClipping();
        current_item->OffsetMove(-offset);
        if (!visible)
            current_item->Hide();
    }
}

void DropDownList::SizeMove(const Pt& ul, const Pt& lr)
{
    // adjust size to keep correct height based on row height, etc.
    Wnd::SizeMove(ul, lr);
    m_LB->SizeMove(Pt(X0, Height()), Pt(Width(), Height() + m_LB->Height()));
}

void DropDownList::SetColor(Clr c)
{ m_LB->SetColor(c); }

DropDownList::iterator DropDownList::Insert(Row* row, iterator it, bool signal/* = true*/)
{
    row->SetDragDropDataType("");
    return m_LB->Insert(row, it, signal);
}

DropDownList::iterator DropDownList::Insert(Row* row, bool signal/* = true*/)
{
    row->SetDragDropDataType("");
    return m_LB->Insert(row, signal);
}

void DropDownList::Insert(const std::vector<Row*>& rows, iterator it, bool signal/* = true*/)
{
    for (std::vector<Row*>::const_iterator rows_it = rows.begin();
         rows_it != rows.end(); ++rows_it)
    { (*rows_it)->SetDragDropDataType(""); }
    m_LB->Insert(rows, it, signal);
}

void DropDownList::Insert(const std::vector<Row*>& rows, bool signal/* = true*/)
{
    for (std::vector<Row*>::const_iterator rows_it = rows.begin();
         rows_it != rows.end(); ++rows_it)
    { (*rows_it)->SetDragDropDataType(""); }
    m_LB->Insert(rows, signal);
}

DropDownList::Row* DropDownList::Erase(iterator it, bool signal/* = false*/)
{
    if (it == m_current_item)
        m_current_item = m_LB->end();
    return m_LB->Erase(it, signal);
}

void DropDownList::Clear()
{
    m_current_item = m_LB->end();
    if (m_modal_picker)
        m_modal_picker->EndRun();
    m_modal_picker = 0;
    m_LB->Clear();
}

DropDownList::iterator DropDownList::begin()
{ return m_LB->begin(); }

DropDownList::iterator DropDownList::end()
{ return m_LB->end(); }

DropDownList::reverse_iterator DropDownList::rbegin()
{ return m_LB->rbegin(); }

DropDownList::reverse_iterator DropDownList::rend()
{ return m_LB->rend(); }

DropDownList::Row& DropDownList::GetRow(std::size_t n)
{ return m_LB->GetRow(n); }

void DropDownList::Select(iterator it)
{ SelectImpl(it, false); }

void DropDownList::Select(std::size_t n)
{ SelectImpl(n < m_LB->NumRows() ? boost::next(m_LB->begin(), n) : m_LB->end(), false); }

void DropDownList::SetInteriorColor(Clr c)
{ m_LB->SetInteriorColor(c); }

void DropDownList::SetDropHeight(Y h)
{ m_LB->Resize(Pt(Width(), h)); }

void DropDownList::SetStyle(Flags<ListBoxStyle> s)
{
    s &= ~(LIST_NOSEL | LIST_QUICKSEL | LIST_USERDELETE | LIST_BROWSEUPDATES);
    s |= LIST_SINGLESEL;
    m_LB->SetStyle(s);
    m_current_item = m_LB->end();
}

void DropDownList::SetNumCols(std::size_t n)
{ m_LB->SetNumCols(n); }

void DropDownList::SetSortCol(std::size_t n)
{
    m_LB->SetSortCol(n);
    m_current_item = m_LB->end();
}

void DropDownList::SetColWidth(std::size_t n, X w)
{ m_LB->SetColWidth(n, w); }

void DropDownList::LockColWidths()
{ m_LB->LockColWidths(); }

void DropDownList::UnLockColWidths()
{ m_LB->UnLockColWidths(); }

void DropDownList::SetColAlignment(std::size_t n, Alignment align) 
{ m_LB->SetColAlignment(n, align); }

void DropDownList::SetRowAlignment(iterator it, Alignment align) 
{ m_LB->SetRowAlignment(it, align); }

void DropDownList::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    ModalListPicker picker(this, m_LB);
    m_modal_picker = &picker;

    const ListBox::SelectionSet& LB_sels = m_LB->Selections();
    if (!LB_sels.empty()) {
        if (m_LB->m_vscroll) {
            m_LB->m_vscroll->ScrollTo(0);
            SignalScroll(*m_LB->m_vscroll, true);
        }
    }
    m_LB->m_first_col_shown = 0;
    picker.Run();
    m_modal_picker = 0;
}

void DropDownList::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        switch (key) {
        case GGK_UP: // arrow-up (not numpad arrow)
            if (m_current_item != m_LB->end() && m_current_item != m_LB->begin())
                SelectImpl(boost::prior(m_current_item), true);
            break;
        case GGK_DOWN: // arrow-down (not numpad arrow)
            if (m_current_item != m_LB->end() && m_current_item != --m_LB->end())
                SelectImpl(boost::next(m_current_item), true);
            break;
        case GGK_PAGEUP: // page up key (not numpad key)
            if (m_LB->NumRows() && m_current_item != m_LB->end()) {
                std::size_t i = 10;
                iterator it = m_current_item;
                while (i && it != m_LB->begin()) {
                    --it;
                    --i;
                }
                SelectImpl(it, true);
            }
            break;
        case GGK_PAGEDOWN: // page down key (not numpad key)
            if (m_LB->NumRows()) {
                std::size_t i = 10;
                iterator it = m_current_item;
                while (i && it != --m_LB->end()) {
                    ++it;
                    ++i;
                }
                SelectImpl(it, true);
            }
            break;
        case GGK_HOME: // home key (not numpad)
            if (m_LB->NumRows())
                SelectImpl(m_LB->begin(), true);
            break;
        case GGK_END: // end key (not numpad)
            if (m_LB->NumRows() && !m_LB->Empty())
                SelectImpl(--m_LB->end(), true);
            break;
        default:
            Control::KeyPress(key, key_code_point, mod_keys);
        }
    } else {
        Control::KeyPress(key, key_code_point, mod_keys);
    }
}

ListBox* DropDownList::LB()
{ return m_LB; }

void DropDownList::SelectImpl(iterator it, bool signal)
{
    iterator old_m_current_item = m_current_item;
    if (it == m_LB->end()) {
        m_current_item = m_LB->end();
        m_LB->DeselectAll();
    } else {
        m_current_item = it;
        m_LB->SelectRow(m_current_item);
    }

    if (signal && m_current_item != old_m_current_item)
        SelChangedSignal(m_current_item);
}
