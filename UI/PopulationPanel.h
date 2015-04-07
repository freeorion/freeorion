// -*- C++ -*-
#ifndef _PopulationPanel_h_
#define _PopulationPanel_h_

#include "AccordionPanel.h"
#include "../universe/TemporaryPtr.h"

class MultiIconValueIndicator;
class MultiMeterStatusBar;
class PopCenter;
class StatisticIcon;

/** Shows population with meter bars */
class PopulationPanel : public AccordionPanel {
public:
    /** \name Structors */ //@{
    PopulationPanel(GG::X w, int object_id);
    ~PopulationPanel();
    //@}

    /** \name Accessors */ //@{
    int PopCenterID() const { return m_popcenter_id; }
    //@}

    /** \name Mutators */ //@{
    /** expands or collapses panel to show details or just summary info */
    void ExpandCollapse(bool expanded);

    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event);

    /** updates indicators with values of associated object.  Does not do layout and resizing. */
    void Update();
    /** updates, redoes layout, resizes indicator */
    void Refresh();

    /** Enables, or disables if \a enable is false, issuing orders via this panel. */
    void EnableOrderIssuing(bool enable = true);
    //@}

private:
    /** toggles panel expanded or collapsed */
    void ExpandCollapseButtonPressed();
    /** resizes panel and positions widgets */
    void DoLayout();

    /** object id for the PopulationCenter that this panel displays */
    int m_popcenter_id;

    /** returns the PopCenter object with id m_popcenter_id */
    TemporaryPtr<const PopCenter> GetPopCenter() const;

    /** icon and number of population */
    StatisticIcon* m_pop_stat;
    /** icon and number of happiness */
    StatisticIcon* m_happiness_stat;

    /** textually / numerically indicates population */
    MultiIconValueIndicator* m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    MultiMeterStatusBar* m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
