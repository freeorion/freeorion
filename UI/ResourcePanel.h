// -*- C++ -*-
#ifndef _ResourcePanel_h_
#define _ResourcePanel_h_

#include "AccordionPanel.h"
#include "../universe/Enums.h"
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

    /** updates indicators with values of associated object.  Does not do layout and resizing. */
    void Update();
    /** updates, redoes layout, resizes indicator */
    void Refresh();
    //@}

protected:
    /** \name Mutators */ //@{
    /** resizes panel and positions widgets */
    virtual void DoLayout();
    //@}

private:
    /** toggles panel expanded or collapsed */
    void ExpandCollapseButtonPressed();

    /** object id for the ResourceCenter that this panel displays */
    int m_rescenter_id;

    /** returns the ResourceCenter object with id m_rescenter_id */
    TemporaryPtr<const ResourceCenter> GetResCenter() const;

    /** Icons for the associated meter type. */
    std::vector<std::pair<MeterType, StatisticIcon*> > m_meter_stats;

    /** textually / numerically indicates resource production and construction meter */
    MultiIconValueIndicator* m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    MultiMeterStatusBar* m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
