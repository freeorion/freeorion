#ifndef _FleetWnd_h_
#define _FleetWnd_h_

#include <GG/GGFwd.h>
#include <GG/ListBox.h>

#include "CUIWnd.h"
#include "MapWnd.h"

class Fleet;
class FleetDataPanel;
class FleetDetailPanel;
class FleetsListBox;
class Ship;
class System;
class UniverseObject;
class StatisticIcon;
class ScanlineControl;


/* the new fleet aggression settings */
GG_ENUM(NewFleetAggression,
    INVALID_FLEET_AGGRESSION = -1,
    FLEET_AGGRESSIVE,
    FLEET_PASSIVE
)

/** Manages the lifetimes of FleetWnds. */
class FleetUIManager {
public:
    typedef std::set<std::weak_ptr<FleetWnd>>::const_iterator iterator;

    //! \name Accessors //@{
    bool            empty() const;
    iterator        begin() const;
    iterator        end() const;
    FleetWnd*       ActiveFleetWnd() const;
    std::shared_ptr<FleetWnd> WndForFleetID(int fleet_id) const;
    std::shared_ptr<FleetWnd> WndForFleetIDs(const std::vector<int>& fleet_ids_) const;
    int             SelectedShipID() const;     // if a single ship is selected in the active fleetwnd, returns that ship's ID.  Otherwise, returns INVALID_OBJECT_ID
    std::set<int>   SelectedShipIDs() const;    // returns the ids of all selected ships in the active fleetwnd
    //@}

    //! \name Mutators //@{
    std::shared_ptr<FleetWnd> NewFleetWnd(const std::vector<int>& fleet_ids,
                                          double allowed_bounding_box_leeway = 0,
                                          int selected_fleet_id = INVALID_OBJECT_ID,
                                          GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE | GG::RESIZABLE);

    void            CullEmptyWnds();
    void            SetActiveFleetWnd(std::shared_ptr<FleetWnd> fleet_wnd);
    bool            CloseAll();
    void            RefreshAll();

    /** Enables, or disables if \a enable is false, issuing orders via FleetWnds. */
    void            EnableOrderIssuing(bool enable = true);
    //@}

    /** emitted when the selected FleetWnd changes */
    mutable boost::signals2::signal<void ()> ActiveFleetWndChangedSignal;

    /** emitted when the selected fleets in the active FleetWnd change */
    mutable boost::signals2::signal<void ()> ActiveFleetWndSelectedFleetsChangedSignal;

    /** emitted when the selected ships in the active FleetWnd change */
    mutable boost::signals2::signal<void ()> ActiveFleetWndSelectedShipsChangedSignal;

    /** emitted when a fleet is right-clicked */
    mutable boost::signals2::signal<void (int)> FleetRightClickedSignal;

    /** emitted when a ship is right-clicked */
    mutable boost::signals2::signal<void (int)> ShipRightClickedSignal;

    static FleetUIManager& GetFleetUIManager();

private:
    FleetUIManager();

    void            FleetWndClosing(FleetWnd* fleet_wnd);
    void            FleetWndClicked(std::shared_ptr<FleetWnd> fleet_wnd);                          //!< sets active FleetWnd

    bool                                    m_order_issuing_enabled;

    /** All fleet windows.  mutable so expired ptrs can be reset(). */
    mutable std::set<std::weak_ptr<FleetWnd>,
                     std::owner_less<std::weak_ptr<FleetWnd>>> m_fleet_wnds;
    /** Active fleet window.  mutable so expired ptr can be reset(). */
    mutable std::weak_ptr<FleetWnd>                               m_active_fleet_wnd;

    std::vector<boost::signals2::connection> m_active_fleet_wnd_signals;
};

/** This is the top level Fleet UI element.  It shows a list of fleets, a
    new-fleet drop target, and a detail view of the currently selected fleet
    (a FleetDetailPanel). */
class FleetWnd : public MapWndPopup {
public:
    /** \name Structors */ //@{
    FleetWnd();
    FleetWnd(const std::vector<int>& fleet_ids, bool order_issuing_enabled,
             double allowed_bounding_box_leeway = 0,
             int selected_fleet_id = INVALID_OBJECT_ID,
             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE | GG::RESIZABLE,
             const std::string& config_name = "");

    ~FleetWnd();
    //@}
    void CompleteConstruction() override;

