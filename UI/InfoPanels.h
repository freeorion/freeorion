// -*- C++ -*-
#ifndef _InfoPanels_h_
#define _InfoPanels_h_

#include "../universe/Enums.h"

#include <GG/Button.h>

class PopulationPanel;
class ResourcePanel;
class BuildingsPanel;
class BuildingIndicator;
class SpecialsPanel;
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

    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    
    void Update();          ///< updates indicators with values of associated object.  Does not do layout and resizing.
    void Refresh();         ///< updates, redoes layout, resizes indicator

    mutable ExpandCollapseSignalType ExpandCollapseSignal;
    //@}

private:
    void ExpandCollapseButtonPressed();         ///< toggles panel expanded or collapsed

    void DoExpandCollapseLayout();              ///< resizes panel and positions widgets according to present collapsed / expanded status

    PopCenter*          GetPopCenter();         ///< returns the planet with ID m_planet_id
    const PopCenter*    GetPopCenter() const;

    int                 m_popcenter_id;         ///< object id for the UniverseObject that is also a PopCenter which is being displayed in this panel

    StatisticIcon*      m_pop_stat;             ///< icon and number of population
    StatisticIcon*      m_health_stat;          ///< icon and number of health

    MeterStatusBar2*    m_pop_meter_bar;        ///< graphically indicates status of population
    MeterStatusBar2*    m_health_meter_bar;     ///< graphically indicates health
    
    GG::Button*         m_expand_button;        ///< at top right of panel, toggles the panel open/closed to show details or minimal summary

    std::set<boost::signals::connection> m_misc_connections;

    static std::map<int, bool> s_expanded_map;  ///< map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false)
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
    void ExpandCollapse(bool expanded); ///< expands or collapses panel to show details or just summary info

    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    
    void Update();  ///< updates indicators with values of associated object.  Does not do layout and resizing.
    void Refresh(); ///< updates, redoes layout, resizes indicator

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

    std::set<boost::signals::connection> m_misc_connections;

    static std::map<int, bool> s_expanded_map;  ///< map indexed by popcenter ID indicating whether the PopulationPanel for each object is expanded (true) or collapsed (false)
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
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    
    void Refresh();                             ///< recreates indicators, redoes layout, resizes

    mutable ExpandCollapseSignalType ExpandCollapseSignal;
    //@}

private:
    void ExpandCollapseButtonPressed();         ///< toggles panel expanded or collapsed

    void DoExpandCollapseLayout();              ///< resizes panel and positions indicators, differently depending on collapsed / expanded status

    void Update();                              ///< recreates building indicators for building on or being built at this planet

    Planet*                 GetPlanet();        ///< returns the planet with ID m_planet_id
    const Planet*           GetPlanet() const;

    int                     m_planet_id;        ///< object id for the Planet whose buildings this panel displays
    int                     m_columns;          ///< number of columns in which to display building indicators

    std::vector<BuildingIndicator*> m_building_indicators;

    GG::Button*             m_expand_button;    ///< at top right of panel, toggles the panel open/closed to show details or minimal summary

    std::set<boost::signals::connection> m_misc_connections;

    static std::map<int, bool> s_expanded_map;  ///< map indexed by planet ID indicating whether the BuildingsPanel for each object is expanded (true) or collapsed (false)
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
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)


private:
    const BuildingType& m_type;

    GG::StaticGraphic*      m_graphic;
    MultiTurnProgressBar*   m_progress_bar;
};

class SpecialsPanel : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    SpecialsPanel(int w, const UniverseObject &obj);   ///< basic ctor
    ~SpecialsPanel();
    //@}

    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const;
    int ObjectID() const {return m_object_id;}
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    
    void Update();          ///< regenerates indicators according to buildings on planets and on queue on planet and redoes layout
    //@}

private:
    
    UniverseObject*         GetObject();        ///< returns the object with ID m_object_id
    const UniverseObject*   GetObject() const;

    int                     m_object_id;        ///< id for the Object whose specials this panel displays

    std::vector<GG::StaticGraphic*> m_icons;
};

/** Graphically represents the current, max and (in future: projected) changes to values of a Meter as a
    horizontal bar */
class MeterStatusBar2 : public GG::Wnd
{
public:
    MeterStatusBar2(int w, int h, const Meter& meter);

    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)

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

