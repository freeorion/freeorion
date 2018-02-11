#ifndef GRAPHICALSUMMARY_H
#define GRAPHICALSUMMARY_H

#include "CombatReportData.h"

#include <GG/Wnd.h>

#include <boost/signals2/signal.hpp>


class BarSizer;
class SideBar;
class OptionsBar;

/// Shows a graphical summary of the battle results
class GraphicalSummaryWnd : public GG::Wnd {
public:
    boost::signals2::signal<void()> MinSizeChangedSignal;

    GraphicalSummaryWnd();
    ~GraphicalSummaryWnd();

    /// Get the minimum size of this window required to show all of its
    /// children
    GG::Pt MinUsableSize() const override;

    void Render() override;

    /// Sets the log to show
    void SetLog(int log_id);

    /// Position and sizes things appropriately
    void DoLayout();

private:
    std::vector<std::shared_ptr<SideBar>>   m_side_boxes;
    std::map<int, CombatSummary>            m_summaries;
    std::unique_ptr<BarSizer>               m_sizer;
    std::shared_ptr<OptionsBar>             m_options_bar; // Is a child window->GG handles memory

    void HandleButtonChanged();
    void MakeSummaries(int log_id);
    void DeleteSideBars();
    void GenerateGraph();
};

#endif // GRAPHICALSUMMARY_H