    //! \name Accessors //@{
    int                     SystemID() const;                   ///< returns ID of system whose fleets are shown in this FleetWnd, which may be INVALID_OBJECT_ID if this FleetWnd isn't set to show fleets of a system
    int                     EmpireID() const;                   ///< returns ID of empire whose fleets are shown in this FleetWnd, which may be ALL_EMPIRES if this FleetWnd isn't set to show fleets of a particular empire
    bool                    ContainsFleet(int fleet_id) const;  ///< returns true if fleet with ID \a fleet_id is shown in this FleetWnd
    /** Return true if this FleetWnd contains all \p fleet_ids. */
    template <typename Set>
    bool                    ContainsFleets(const Set& fleet_ids) const;
    const std::set<int>&    FleetIDs() const;                   ///< returns IDs of all fleets shown in this FleetWnd
    std::set<int>           SelectedFleetIDs() const;           ///< returns IDs of selected fleets in this FleetWnd
    std::set<int>           SelectedShipIDs() const;            ///< returns IDs of selected ships in this FleetWnd
    NewFleetAggression      GetNewFleetAggression() const;      ///< returns this FleetWnd's setting for new fleet aggression (auto, aggressive, or passive)

    GG::Rect CalculatePosition() const override;
    //@}

    //! \name Mutators //@{
    void PreRender() override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    /** Deselect all fleets. */
    void                    DeselectAllFleets();
    void                    SelectFleet(int fleet_id);                     ///< deselects any selected fleets, and selects the indicated fleet, bringing it into the fleet detail window
    void                    SelectFleets(const std::set<int>& fleet_ids);  ///< deselects any selected fleets, and selects the fleets with the indicated ids
    void                    SelectShips(const std::set<int>& ship_ids);    ///< deselected any selected ships, and selects the ships with the indicated ids if they are in the selected fleet.

    /** Enables, or disables if \a enable is false, issuing orders via this FleetWnd. */
    void                    EnableOrderIssuing(bool enable = true);
    //@}

    mutable boost::signals2::signal<void ()>          SelectedFleetsChangedSignal;
    mutable boost::signals2::signal<void ()>          SelectedShipsChangedSignal;
    mutable boost::signals2::signal<void (std::shared_ptr<FleetWnd>)> ClickedSignal;
    mutable boost::signals2::signal<void (int)>       FleetRightClickedSignal;
    mutable boost::signals2::signal<void (int)>       ShipRightClickedSignal;

protected:
    //! \name Mutators //@{
    void CloseClicked() override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void            DoLayout();
    //@}

private:
    void            RequireRefresh();
    void            Refresh();                          ///< regenerates contents
    void            RefreshStateChangedSignals();

    void            AddFleet(int fleet_id);     ///< adds a new fleet row to this FleetWnd's ListBox of FleetRows and updates internal fleets bookkeeping

    void            FleetSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            FleetRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void            FleetLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void            FleetDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);

    int             FleetInRow(GG::ListBox::iterator it) const;
    std::string     TitleText() const;
    void            CreateNewFleetFromDrops(const std::vector<int>& ship_ids);

    void            ShipSelectionChanged(const GG::ListBox::SelectionSet& rows);

    void            UniverseObjectDeleted(std::shared_ptr<const UniverseObject> obj);

    void            SetStatIconValues();          ///< sets values for multi-fleet aggregate stat icons at top of FleetWnd
    mutable boost::signals2::signal<void (FleetWnd*)> ClosingSignal;

    boost::signals2::connection              m_system_connection;
    std::vector<boost::signals2::connection> m_fleet_connections;

    std::set<int>       m_fleet_ids;        ///< IDs of fleets shown in this wnd (always.  set when creating wnd, either by being passed in directly, or found by checking indicated system for indicated empire's fleets.  If set directly, never updates.  If set by checking system, updates when the system has a fleet added or removed.
    int                 m_empire_id;        ///< ID of empire whose fleets are shown in this wnd.  May be ALL_EMPIRES if this FleetWnd wasn't set to shown a particular empire's fleets.
    int                 m_system_id;        ///< ID of system whose fleets are shown in this wnd.  May be INVALID_OBJECT_ID if this FleetWnd wasn't set to show a system's fleets.

    /** The size of box up to which moving fleets are still within the same fleet window.*/
    GG::Rect            m_bounding_box;
    bool                m_order_issuing_enabled;
    bool                m_needs_refresh = false;

    std::shared_ptr<FleetsListBox>      m_fleets_lb;
    std::shared_ptr<FleetDataPanel>     m_new_fleet_drop_target;
    std::shared_ptr<FleetDetailPanel>   m_fleet_detail_panel;

    std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>> m_stat_icons; /// statistic icons and associated meter types for multi-fleet aggregate

    friend class FleetUIManager;
};

#endif // _FleetWnd_h_
