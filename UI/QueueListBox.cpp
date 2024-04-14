#include "QueueListBox.h"

#include "../util/i18n.h"
#include "../util/Logger.h"

#include <GG/WndEvent.h>

////////////////////////////////////////////////////////////
// PromptRow
////////////////////////////////////////////////////////////
PromptRow::PromptRow(GG::X w, const std::string& prompt) :
    GG::ListBox::Row(w, GG::Y(20)),
    m_prompt(
        GG::Wnd::Create<CUILabel>(
            prompt,
            GG::FORMAT_TOP | GG::FORMAT_LEFT | GG::FORMAT_LINEWRAP | GG::FORMAT_WORDBREAK))
{}

void PromptRow::CompleteConstruction() {
    GG::ListBox::Row::CompleteConstruction();

    m_prompt->MoveTo(GG::Pt(GG::X(2), GG::Y(2)));
    m_prompt->Resize(GG::Pt(Width() - 10, Height()));
    m_prompt->SetTextColor(GG::LightenClr(ClientUI::TextColor()));
    m_prompt->ClipText(true);
    Resize(GG::Pt(Width(), m_prompt->Height()));
    push_back(m_prompt);
}

void PromptRow::SizeMove(GG::Pt ul, GG::Pt lr)  {
    const auto old_size = Size();
    GG::ListBox::Row::SizeMove(ul, lr);
    if (!empty() && old_size != Size() && m_prompt)
        m_prompt->Resize(Size());
}


////////////////////////////////////////////////////////////
// QueueListBox
////////////////////////////////////////////////////////////
QueueListBox::QueueListBox(boost::optional<std::string_view> drop_type_str,
                           std::string prompt_str) :
    m_drop_point(end()),
    m_prompt_str(std::move(prompt_str))
{
    if (drop_type_str)
        AllowDropType(*drop_type_str);
}

void QueueListBox::CompleteConstruction() {
    CUIListBox::CompleteConstruction();

    SetNumCols(1);
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);

    ShowPromptSlot();

    namespace ph = boost::placeholders;

    BeforeInsertRowSignal.connect(
        boost::bind(&QueueListBox::EnsurePromptHiddenSlot, this, ph::_1));
    AfterEraseRowSignal.connect(
        boost::bind(&QueueListBox::ShowPromptConditionallySlot, this, ph::_1));
    ClearedRowsSignal.connect(
        boost::bind(&QueueListBox::ShowPromptSlot, this));
    GG::ListBox::RightClickedRowSignal.connect(
        boost::bind(&QueueListBox::ItemRightClicked, this, ph::_1, ph::_2, ph::_3));
}

GG::X QueueListBox::RowWidth() const noexcept
{ return ClientWidth(); }

void QueueListBox::KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    if (Disabled()) {
        CUIListBox::KeyPress(key, key_code_point, mod_keys);
        return;
    }
    if (key == GG::Key::GGK_DELETE) {
        auto it = Caret();
        if (it == end())
            return;
        QueueItemDeletedSignal(it);
    } else {
        CUIListBox::KeyPress(key, key_code_point, mod_keys);
    }
}

void QueueListBox::AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (wnds.size() > 1)
        ErrorLogger() << "QueueListBox::AcceptDrops given multiple wnds unexpectedly...";
    auto& wnd = wnds.front();
    if (!AllowedDropType(wnd->DragDropDataType()))
        return;

    const auto row = std::dynamic_pointer_cast<GG::ListBox::Row>(wnd);
    if (!row || !std::count(begin(), end(), row))
        return;

    ListBox::AcceptDrops(pt, std::vector<std::shared_ptr<GG::Wnd>>{wnd}, mod_keys);
}

void QueueListBox::Render() {
    ListBox::Render();
    // render drop point line
    if (m_show_drop_point && m_order_issuing_enabled) {
        auto&& row = m_drop_point == end() ? (--end())->get() : m_drop_point->get();
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

void QueueListBox::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    CUIListBox::SizeMove(ul, lr);
    if (old_size != Size() && !Empty()) {
        const GG::Pt row_size(RowWidth(), (*begin())->Height());
        for (auto& row : *this)
            row->Resize(row_size);
    }
}

void QueueListBox::DragDropHere(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                                 GG::Flags<GG::ModKey> mod_keys)
{
    CUIListBox::DragDropHere(pt, drop_wnds_acceptable, mod_keys);

    if (drop_wnds_acceptable.size() == 1 &&
        AllowedDropType(drop_wnds_acceptable.begin()->first->DragDropDataType()))
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

int QueueListBox::IteraterIndex(const const_iterator it) {
    if (it == this->end())
        return -1;

    std::size_t dist = 0;
    for (auto qit = this->begin(); qit != this->end(); ++qit) {
        if (qit == it)
            return dist;
        dist++;
    }
    return -1;
}

void QueueListBox::EnableOrderIssuing(bool enable) {
    m_order_issuing_enabled = enable;
    AllowDrops(enable);
    for (auto& row : *this)
        row->Disable(!enable);
}

bool QueueListBox::DisplayingValidQueueItems() const noexcept
{ return !m_showing_prompt; }

void QueueListBox::Clear() {
    CUIListBox::Clear();
    DragDropLeave();
}

void QueueListBox::SetEmptyPromptText(std::string prompt) {
    if (m_prompt_str == prompt)
        return;

    m_prompt_str = std::move(prompt);

    if (m_showing_prompt)
        ShowPromptSlot();
}

std::function<void()> QueueListBox::MoveToTopAction(GG::ListBox::iterator it) {
    return [it, this]() {
        if (OrderIssuingEnabled())
            ListBox::Insert(*it, begin(), true);
    };
}

std::function<void()> QueueListBox::MoveToBottomAction(GG::ListBox::iterator it) {
    return [it, this]() {
        if (OrderIssuingEnabled())
            ListBox::Insert(*it, end(), true);
    };
}

std::function<void()> QueueListBox::DeleteAction(GG::ListBox::iterator it) const {
    return [it, this]() {
        if (OrderIssuingEnabled())
            QueueItemDeletedSignal(it);
    };
}

void QueueListBox::ItemRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys)
{ this->ItemRightClickedImpl(it, pt, modkeys); }

void QueueListBox::ItemRightClickedImpl(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) {
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    bool disabled = !OrderIssuingEnabled();

    popup->AddMenuItem(GG::MenuItem(UserString("MOVE_UP_QUEUE_ITEM"),   disabled, false, MoveToTopAction(it)));
    popup->AddMenuItem(GG::MenuItem(UserString("MOVE_DOWN_QUEUE_ITEM"), disabled, false, MoveToBottomAction(it)));
    popup->AddMenuItem(GG::MenuItem(UserString("DELETE_QUEUE_ITEM"),    disabled, false, DeleteAction(it)));
    popup->Run();
}

void QueueListBox::EnsurePromptHiddenSlot(iterator it) {
    if (m_showing_prompt) {
        Erase(begin(), false, false); // if the prompt is shown, it must be the only row in the ListBox
        m_showing_prompt = false;
    }
}

void QueueListBox::ShowPromptSlot() {
    Insert(GG::Wnd::Create<PromptRow>(Width() - 4, m_prompt_str), begin(), false);
    m_showing_prompt = true;
}

void QueueListBox::ShowPromptConditionallySlot(iterator it) {
    if (begin() == end()) {
        ShowPromptSlot();
    }
}
