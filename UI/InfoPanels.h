#ifndef _InfoPanels_h_
#define _InfoPanels_h_

#include "../universe/Enums.h"

#include <GG/Button.h>

class PopulationPanel;
class ResourcePanel;
class BuildingsPanel;
class BuildingIndicator;
class MultiTurnProgressBar;
class MeterStatusBar2;
class Meter;
class Planet;
class ResourceCenter;
class PopCenter;
class UniverseObject;
class BuildingType;
class StatisticIcon;
class CUIDropDownList;
namespace GG {
    class StaticGraphic;
}

class PopulationPanel : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ExpandCollapseSignalType;    ///< emitted when the panel is expanded or collapsed, so that container can reposition other panels whose location depends on this one's size
    //@}

    /** \name Structors */ //@{
    PopulationPanel(int w, const UniverseObject &obj); ///< basic ctor
    ~PopulationPanel();
    //@}

    /** \name Accessors */ //@{
    int PopCenterID() const {return m_popcenter_id;}
    //@}

    /** \name Mutators */ //@{
    void ExpandCollapse(bool expanded);         ///< expands or collapses panel to show details or just summary info

    virtual void Render();
    //virtual void LClick(const GG::Pt& pt, Uint32 keys);
    //virtual void RClick(const GG::Pt& pt, Uint32 keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    
    void Update();          ///< refreshes / updates indicators

    mutable ExpandCollapseSignalType ExpandCollapseSignal;
    //@}

private:
    void ExpandCollapseButtonPressed();         ///< toggles panel expanded or collapsed

    void DoExpandCollapseLayout();              ///< resizes panel and positions widgets according to present collapsed / expanded status

    PopCenter*          GetPopCenter();         ///< returns the planet with ID m_planet_id
    const PopCenter*    GetPopCenter() const;

    int                     m_popcenter_id;     ///< object id for the UniverseObject that is also a PopCenter which is being displayed in this panel

    StatisticIcon*          m_pop_stat;         ///< icon and number of population
    StatisticIcon*          m_health_stat;      ///< icon and number of health

    MeterStatusBar2*         m_pop_meter_bar;    ///< graphically indicates status of population
    MeterStatusBar2*         m_health_meter_bar; ///< graphically indicates health
    
    GG::Button*             m_expand_button;    ///< at top right of panel, toggles the panel open/closed to show details or minimal summary

    static std::map<int, bool> s_expanded_map;  ///< map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false)
    
    boost::signals::connection m_connection_system_changed;     ///< stores connection used to handle a system change
    boost::signals::connection m_connection_popcenter_changed;  ///< stores connection used to handle a popcenter change
};

class ResourcePanel : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()>          ExpandCollapseSignalType;   ///< emitted when the panel is expanded or collapsed, so that container can reposition other panels whose location depends on this one's size
    typedef boost::signal<void (FocusType)> FocusChangedSignalType;     ///< emitted when either the primary or secondary focus changes
    //@}


    /** \name Structors */ //@{
    ResourcePanel(int w, const UniverseObject &obj);
    ~ResourcePanel();
    //@}

    /** \name Accessors */ //@{
    int ResourceCenterID() const {return m_rescenter_id;}
    //@}

    /** \name Mutators */ //@{
    void ExpandCollapse(bool expanded);         ///< expands or collapses panel to show details or just summary info

    virtual void Render();
    //virtual void LClick(const GG::Pt& pt, Uint32 keys);
    //virtual void RClick(const GG::Pt& pt, Uint32 keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    
    void Update();          ///< refreshes / updates indicators

    mutable ExpandCollapseSignalType ExpandCollapseSignal;
    mutable FocusChangedSignalType PrimaryFocusChangedSignal;
    mutable FocusChangedSignalType SecondaryFocusChangedSignal;
    //@}

