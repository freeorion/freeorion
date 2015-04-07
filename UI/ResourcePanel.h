// -*- C++ -*-
#ifndef _ResourcePanel_h_
#define _ResourcePanel_h_

#include "AccordionPanel.h"
#include "../universe/TemporaryPtr.h"

class MultiIconValueIndicator;
class MultiMeterStatusBar;
class ResourceCenter;
class StatisticIcon;

/** Shows resource meters with meter-bars */
class ResourcePanel : public AccordionPanel {
public:
    /** \name Structors */ //@{
    ResourcePanel(GG::X w, int object_id);
    ~ResourcePanel();
    //@}

    /** \name Accessors */ //@{
    int ResourceCenterID() const { return m_rescenter_id; }
    //@}

    /** \name Mutators */ //@{
    /** expands or collapses panel to show details or just summary info */
    void ExpandCollapse(bool expanded);

    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);

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

    /** object id for the ResourceCenter that this panel displays */
    int m_rescenter_id;

    /** returns the ResourceCenter object with id m_rescenter_id */
    TemporaryPtr<const ResourceCenter> GetResCenter() const;

    /** icon and number of industry production */
    StatisticIcon* m_industry_stat;
    /** icon and number of research production */
    StatisticIcon* m_research_stat;
    /** icon and number of trade production */
    StatisticIcon* m_trade_stat;
    /** icon and number of infrastructure */
    StatisticIcon* m_construction_stat;

    /** textually / numerically indicates resource production and construction meter */
    MultiIconValueIndicator* m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    MultiMeterStatusBar* m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
