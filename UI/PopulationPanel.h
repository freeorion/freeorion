#ifndef _PopulationPanel_h_
#define _PopulationPanel_h_

#include "AccordionPanel.h"
#include "../universe/EnumsFwd.h"

#include <memory>


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
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    int PopCenterID() const { return m_popcenter_id; }
    //@}

    /** \name Mutators */ //@{
    void PreRender() override;

    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;

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

    /** object id for the PopulationCenter that this panel displays */
    int m_popcenter_id;

    /** returns the PopCenter object with id m_popcenter_id */
    std::shared_ptr<const PopCenter> GetPopCenter() const;

    /** Icons for the associated meter type. */
    std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>> m_meter_stats;

    /** textually / numerically indicates population */
    std::shared_ptr<MultiIconValueIndicator> m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    std::shared_ptr<MultiMeterStatusBar> m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
