#ifndef _BuildDesignatorWnd_h_
#define _BuildDesignatorWnd_h_

#include "../universe/EnumsFwd.h"
#include "../Empire/Empire.h"

#include <GG/Wnd.h>


class SidePanel;
class EncyclopediaDetailPanel;

class BuildDesignatorWnd : public GG::Wnd {
public:
    static const std::string PRODUCTION_ITEM_DROP_TYPE;

    /** \name Structors */ //@{
    BuildDesignatorWnd(GG::X w, GG::Y h);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const override;
    bool InClient(const GG::Pt& pt) const override;

    /** returns set of BulldType shown in this selector */
    const std::set<BuildType>&      GetBuildTypesShown() const;

    /** .first -> available items; .second -> unavailable items */
    const std::pair<bool, bool>&    GetAvailabilitiesShown() const;

    int SelectedPlanetID() const;
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    /** Centres map wnd on location of item on queue with index \a queue_idx
      * and displays info about that item in encyclopedia window
      * If \a open is true, the sidepanel for that planet is opened and 
      * it is set as the selected location (so items queued will be built there). */
    void CenterOnBuild(int queue_idx, bool open = false);

    /** Programatically sets this Wnd's selected system.
      * Does not emit a SystemSelectedSignal. */
    void SelectSystem(int system_id);

    /** Programatically sets this Wnd's selected planet.
      * Does not emit a PlanetSelectedSignal. */
    void SelectPlanet(int planet_id);

    /** Attempts to find a planet to select, and if successful, selects that
      * planet */
    void SelectDefaultPlanet();

    /** Sets BuildSelector's empire ID to empire of this client, and then calls
      * Update. */
    void Refresh();

    /** Updates sidepanels and refreshes encyclopedia and build selector. */
    void Update();

    /** Sets sidepanel to no system, build location to no planet, refreshes
      * build selector and sets encyclopedia to show nothing, and resets shown
      * types and availabilities to default. */
    void Reset();

    /** Resets, and also clears default planet selections for each system. */
    void Clear();

    /** Show or hide indicated types of buildable items */
    void ShowType(BuildType type, bool refresh_list = true);
    void ShowAllTypes(bool refresh_list = true);
    void HideType(BuildType type, bool refresh_list = true);
    void HideAllTypes(bool refresh_list = true);
    void ToggleType(BuildType type, bool refresh_list = true);
    void ToggleAllTypes(bool refresh_list = true);

    /** Show or hide indicated availabilities of buildable items.  Available
      * items are those which have been unlocked for this selector's emipre. */
    void ShowAvailability(bool available, bool refresh_list = true);
    void HideAvailability(bool available, bool refresh_list = true);
    void ToggleAvailabilitly(bool available, bool refresh_list = true);

    /** Open a pedia entry on the encyclopedia */
    void ShowBuildingTypeInEncyclopedia(const std::string& building_type);
    void ShowShipDesignInEncyclopedia(int design_id);
    void ShowPlanetInEncyclopedia(int planet_id);
    void ShowTechInEncyclopedia(const std::string& tech_name);
    void ShowPartTypeInEncyclopedia(const std::string& part_type_name);
    void ShowSpeciesInEncyclopedia(const std::string& species_name);
    void ShowEmpireInEncyclopedia(int empire_id);
    void ShowSpecialInEncyclopedia(const std::string& special_name);
    void ShowFieldTypeInEncyclopedia(const std::string& field_type_name);

    /** Show or hide the encyclopedia detail panel */
    void ShowPedia();
    void HidePedia();
    void TogglePedia();

    /** returns true if the encyclopedia detail panel is visible */
    bool PediaVisible();
    //@}

    /** emitted when the indicated build is indicated by the user */
    mutable boost::signals2::signal<void (const ProductionQueue::ProductionItem&, int, int, int)>
                                                        AddBuildToQueueSignal;
    /** emitted when the quantity of items in a single build queue item is
      * changed by the user */
    mutable boost::signals2::signal<void (int, int)>    BuildQuantityChangedSignal;
    /** emitted when the user selects a system from within this Wnd (but not
      * when this Wnd's system is set programatically) */
    mutable boost::signals2::signal<void (int)>         SystemSelectedSignal;
    /** emitted when the user changes the planet selection from within this
      * Wnd (but not when this Wnd's selected planet is set programatically) */
    mutable boost::signals2::signal<void (int)>         PlanetSelectedSignal;

private:
    class BuildSelector;

    int BuildLocation() const;
    void BuildItemRequested(const ProductionQueue::ProductionItem& item, int num_to_build, int pos);
    void BuildQuantityChanged(int queue_idx, int quantity);
    void SetBuild(int queue_idx);
    void InitializeWindows();

    std::shared_ptr<EncyclopediaDetailPanel>    m_enc_detail_panel;
    std::shared_ptr<BuildSelector>              m_build_selector;
    std::shared_ptr<SidePanel>                  m_side_panel;
    std::map<int, int>                          m_system_default_planets;   //!< map from system id to id of planet to auto select when viewing each system
};

#endif // _BuildDesignatorWnd_h_
