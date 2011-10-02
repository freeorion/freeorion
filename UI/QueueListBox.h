// -*- C++ -*-
#ifndef _QueueListBox_h_
#define _QueueListBox_h_

#include "CUIControls.h"


/** A list box type for representing queues (eg the research and production queues). */
class QueueListBox :
    public CUIListBox
{
public:
    QueueListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& drop_type_str);

    virtual void DropsAcceptable(DropsAcceptableIter first,
                                 DropsAcceptableIter last,
                                 const GG::Pt& pt) const;
    virtual void AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt);
    virtual void Render();
    virtual void DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds,
                               GG::Flags<GG::ModKey> mod_keys);
    virtual void DragDropHere(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds,
                              GG::Flags<GG::ModKey> mod_keys);
    virtual void DragDropLeave();
    virtual void EnableOrderIssuing(bool enable = true);

    boost::signal<void (GG::ListBox::Row*, std::size_t)> QueueItemMoved;

private:
    iterator    m_drop_point;
    bool        m_show_drop_point;
    bool        m_enabled;
};

#endif
