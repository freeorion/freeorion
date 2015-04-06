// -*- C++ -*-
#ifndef _MilitaryPanel_h_
#define _MilitaryPanel_h_

#include "AccordionPanel.h"

class MultiIconValueIndicator;
class MultiMeterStatusBar;
class StatisticIcon;

/** Shows military-related meters including stealth, detection, shields, defense; with meter bars */
class MilitaryPanel : public AccordionPanel {
public:
    /** \name Structors */ //@{
    MilitaryPanel(GG::X w, int planet_id);
    ~MilitaryPanel();
    //@}

    /** \name Accessors */ //@{
    int                     PlanetID() const { return m_planet_id; }
    //@}

    /** \name Mutators */ //@{
    void                    ExpandCollapse(bool expanded);  ///< expands or collapses panel to show details or just summary info

    virtual void            Render();
    virtual void            MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void            SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void                    Update();                       ///< updates indicators with values of associated object.  Does not do layout and resizing.
    void                    Refresh();                      ///< updates, redoes layout, resizes indicator

    /** Enables, or disables if \a enable is false, issuing orders via this MilitaryPanel. */
    void                    EnableOrderIssuing(bool enable = true);
    //@}

private:
    void                    ExpandCollapseButtonPressed();  ///< toggles panel expanded or collapsed
    void                    DoLayout();                     ///< resizes panel and positions widgets

    int                         m_planet_id;                ///< object id for the UniverseObject that this panel display info about

    StatisticIcon*              m_fleet_supply_stat;
    StatisticIcon*              m_shield_stat;
    StatisticIcon*              m_defense_stat;
    StatisticIcon*              m_troops_stat;
    StatisticIcon*              m_detection_stat;
    StatisticIcon*              m_stealth_stat;

    MultiIconValueIndicator*    m_multi_icon_value_indicator;   ///< textually / numerically indicates resource production and construction meter
    MultiMeterStatusBar*        m_multi_meter_status_bar;       ///< graphically indicates meter values

    static std::map<int, bool>  s_expanded_map;             ///< map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false)
};

#endif
