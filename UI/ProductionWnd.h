#ifndef _ProductionWnd_h_
#define _ProductionWnd_h_

#include "CUIWnd.h"
#include "../universe/EnumsFwd.h"
#include "../Empire/Empire.h"

#include <GG/ListBox.h>

class ResourceInfoPanel;
class BuildDesignatorWnd;
class ProductionQueueWnd;

/** Contains a BuildDesignatorWnd, some stats on the empire-wide production queue, and the queue itself. */
class ProductionWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    ProductionWnd(GG::X w, GG::Y h);

    virtual ~ProductionWnd();
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    int SelectedPlanetID() const;
    int ShownEmpireID() const;
    //@}

    /** \name Mutators */ //@{
    bool InWindow(const GG::Pt& pt) const override;
    bool InClient(const GG::Pt& pt) const override;
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Render() override;

    void SetEmpireShown(int empire_id);

    void Refresh();
    void Reset();
    void Update();

    /** Shows \a building_type in production encyclopedia window */
    void ShowBuildingTypeInEncyclopedia(const std::string& building_type);

    /** Shows ShipDesign with id \a design_id in production encyclopedia window */
    void ShowShipDesignInEncyclopedia(int design_id);

    /** Shows \a planet in production encyclopedia window */
    void ShowPlanetInEncyclopedia(int planet_id);

    /** Shows \a tech in production encyclopedia window */
    void ShowTechInEncyclopedia(const std::string& tech_name);

    /** Shows \a ship_part in production encyclopedia window */
    void ShowPartTypeInEncyclopedia(const std::string& part_type_name);

    /** Shows \a species in production encyclopedia window */
    void ShowSpeciesInEncyclopedia(const std::string& species_name);

    /** Shows \a empire in production encyclopedia window */
    void ShowEmpireInEncyclopedia(int empire_id);

    /** Shows \a special in production encyclopedia window */
    void ShowSpecialInEncyclopedia(const std::string& special_name);

    /** Shows \a field_type in production encyclopedia window */
    void ShowFieldTypeInEncyclopedia(const std::string& field_type_name);

    /** Change visibility of production encyclopdia panel */
    void ShowPedia();
    void HidePedia();
    void TogglePedia();

    /** Returns true if pedia is visible */
    bool PediaVisible();


    /** Centres map wnd on location of item on queue with index \a queue_idx
      * and displays info about that item in encyclopedia window.
      * If \a open is true, the location is set as the selected planet. */
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

    void Sanitize();

    /** Enables, or disables if \a enable is false, issuing orders via this ProductionWnd. */
    void EnableOrderIssuing(bool enable = true);
    //@}

    /** emitted when the user changes the selected system in the
      * production screen */
    mutable boost::signals2::signal<void (int)> SystemSelectedSignal;

    /** emitted when the user changes the selected planet in the
      * production screen */
    mutable boost::signals2::signal<void (int)> PlanetSelectedSignal;

    mutable boost::signals2::signal<void (int,int)> RowQuantChangedSignal;

private:
    void    DoLayout();
    void    ProductionQueueChangedSlot();
    void    UpdateQueue();     ///< Clears and repopulates queue list with listitems corresponding to contents of empire's production queue
    void    UpdateInfoPanel(); ///< Updates production summary at top left of production screen, and signals that the empire's minerals resource pool has changed (propagates to the mapwnd to update indicator)

    void    AddBuildToQueueSlot(const ProductionQueue::ProductionItem& item, int number, int location, int pos);

    void    ChangeBuildQuantitySlot(int queue_idx, int quantity);
    void    ChangeBuildQuantityBlockSlot(int queue_idx, int quantity, int blocksize);

    void    DeleteQueueItem(GG::ListBox::iterator it);
    void    QueueItemMoved(const GG::ListBox::iterator& row_it, const GG::ListBox::iterator& original_position_it);
    void    QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    QueueItemDoubleClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    QueueItemRallied(GG::ListBox::iterator it, int object_id);
    void    QueueItemPaused(GG::ListBox::iterator it, bool pause);
    void    QueueItemDuped(GG::ListBox::iterator it);
    void    QueueItemSplit(GG::ListBox::iterator it);
    void    QueueItemUseImperialPP(GG::ListBox::iterator it, bool allow);

    std::shared_ptr<ResourceInfoPanel>  m_production_info_panel;
    std::shared_ptr<ProductionQueueWnd> m_queue_wnd;
    std::shared_ptr<BuildDesignatorWnd> m_build_designator_wnd;

    bool                        m_order_issuing_enabled;
    int                         m_empire_shown_id;
    boost::signals2::connection m_empire_connection;
};

#endif // _ProductionWnd_h_
