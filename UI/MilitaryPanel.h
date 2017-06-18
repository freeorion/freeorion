#ifndef _MilitaryPanel_h_
#define _MilitaryPanel_h_

#include "AccordionPanel.h"
#include "../universe/EnumsFwd.h"


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
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;

    int PlanetID() const { return m_planet_id; }
    //@}

    /** \name Mutators */ //@{
    void PreRender() override;
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
    void DoLayout() override;
    //@}

private:
    /** toggles panel expanded or collapsed */
    void ExpandCollapseButtonPressed();

    /** object id for the Planet that this panel displays */
    int m_planet_id;

    /** returns the Planet object with id m_planet_id */
    std::shared_ptr<const Planet> GetPlanet() const;

    /** Icons for the associated meter type. */
    std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>> m_meter_stats;

    /** textually / numerically indicates military capabilities */
    std::shared_ptr<MultiIconValueIndicator> m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    std::shared_ptr<MultiMeterStatusBar> m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
