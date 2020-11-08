#ifndef _ResourcePanel_h_
#define _ResourcePanel_h_

#include "AccordionPanel.h"
#include "../../../common/universe/EnumsFwd.h"

#include <memory>


class MultiIconValueIndicator;
class MultiMeterStatusBar;
class ResourceCenter;
class StatisticIcon;

/** Shows resource meters with meter-bars */
class ResourcePanel : public AccordionPanel {
public:
    ResourcePanel(GG::X w, int object_id);
    ~ResourcePanel();
    void CompleteConstruction() override;

    int ResourceCenterID() const { return m_rescenter_id; }

    void PreRender() override;

    /** expands or collapses panel to show details or just summary info */
    void ExpandCollapse(bool expanded);

    /** updates indicators with values of associated object.  Does not do layout and resizing. */
    void Update();
    /** updates, redoes layout, resizes indicator */
    void Refresh();

protected:
    /** resizes panel and positions widgets */
    void DoLayout() override;

private:
    /** toggles panel expanded or collapsed */
    void ExpandCollapseButtonPressed();

    /** object id for the ResourceCenter that this panel displays */
    int m_rescenter_id;

    /** Icons for the associated meter type. */
    std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>> m_meter_stats;

    /** textually / numerically indicates resource production and construction meter */
    std::shared_ptr<MultiIconValueIndicator> m_multi_icon_value_indicator;
    /** graphically indicates meter values */
    std::shared_ptr<MultiMeterStatusBar> m_multi_meter_status_bar;

    /** map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

#endif
