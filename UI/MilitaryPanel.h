// -*- C++ -*-
#ifndef _MilitaryPanel_h_
#define _MilitaryPanel_h_

#include "AccordionPanel.h"
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

    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);

    /** updates indicators with values of associated object.  Does not do layout and resizing. */
    void Update();
    /** updates, redoes layout, resizes indicator */
    void Refresh();

    /** Enables, or disables if \a enable is false, issuing orders via this panel. */
    void EnableOrderIssuing(bool enable = true);
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

    StatisticIcon* m_fleet_supply_stat;
    StatisticIcon* m_shield_stat;
    StatisticIcon* m_defense_stat;
    StatisticIcon* m_troops_stat;
    StatisticIcon* m_detection_stat;
    StatisticIcon* m_stealth_stat;

    /** textually / numerically indicates resource production and construction meter */
    MultiIconValueIndicator* m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    MultiMeterStatusBar* m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
