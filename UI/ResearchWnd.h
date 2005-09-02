// -*- C++ -*-
#ifndef _ResearchWnd_h_
#define _ResearchWnd_h_

#include "CUI_Wnd.h"
#include "GGListBox.h"

class CUIListBox;
class Tech;
class TechTreeWnd;
class ProductionInfoPanel;

/** Contains a TechTreeWnd, some stats on the empire-wide research queue, and the queue itself. */
class ResearchWnd : public CUI_Wnd
{
public:
    /** \name Structors */ //@{
    ResearchWnd(int w, int h);
    //@}

    /** \name Mutators */ //@{
    virtual GG::Pt ClientUpperLeft() const;
    virtual GG::Pt ClientLowerRight() const;

    void Reset();
    void CenterOnTech(const std::string& tech_name);
    void Sanitize();
    //@}

private:
    void UpdateQueue();
    void ResetInfoPanel();
    void AddTechToQueueSlot(const Tech* tech);
    void QueueItemDeletedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row);
    void QueueItemMovedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row);
    void QueueItemClickedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row, const GG::Pt& pt);
    void QueueItemDoubleClickedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row);

    ProductionInfoPanel* m_research_info_panel;
    CUIListBox*          m_queue_lb;
    TechTreeWnd*         m_tech_tree_wnd;
};

inline std::pair<std::string, std::string> ResearchWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _ResearchWnd_h_
