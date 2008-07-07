#include "QueueListBox.h"

#include <GG/DrawUtil.h>
#include <GG/WndEvent.h>

#include <boost/cast.hpp>


////////////////////////////////////////////////////////////
// QueueListBox
////////////////////////////////////////////////////////////
QueueListBox::QueueListBox(int x, int y, int w, int h, const std::string& drop_type_str) :
    CUIListBox(x, y, w, h),
    m_drop_point(end()),
    m_show_drop_point(false)
{ AllowDropType(drop_type_str); }

void QueueListBox::DropsAcceptable(DropsAcceptableIter first,
                                   DropsAcceptableIter last,
                                   const GG::Pt& pt) const
{
    assert(std::distance(first, last) == 1);
    for (DropsAcceptableIter it = first; it != last; ++it) {
        it->second =
            AllowedDropTypes().find(it->first->DragDropDataType()) != AllowedDropTypes().end();
    }
}

void QueueListBox::AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt)
{
    assert(wnds.size() == 1);
    assert(AllowedDropTypes().find((*wnds.begin())->DragDropDataType()) != AllowedDropTypes().end());
    GG::ListBox::Row* row = boost::polymorphic_downcast<GG::ListBox::Row*>(*wnds.begin());
    assert(std::find(begin(), end(), row) != end());
    iterator it = RowUnderPt(pt);
    QueueItemMoved(row, std::distance(begin(), it));
}

void QueueListBox::Render()
{
    ListBox::Render();
    if (m_show_drop_point) {
        GG::ListBox::Row& row = **(m_drop_point == end() ? --end() : m_drop_point);
        GG::Control* panel = row[0];
        GG::Pt ul = row.UpperLeft(), lr = row.LowerRight();
        if (m_drop_point == end())
            ul.y = lr.y;
        ul.x = panel->UpperLeft().x;
        lr.x = panel->LowerRight().x;
        GG::FlatRectangle(ul.x, ul.y - 1, lr.x, ul.y, GG::CLR_ZERO, GG::CLR_WHITE, 1);
    }
}

void QueueListBox::DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds,
                                 GG::Flags<GG::ModKey> mod_keys)
{ DragDropHere(pt, drag_drop_wnds, mod_keys); }

void QueueListBox::DragDropHere(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds,
                                GG::Flags<GG::ModKey> mod_keys)
{
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

void QueueListBox::DragDropLeave()
{
    m_drop_point = end();
    m_show_drop_point = false;
}
