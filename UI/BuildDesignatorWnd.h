// -*- C++ -*-
#ifndef _BuildDesignatorWnd_h_
#define _BuildDesignatorWnd_h_

#include "../universe/Enums.h"

#include <GG/Wnd.h>


class SidePanel;
class EncyclopediaDetailPanel;

class BuildDesignatorWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    BuildDesignatorWnd(GG::X w, GG::Y h);
    //@}

    /** \name Accessors */ //@{
    /** returns set of BulldType shown in this selector */
    const std::set<BuildType>&      GetBuildTypesShown() const;

    /** .first -> available items; .second -> unavailable items */
    const std::pair<bool, bool>&    GetAvailabilitiesShown() const;

    virtual bool    InWindow(const GG::Pt& pt) const;
    virtual bool    InClient(const GG::Pt& pt) const;

    GG::Rect        MapViewHole() const;
    //@}

    /** \name Mutators */ //@{
    /** Centres map wnd on location of item on queue with index \a queue_idx
      * and displays info about that item in encyclopedia window. */
    void            CenterOnBuild(int queue_idx);

    /** Programatically sets this Wnd's selected system.
      * Does not emit a SystemSelectedSignal. */
    void            SelectSystem(int system_id);

    /** Programatically sets this Wnd's selected planet.
      * Does not emit a PlanetSelectedSignal. */
    void            SelectPlanet(int planet_id);

    /** Attempts to find a planet to select, and if successful, selects that
      * planet */
    void            SelectDefaultPlanet();

    /** Updates sidepanels and refreshes encyclopedia and build selector. */
    void            Update();

    /** Sets sidepanel to no system, build location to no planet, refreshes
      * build selector and sets encyclopedia to show nothing, and resets shown
      * types and availabilities to default. */
    void            Reset();

    /** Resets, and also clears default planet selections for each system. */
    void            Clear();

    /** Show or hide indicated types of buildable items */
    void            ShowType(BuildType type, bool refresh_list = true);
    void            ShowAllTypes(bool refresh_list = true);
    void            HideType(BuildType type, bool refresh_list = true);
    void            HideAllTypes(bool refresh_list = true);
    void            ToggleType(BuildType type, bool refresh_list = true);
    void            ToggleAllTypes(bool refresh_list = true);

    /** Show or hide indicated availabilities of buildable items.  Available
      * items are those which have been unlocked for this selector's emipre. */
    void            ShowAvailability(bool available, bool refresh_list = true);
    void            HideAvailability(bool available, bool refresh_list = true);
    void            ToggleAvailabilitly(bool available, bool refresh_list = true);
    //@}

    mutable boost::signal<void (BuildType, const std::string&, int, int)>
                                            AddNamedBuildToQueueSignal; ///< emitted when the indicated named build is indicated by the user
    mutable boost::signal<void (BuildType, int, int, int)>
                                            AddIDedBuildToQueueSignal;  ///< emitted when the indicated id'd build is indicated by the user
    mutable boost::signal<void (int, int)>  BuildQuantityChangedSignal; ///< emitted when the quantity of items in a single build queue item is changed by the user
    mutable boost::signal<void (int)>       SystemSelectedSignal;       ///< emitted when the user selects a system from within this Wnd (but not when this Wnd's system is set programatically)
    mutable boost::signal<void (int)>       PlanetSelectedSignal;       ///< emitted when the user changes the planet selection from within this Wnd (but not when this Wnd's selected planet is set programatically)

private:
    class BuildSelector;
    int             BuildLocation() const;

    void            BuildItemRequested(BuildType build_type, const std::string& item, int num_to_build);
    void            BuildItemRequested(BuildType build_type, int design_id, int num_to_build);
    void            BuildQuantityChanged(int queue_idx, int quantity);
    void            SetBuild(int queue_idx);

    EncyclopediaDetailPanel*    m_enc_detail_panel;
    BuildSelector*              m_build_selector;
    SidePanel*                  m_side_panel;
    GG::Rect                    m_map_view_hole;
    std::map<int, int>          m_system_default_planets;   //!< map from system id to id of planet to auto select when viewing each system
};

#endif // _BuildDesignatorWnd_h_
