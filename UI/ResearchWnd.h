// -*- C++ -*-
#ifndef _ResearchWnd_h_
#define _ResearchWnd_h_

#include "CUIWnd.h"

#include <GG/ListBox.h>

class CUIListBox;
class Tech;
class TechTreeWnd;
class ProductionInfoPanel;

/** Contains a TechTreeWnd, some stats on the empire-wide research queue, and the queue itself. */
class ResearchWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    ResearchWnd(int w, int h);
    //@}

    /** \name Mutators */ //@{
    void Reset();
    void CenterOnTech(const std::string& tech_name);
    void QueueItemMoved(int row_idx, GG::ListBox::Row* row);
    void Sanitize();

    void Render();
    //@}

private:
    void UpdateQueue();
    void ResetInfoPanel();  ///< Updates research summary at top left of production screen, and signals that the empire's minerals research pool has changed (propegates to the mapwnd to update indicator)
    void AddTechToQueueSlot(const Tech* tech);
    void AddMultipleTechsToQueueSlot(std::vector<const Tech*> tech_vec);
    void QueueItemDeletedSlot(int row_idx, GG::ListBox::Row* row);
    void QueueItemClickedSlot(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt);
    void QueueItemDoubleClickedSlot(int row_idx, GG::ListBox::Row* row);

    ProductionInfoPanel* m_research_info_panel;
    CUIListBox*          m_queue_lb;
    TechTreeWnd*         m_tech_tree_wnd;
};

#endif // _ResearchWnd_h_
