#ifndef _ProductionWnd_h_
#define _ProductionWnd_h_

#include "CUIWnd.h"
#include "../universe/EnumsFwd.h"
#include "../Empire/Empire.h"

#include <GG/ListBox.h>

class ResourceInfoPanel;
class BuildDesignatorWnd;
class ProductionQueueWnd;
struct ScriptingContext;

//! Contains a BuildDesignatorWnd, some stats on the empire-wide production queue, and the queue itself.
class ProductionWnd : public GG::Wnd {
public:
    ProductionWnd(GG::X w, GG::Y h);
    void CompleteConstruction() override;

    int SelectedPlanetID() const noexcept;
    int ShownEmpireID() const noexcept { return m_empire_shown_id; }

    bool InWindow(GG::Pt pt) const noexcept override;
    bool InClient(GG::Pt pt) const noexcept override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Render() override;

    void SetEmpireShown(int empire_id, const ScriptingContext& context);

    void Refresh(const ScriptingContext& context);
    void Reset(const ScriptingContext& context);
    void Update(const ScriptingContext& context);

    void ShowBuildingTypeInEncyclopedia(std::string building_type); //! Shows \a building_type in production encyclopedia window
    void ShowShipDesignInEncyclopedia(int design_id);               //! Shows ShipDesign with id \a design_id in production encyclopedia window
    void ShowPlanetInEncyclopedia(int planet_id);                   //! Shows \a planet in production encyclopedia window
    void ShowTechInEncyclopedia(std::string tech_name);             //! Shows \a tech in production encyclopedia window
    void ShowPolicyInEncyclopedia(std::string policy_name);         //! Shows @a policy_name in production encyclopedia window
    void ShowShipPartInEncyclopedia(std::string part_name);         //! Shows @a part_name in production encyclopedia window
    void ShowSpeciesInEncyclopedia(std::string species_name);       //! Shows \a species in production encyclopedia window
    void ShowEmpireInEncyclopedia(int empire_id);                   //! Shows \a empire in production encyclopedia window
    void ShowSpecialInEncyclopedia(std::string special_name);       //! Shows \a special in production encyclopedia window
    void ShowFieldTypeInEncyclopedia(std::string field_type_name);  //! Shows \a field_type in production encyclopedia window

    //! Change visibility of production encyclopdia panel
    void ShowPedia();
    void HidePedia();
    void TogglePedia();

    //! Returns true if pedia is visible
    bool PediaVisible();


    //! Centres map wnd on location of item on queue with index \a queue_idx
    //! and displays info about that item in encyclopedia window.
    //! If \a open is true, the location is set as the selected planet.
    void CenterOnBuild(int queue_idx, bool open = false);

    //! Programatically sets this Wnd's selected system.
    //! Does not emit a SystemSelectedSignal.
    void SelectSystem(int system_id);

    //! Programatically sets this Wnd's selected planet.
    //! Does not emit a PlanetSelectedSignal.
    void SelectPlanet(int planet_id, const ScriptingContext& context);

    //! Attempts to find a planet to select, and if successful, selects that
    //! planet
    void SelectDefaultPlanet(const ObjectMap& objects);

    void Sanitize(const ObjectMap& objects);

    //! Enables, or disables if \a enable is false, issuing orders via this ProductionWnd.
    void EnableOrderIssuing(bool enable = true);

    //! emitted when the user changes the selected system in the
    //! production screen
    mutable boost::signals2::signal<void (int)> SystemSelectedSignal;

    //! emitted when the user changes the selected planet in the
    //! production screen
    mutable boost::signals2::signal<void (int)> PlanetSelectedSignal;

    mutable boost::signals2::signal<void (int,int)> RowQuantChangedSignal;

private:
    void DoLayout();
    void ProductionQueueChangedSlot();
    void UpdateQueue(const ScriptingContext& context);     ///< Clears and repopulates queue list with listitems corresponding to contents of empire's production queue
    void UpdateInfoPanel(const ScriptingContext& context); ///< Updates production summary at top left of production screen, and signals that the empire's minerals resource pool has changed (propagates to the mapwnd to update indicator)

    void AddBuildToQueueSlot(ProductionQueue::ProductionItem item, int number, int location, int pos);

    void ChangeBuildQuantitySlot(int queue_idx, int quantity) const;
    void ChangeBuildQuantityBlockSlot(int queue_idx, int quantity, int blocksize) const;

    void DeleteQueueItem(GG::ListBox::iterator it, bool do_delete);
    void QueueItemMoved(const GG::ListBox::iterator row_it, const GG::ListBox::iterator original_position_it);
    void QueueItemClickedSlot(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void QueueItemDoubleClickedSlot(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void QueueItemRallied(GG::ListBox::iterator it, int object_id);
    void QueueItemPaused(GG::ListBox::iterator it, bool pause);
    void QueueItemDuped(GG::ListBox::iterator it);
    void QueueItemSplit(GG::ListBox::iterator it);
    void QueueItemUseImperialPP(GG::ListBox::iterator it, bool allow);

    std::shared_ptr<ResourceInfoPanel>  m_production_info_panel;
    std::shared_ptr<ProductionQueueWnd> m_queue_wnd;
    std::shared_ptr<BuildDesignatorWnd> m_build_designator_wnd;
    boost::signals2::scoped_connection  m_empire_connection;
    int                                 m_empire_shown_id = ALL_EMPIRES;
    bool                                m_order_issuing_enabled = false;
};


#endif
