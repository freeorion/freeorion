// -*- C++ -*-
#ifndef _BuildingsPanel_h_
#define _BuildingsPanel_h_

#include "AccordionPanel.h"

class BuildingIndicator;
class MultiTurnProgressBar;
class ShaderProgram;

/** Contains various BuildingIndicator to represent buildings on a planet. */
class BuildingsPanel : public AccordionPanel {
public:
    /** \name Structors */ //@{
    BuildingsPanel(GG::X w, int columns, int planet_id);
    ~BuildingsPanel();
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

    /** Enables, or disables if \a enable is false, issuing orders via this panel. */
    void EnableOrderIssuing(bool enable = true);
    //@}

    mutable boost::signals2::signal<void (int)> BuildingRightClickedSignal;

protected:
    /** \name Mutators */ //@{
    /** resizes panel and positions widgets */
    virtual void DoLayout();
    //@}

private:
    /** toggles panel expanded or collapsed */
    void ExpandCollapseButtonPressed();

    /** object id for the Planet whose buildings this panel displays */
    int m_planet_id;

    /** number of columns in which to display building indicators */
    int m_columns;
    std::vector<BuildingIndicator*> m_building_indicators;

    /** map indexed by planet ID indicating whether the BuildingsPanel for each object is expanded (true) or collapsed (false) */
    static std::map<int, bool> s_expanded_map;
};

/** Represents and allows some user interaction with a building */
class BuildingIndicator : public GG::Wnd {
public:
    /** Constructor for use when building is completed, shown without progress 
      * bar. */
    BuildingIndicator(GG::X w, int building_id);
    /** Constructor for use when building is partially complete, to show
      * progress bar. */
    BuildingIndicator(GG::X w, const std::string& building_type,
                      double turns_completed, double total_turns);

    /** \name Mutators */ //@{
    virtual void    Render();
    void            Refresh();

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);

    /** Enables, or disables if \a enable is false, issuing orders via this BuildingIndicator. */
    void            EnableOrderIssuing(bool enable = true);
    //@}

    mutable boost::signals2::signal<void (int)> RightClickedSignal;

private:
    void            DoLayout();

    static boost::shared_ptr<ShaderProgram> s_scanline_shader;

    GG::StaticGraphic*          m_graphic;
    GG::StaticGraphic*          m_scrap_indicator;  ///< shown to indicate building was ordered scrapped
    MultiTurnProgressBar*       m_progress_bar;
    int                         m_building_id;
    bool                        m_order_issuing_enabled;
};

#endif
