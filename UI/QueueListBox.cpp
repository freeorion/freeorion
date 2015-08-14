#include "QueueListBox.h"

#include "../util/i18n.h"
#include "../util/Logger.h"

#include <GG/DrawUtil.h>
#include <GG/WndEvent.h>

#include <boost/cast.hpp>

////////////////////////////////////////////////////////////
// PromptRow
////////////////////////////////////////////////////////////
struct PromptRow : GG::ListBox::Row {
    PromptRow(GG::X w, std::string prompt_str) :
        GG::ListBox::Row(w, GG::Y(20), ""),
        m_prompt(0)
    {
        //std::cout << "PromptRow(" << w << ", ...)" << std::endl;

        m_prompt = new CUILabel(prompt_str, GG::FORMAT_TOP | GG::FORMAT_LEFT | GG::FORMAT_LINEWRAP | GG::FORMAT_WORDBREAK);
        m_prompt->MoveTo(GG::Pt(GG::X(2), GG::Y(2)));
        m_prompt->Resize(GG::Pt(Width() - 10, Height()));
        m_prompt->SetTextColor(GG::LightColor(ClientUI::TextColor()));
        m_prompt->ClipText(true);
        Resize(GG::Pt(w, m_prompt->Height()));
        push_back(m_prompt);
    }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        GG::ListBox::Row::SizeMove(ul, lr);
        if (!empty() && old_size != Size() && m_prompt)
            m_prompt->Resize(Size());
    }

private:
     GG::Label* m_prompt;
};


////////////////////////////////////////////////////////////
// QueueListBox
////////////////////////////////////////////////////////////
QueueListBox::QueueListBox(const std::string& drop_type_str, const std::string& prompt_str) :
    CUIListBox(),
    m_drop_point(end()),
    m_show_drop_point(false),
    m_order_issuing_enabled(true),
    m_showing_prompt(true),
    m_prompt_str(prompt_str)
{
    AllowDropType(drop_type_str);

    GG::Connect(BeforeInsertSignal,                 &QueueListBox::EnsurePromptHiddenSlot,      this);
    GG::Connect(AfterEraseSignal,                   &QueueListBox::ShowPromptConditionallySlot, this);
    GG::Connect(ClearedSignal,                      &QueueListBox::ShowPromptSlot,              this);
    GG::Connect(GG::ListBox::RightClickedSignal,    &QueueListBox::ItemRightClicked,            this);

    // preinitialize listbox/row column widths, because what
    // ListBox::Insert does on default is not suitable for this case
    SetNumCols(1);
    SetColWidth(0, GG::X0);
    LockColWidths();
    NormalizeRowsOnInsert(false);

    Insert(new PromptRow(RowWidth(), m_prompt_str));
}

GG::X QueueListBox::RowWidth() const
{ return Width() - ClientUI::ScrollWidth() - 5; }

void QueueListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::SizeMove(ul, lr);

    if (old_size != Size()) {
        for (GG::ListBox::iterator it = begin(); it != end(); ++it) {
            GG::Pt old_row_sz = (*it)->Size();
            (*it)->Resize(GG::Pt(Width() - ClientUI::ScrollWidth() - 5, old_row_sz.y));
        }
    }
}

void QueueListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
    if (std::distance(first, last) != 1) {
        ErrorLogger() << "QueueListBox::DropsAcceptable unexpected passed more than one Wnd to test";
    }

    for (DropsAcceptableIter it = first; it != last; ++it) {
        it->second = m_order_issuing_enabled &&
            AllowedDropTypes().find(it->first->DragDropDataType()) != AllowedDropTypes().end();
    }
}