private:
    void ExpandCollapseButtonPressed();         ///< toggles panel expanded or collapsed

    void DoExpandCollapseLayout();              ///< resizes panel and positions widgets according to present collapsed / expanded status

    void PrimaryFocusDropListSelectionChanged(int selected);    ///< called when droplist selection changes, emits PrimaryFocusChangedSignal
    void SecondaryFocusDropListSelectionChanged(int selected);  ///< called when droplist selection changes, emits SecondaryFocusChangedSignal
    
    ResourceCenter*        GetResourceCenter(); ///< returns the planet with ID m_planet_id
    const ResourceCenter*  GetResourceCenter() const;

    int                     m_rescenter_id;     ///< object id for the UniverseObject that is also a PopCenter which is being displayed in this panel

    StatisticIcon*          m_farming_stat;         ///< icon and number of food production
    StatisticIcon*          m_mining_stat;          ///< icon and number of minerals production
    StatisticIcon*          m_industry_stat;        ///< icon and number of industry production
    StatisticIcon*          m_research_stat;        ///< icon and number of research production
    StatisticIcon*          m_trade_stat;           ///< icon and number of trade production
    StatisticIcon*          m_construction_stat;    ///< icon and number of construction meter

    MeterStatusBar2*         m_farming_meter_bar;       ///< graphically indicates farming meter
    MeterStatusBar2*         m_mining_meter_bar;        ///< graphically indicates mining meter
    MeterStatusBar2*         m_industry_meter_bar;      ///< graphically indicates industry meter
    MeterStatusBar2*         m_research_meter_bar;      ///< graphically indicates research meter
    MeterStatusBar2*         m_trade_meter_bar;         ///< graphically indicates trade meter
    MeterStatusBar2*         m_construction_meter_bar;  ///< graphically indicates construction meter

    CUIDropDownList*        m_primary_focus_drop;   ///< displays and allows selection of primary focus
    CUIDropDownList*        m_secondary_focus_drop; ///< displays and allows selection of secondary focus

    GG::Button*             m_expand_button;    ///< at top right of panel, toggles the panel open/closed to show details or minimal summary

    static std::map<int, bool> s_expanded_map;  ///< map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false)
    
    boost::signals::connection m_connection_system_changed;     ///< stores connection used to handle a system change
    boost::signals::connection m_connection_rescenter_changed;  ///< stores connection used to handle a popcenter change
};

class BuildingsPanel : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ExpandCollapseSignalType;    ///< emitted when the panel is expanded or collapsed, so that container can reposition other panels whose location depends on this one's size
    //@}

    /** \name Structors */ //@{
    BuildingsPanel(int w, int columns, const Planet &plt);   ///< basic ctor
    ~BuildingsPanel();
    //@}

    /** \name Accessors */ //@{
    int PlanetID() const {return m_planet_id;}
    //@}

    /** \name Mutators */ //@{
    void ExpandCollapse(bool expanded);         ///< expands or collapses panel to show details or just summary info

    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    
    void Update();          ///< regenerates indicators according to buildings on planets and on queue on planet and redoes layout
    void Refresh();         ///< Updates, redraws and signals expand / collapse

    mutable ExpandCollapseSignalType ExpandCollapseSignal;
    //@}

private:
    void ExpandCollapseButtonPressed();         ///< toggles panel expanded or collapsed

    void DoExpandCollapseLayout();              ///< resizes panel and positions indicators, differently depending on collapsed / expanded status

    Planet*                 GetPlanet();        ///< returns the planet with ID m_planet_id
    const Planet*           GetPlanet() const;

    int                     m_planet_id;        ///< object id for the Planet whose buildings this panel displays
    int                     m_columns;          ///< number of columns in which to display building indicators

    std::vector<BuildingIndicator*> m_building_indicators;

    GG::Button*             m_expand_button;    ///< at top right of panel, toggles the panel open/closed to show details or minimal summary

    static std::map<int, bool> s_expanded_map;  ///< map indexed by planet ID indicating whether the BuildingsPanel for each object is expanded (true) or collapsed (false)
    
    boost::signals::connection m_connection_system_changed;     ///< stores connection used to handle a system change
    boost::signals::connection m_connection_planet_changed;     ///< stores connection used to handle a planet change
};

/** Represents and allows some user interaction with a building */
class BuildingIndicator : public GG::Wnd
{
public:
    BuildingIndicator(int w, const BuildingType &type); ///< constructor for use when building is completed, shown without progress bar
    BuildingIndicator(int w, const BuildingType &type, int turns_left, int turns_completed,
                      double partial_turn);             ///< constructor for use when building is partially complete, to show progress bar
    
    virtual void Render();

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)


private:
    const BuildingType& m_type;

    GG::StaticGraphic*      m_graphic;
    MultiTurnProgressBar*   m_progress_bar;
};

/** Graphically represents the current, max and (in future: projected) changes to values of a Meter as a
    horizontal bar */
class MeterStatusBar2 : public GG::Wnd
{
public:
    MeterStatusBar2(int w, int h, const Meter& meter);

    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)

    void SetProjectedCurrent(double current);
    void SetProjectedMax(double max);

private:
    const Meter& m_meter;
    double m_initial_max;
    double m_initial_current;
    double m_projected_max;
    double m_projected_current;
};

#endif

