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
        m_prompt(nullptr)
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

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
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
    m_showing_prompt(false),
    m_prompt_str(prompt_str)
{
    if (!drop_type_str.empty())
        AllowDropType(drop_type_str);

    BeforeInsertSignal.connect(
        boost::bind(&QueueListBox::EnsurePromptHiddenSlot, this, _1));
    AfterEraseSignal.connect(
        boost::bind(&QueueListBox::ShowPromptConditionallySlot, this, _1));
    ClearedSignal.connect(
        boost::bind(&QueueListBox::ShowPromptSlot, this));
    GG::ListBox::RightClickedSignal.connect(
        boost::bind(&QueueListBox::ItemRightClicked, this, _1, _2, _3));

    SetNumCols(1);
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);

    ShowPromptSlot();
}

GG::X QueueListBox::RowWidth() const
{ return ClientWidth(); }

void QueueListBox::KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    if (Disabled()) {
        CUIListBox::KeyPress(key, key_code_point, mod_keys);
        return;
    }
    if (key == GG::GGK_DELETE) {
        QueueListBox::iterator it = Caret();
        if (it == end())
            return;
        QueueItemDeletedSignal(it);
    } else {
        CUIListBox::KeyPress(key, key_code_point, mod_keys);
    }
}

void QueueListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                   const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    if (std::distance(first, last) != 1) {
        ErrorLogger() << "QueueListBox::DropsAcceptable unexpected passed more than one Wnd to test";
    }

    for (DropsAcceptableIter it = first; it != last; ++it) {
        it->second = m_order_issuing_enabled &&
            AllowedDropTypes().find(it->first->DragDropDataType()) != AllowedDropTypes().end();
    }
}

void QueueListBox::AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (wnds.empty())
        return;
    if (wnds.size() > 1) {
        // delete any extra wnds that won't be processed below
        for (std::vector<GG::Wnd*>::const_iterator it = ++wnds.begin(); it != wnds.end(); ++it)
            delete *it;
        ErrorLogger() << "QueueListBox::AcceptDrops given multiple wnds unexpectedly...";
    }
    GG::Wnd* wnd = *wnds.begin();
    const std::string& drop_type = wnd->DragDropDataType();
    GG::ListBox::Row* row = boost::polymorphic_downcast<GG::ListBox::Row*>(wnd);
    if (AllowedDropTypes().find(drop_type) == AllowedDropTypes().end() ||
        !row ||
        std::find(begin(), end(), row) == end())
    {
        delete wnd;
        return;
    }
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
            GG::Control* panel =  row->at(0);
            ul.x = panel->Left();
            lr.x = panel->Right();
        }
        GG::FlatRectangle(GG::Pt(ul.x, ul.y - 1), GG::Pt(lr.x, ul.y), GG::CLR_ZERO, GG::CLR_WHITE, 1);
    }
}

void QueueListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::SizeMove(ul, lr);
    if (old_size != Size() && !Empty()) {
        const GG::Pt row_size(RowWidth(), (*begin())->Height());
        for (GG::ListBox::Row* row : *this)
            row->Resize(row_size);
    }
}

void QueueListBox::DragDropHere(const GG::Pt& pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                                 GG::Flags<GG::ModKey> mod_keys)
{
    CUIListBox::DragDropHere(pt, drop_wnds_acceptable, mod_keys);

    if (drop_wnds_acceptable.size() == 1 &&
        AllowedDropTypes().find(drop_wnds_acceptable.begin()->first->DragDropDataType()) !=
        AllowedDropTypes().end())
    {
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

void QueueListBox::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled())
        switch (key) {
        case GG::GGK_DELETE: // delete key
            QueueItemDeletedSignal(Caret());
            break;
        default:
            ListBox::KeyPress(key, key_code_point, mod_keys); // pass on if not delete key
            break;
    }
}

void QueueListBox::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    for (GG::ListBox::Row* row : *this)
        row->Disable(!enable);
}

bool QueueListBox::DisplayingValidQueueItems()
{ return !m_showing_prompt; }

void QueueListBox::Clear() {
    CUIListBox::Clear();
    DragDropLeave();
}

std::function<void()> QueueListBox::MoveToTopAction(GG::ListBox::iterator it) const {
    return [it, this]() {
        if (GG::ListBox::Row* row = *it)
            QueueItemMovedSignal(row, 0);
    };
}

std::function<void()> QueueListBox::MoveToBottomAction(GG::ListBox::iterator it) const {
    return [it, this]() {
        if (GG::ListBox::Row* row = *it)
            QueueItemMovedSignal(row, NumRows());
    };
}

std::function<void()> QueueListBox::DeleteAction(GG::ListBox::iterator it) const
{ return [it, this]() { QueueItemDeletedSignal(it); }; }

void QueueListBox::ItemRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
{ this->ItemRightClickedImpl(it, pt, modkeys); }

void QueueListBox::ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    CUIPopupMenu popup(pt.x, pt.y);
    popup.AddMenuItem(GG::MenuItem(UserString("MOVE_UP_QUEUE_ITEM"),   false, false, MoveToTopAction(it)));
    popup.AddMenuItem(GG::MenuItem(UserString("MOVE_DOWN_QUEUE_ITEM"), false, false, MoveToBottomAction(it)));
    popup.AddMenuItem(GG::MenuItem(UserString("DELETE_QUEUE_ITEM"),    false, false, DeleteAction(it)));
    popup.Run();
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
        ShowPromptSlot();
    }
}
