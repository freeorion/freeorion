// -*- C++ -*-
#ifndef _MilitaryPanel_h_
#define _MilitaryPanel_h_

#include "AccordionPanel.h"
#include "../universe/Enums.h"
#include "../universe/TemporaryPtr.h"

class MultiIconValueIndicator;
class MultiMeterStatusBar;
class Planet;
class StatisticIcon;

/** Shows military-related meters including stealth, detection, shields, defense; with meter bars */
class MilitaryPanel : public AccordionPanel {
public:
    /** \name Structors */ //@{
    MilitaryPanel(GG::X w, int planet_id);
    ~MilitaryPanel();
    //@}

    /** \name Accessors */ //@{
    int PlanetID() const { return m_planet_id; }
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

    /** object id for the Planet that this panel displays */
    int m_planet_id;

    /** returns the Planet object with id m_planet_id */
    TemporaryPtr<const Planet> GetPlanet() const;

    /** Icons for the associated meter type. */
    std::vector<std::pair<MeterType, StatisticIcon*> > m_meter_stats;

    /** textually / numerically indicates military capabilities */
    MultiIconValueIndicator* m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    MultiMeterStatusBar* m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
