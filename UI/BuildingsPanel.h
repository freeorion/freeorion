#ifndef _BuildingsPanel_h_
#define _BuildingsPanel_h_

#include "AccordionPanel.h"
#include "CUIDrawUtil.h"
#include "../universe/ConstantsFwd.h"

class BuildingIndicator;
class MultiTurnProgressBar;
class ScanlineControl;

/** Contains various BuildingIndicator to represent buildings on a planet. */
class BuildingsPanel final : public AccordionPanel {
public:
    BuildingsPanel(GG::X w, int columns, int planet_id);
    ~BuildingsPanel() = default;
    void CompleteConstruction() override;

    [[nodiscard]] int PlanetID() const noexcept { return m_planet_id; }

    void PreRender() override;

    /** updates, redoes layout, resizes indicator */
    void Refresh();

    /** expands or collapses panel to show details or just summary info */
    void ExpandCollapse(bool expanded);

    /** Enables, or disables if \a enable is false, issuing orders via this panel. */
    void EnableOrderIssuing(bool enable = true);

    mutable boost::signals2::signal<void (int)> BuildingRightClickedSignal;

protected:
    /** resizes panel and positions widgets */
    void DoLayout() override;

    /** updates indicators with values of associated object.  Does not do layout and resizing. */
    void Update();
    void RefreshImpl();

private:
    /** toggles panel expanded or collapsed */
    void ExpandCollapseButtonPressed();

    int m_planet_id = INVALID_OBJECT_ID; // object id for the Planet whose buildings this panel displays
    int m_columns = 1; // number of columns in which to display building indicators
    std::vector<std::shared_ptr<BuildingIndicator>> m_building_indicators;
    boost::signals2::scoped_connection m_queue_connection;

    /** map indexed by planet ID indicating whether the BuildingsPanel for each
      * object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

/** Represents and allows some user interaction with a building */
class BuildingIndicator final : public GG::Wnd {
public:
    /** Constructor for use when building is completed, shown without progress 
      * bar. */
    BuildingIndicator(GG::X w, int building_id);
    /** Constructor for use when building is partially complete, to show
      * progress bar. */
    BuildingIndicator(GG::X w, const std::string& building_type,
                      double turns_completed, double total_turns, double total_cost, double turn_spending);

    void CompleteConstruction() override;

    void PreRender() override;
    void Render() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;

    /** Enables, or disables if \a enable is false, issuing orders via this BuildingIndicator. */
    void EnableOrderIssuing(bool enable = true);

    mutable boost::signals2::signal<void (int)> RightClickedSignal;

private:
    void Refresh();
    void DoLayout();

    std::shared_ptr<GG::StaticGraphic>      m_graphic;
    std::shared_ptr<ScanlineControl>        m_scanlines;
    std::shared_ptr<GG::StaticGraphic>      m_scrap_indicator; ///< shown to indicate building was ordered scrapped
    std::shared_ptr<MultiTurnProgressBar>   m_progress_bar;
    boost::signals2::scoped_connection      m_signal_connection;
    int                                     m_building_id = INVALID_OBJECT_ID;
    bool                                    m_order_issuing_enabled = true;
};

#endif
