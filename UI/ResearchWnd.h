// -*- C++ -*-
#ifndef _ResearchWnd_h_
#define _ResearchWnd_h_

#include "CUIWnd.h"

#include <GG/ListBox.h>

class QueueListBox;
class Tech;
class TechTreeWnd;
class ProductionInfoPanel;

/** Contains a TechTreeWnd, some stats on the empire-wide research queue, and the queue itself. */
class ResearchWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    ResearchWnd(GG::X w, GG::Y h);
    //@}

    /** \name Mutators */ //@{
    void Reset();
    void Update();
    void CenterOnTech(const std::string& tech_name);
    void QueueItemMoved(GG::ListBox::Row* row, std::size_t position);
    void Sanitize();

    void Render();
    //@}

private:
    void UpdateQueue();
    void UpdateInfoPanel();     ///< Updates research summary at top left of production screen, and signals that the empire's minerals research pool has changed (propegates to the mapwnd to update indicator)
    void AddTechToQueueSlot(const Tech* tech);
    void AddMultipleTechsToQueueSlot(const std::vector<const Tech*>& tech_vec);
    void QueueItemDeletedSlot(GG::ListBox::iterator it);
    void QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt);
    void QueueItemDoubleClickedSlot(GG::ListBox::iterator it);

    ProductionInfoPanel* m_research_info_panel;
    QueueListBox*        m_queue_lb;
    TechTreeWnd*         m_tech_tree_wnd;
};

#endif // _ResearchWnd_h_