void QueueListBox::AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
    if (wnds.size() != 1)
        return;
    GG::Wnd* wnd = *wnds.begin();
    const std::string& drop_type = wnd->DragDropDataType();
    if (AllowedDropTypes().find(drop_type) == AllowedDropTypes().end())
        return;
    GG::ListBox::Row* row = boost::polymorphic_downcast<GG::ListBox::Row*>(wnd);
    if (!row)
        return;
    if (std::find(begin(), end(), row) == end())
        return;
    iterator it = RowUnderPt(pt);
    QueueItemMovedSignal(row, std::distance(begin(), it));
}

void QueueListBox::Render() {
    ListBox::Render();
    // render drop point line
    if (m_show_drop_point && m_order_issuing_enabled) {
        GG::ListBox::Row* row = *(m_drop_point == end() ? --end() : m_drop_point);
        if (!row)
            return;
        GG::Pt ul = row->UpperLeft(), lr = row->LowerRight();
        if (m_drop_point == end())
            ul.y = lr.y;
        if (!row->empty()) {
            GG::Control* panel = (*row)[0];
            ul.x = panel->Left();
            lr.x = panel->Right();
        }
        GG::FlatRectangle(GG::Pt(ul.x, ul.y - 1), GG::Pt(lr.x, ul.y), GG::CLR_ZERO, GG::CLR_WHITE, 1);
    }
}

void QueueListBox::DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds,
                                 GG::Flags<GG::ModKey> mod_keys)
{ DragDropHere(pt, drag_drop_wnds, mod_keys); }

void QueueListBox::DragDropHere(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds,
                                GG::Flags<GG::ModKey> mod_keys)
{
    CUIListBox::DragDropHere(pt, drag_drop_wnds, mod_keys);
    if (drag_drop_wnds.size() == 1 &&
        AllowedDropTypes().find(drag_drop_wnds.begin()->first->DragDropDataType()) !=
        AllowedDropTypes().end()) {
        m_drop_point = RowUnderPt(pt);
        m_show_drop_point = true;
    } else {
        m_drop_point = end();
        m_show_drop_point = false;
    }
}

void QueueListBox::DragDropLeave() {
    m_drop_point = end();
    m_show_drop_point = false;
}

void QueueListBox::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    for (GG::ListBox::iterator it = begin(); it != end(); ++it)
        (*it)->Disable(!enable);
}

bool QueueListBox::DisplayingValidQueueItems()
{ return !m_showing_prompt; }

void QueueListBox::Clear() {
    CUIListBox::Clear();
    DragDropLeave();
}

void QueueListBox::ItemRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    // Create popup menu with a Delete Item command to provide same functionality as
    // DoubleClick since under laggy conditions it DoubleClick can have trouble
    // being interpreted correctly (can instead be treated as simply two unrelated left clicks)

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_UP_QUEUE_ITEM"),   1, false, false));
    menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_DOWN_QUEUE_ITEM"), 2, false, false));
    menu_contents.next_level.push_back(GG::MenuItem(UserString("DELETE_QUEUE_ITEM"),    3, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // move item to top
            if (GG::ListBox::Row* row = *it)
                QueueItemMovedSignal(row, 0);
            break;
        }
        case 2: { // move item to bottom
            if (GG::ListBox::Row* row = *it)
                QueueItemMovedSignal(row, NumRows());
            break;
        }
        case 3: { // delete item
            DoubleClickedSignal(it);
            break;
        }

        default:
            break;
        }
    }
}

void QueueListBox::EnsurePromptHiddenSlot(iterator it) {
    if (m_showing_prompt) {
        Erase(begin(), false, false); // if the prompt is shown, it must be the only row in the ListBox
        m_showing_prompt = false;
    }
}

void QueueListBox::ShowPromptSlot() {
    Insert(new PromptRow(Width() - 4, m_prompt_str), begin(), false, false);
    m_showing_prompt = true;
}

void QueueListBox::ShowPromptConditionallySlot(iterator it) {
    if (begin() == end()) {
        Insert(new PromptRow(Width() - 4, m_prompt_str), begin(), false, false);
        m_showing_prompt = true;
    }
}

