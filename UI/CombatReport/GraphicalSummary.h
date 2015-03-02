#ifndef GRAPHICALSUMMARY_H
#define GRAPHICALSUMMARY_H

#include "CombatReportData.h"

#include <GG/Wnd.h>

class BarSizer;
class SideBar;
class OptionsBar;

/// Shows a graphical summary of the battle results
class GraphicalSummaryWnd : public GG::Wnd {
public:
    GraphicalSummaryWnd();

    /// Sets the log to show
    void SetLog(int log_id);

    /// Position and sizes things appropriately
    void DoLayout();

    virtual void Render();
private:
    std::vector<SideBar*> m_side_boxes;
    std::map<int, CombatSummary> m_summaries;
    boost::scoped_ptr<BarSizer> m_sizer;
    OptionsBar* m_options_bar; // Is a child window->GG handles memory

    void MakeSummaries(int log_id);

    void DeleteSideBars();

    void GenerateGraph();
};

#endif // GRAPHICALSUMMARY_H
