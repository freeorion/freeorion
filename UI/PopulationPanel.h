// -*- C++ -*-
#ifndef _PopulationPanel_h_
#define _PopulationPanel_h_

#include "AccordionPanel.h"
#include "../universe/Enums.h"
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

    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event);

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

    /** object id for the PopulationCenter that this panel displays */
    int m_popcenter_id;

    /** returns the PopCenter object with id m_popcenter_id */
    TemporaryPtr<const PopCenter> GetPopCenter() const;

    /** Icons for the associated meter type. */
    std::vector<std::pair<MeterType, StatisticIcon*> > m_meter_stats;

    /** textually / numerically indicates population */
    MultiIconValueIndicator* m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    MultiMeterStatusBar* m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
