// -*- C++ -*-
#ifndef _FleetWnd_h_
#define _FleetWnd_h_

#include "CUIWnd.h"
#include <GG/ListBox.h>
#include "MapWnd.h"

class CUIListBox;
class Fleet;
class FleetDataPanel;
class FleetDetailPanel;
class FleetsListBox;
class Ship;
class System;
class UniverseObject;
class StatisticIcon;
namespace GG {
    class StaticGraphic;
}
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
    typedef std::set<FleetWnd*>::const_iterator iterator;

    //! \name Accessors //@{
    bool            empty() const;
    iterator        begin() const;
    iterator        end() const;
    FleetWnd*       ActiveFleetWnd() const;
    FleetWnd*       WndForFleet(TemporaryPtr<const Fleet> fleet) const;
    int             SelectedShipID() const;     // if a single ship is selected in the active fleetwnd, returns that ship's ID.  Otherwise, returns INVALID_OBJECT_ID
    std::set<int>   SelectedShipIDs() const;    // returns the ids of all selected ships in the active fleetwnd
    //@}

    //! \name Mutators //@{
    FleetWnd*       NewFleetWnd(const std::vector<int>& fleet_ids,
                                int selected_fleet_id = INVALID_OBJECT_ID,
                                GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE | GG::RESIZABLE);

    FleetWnd*       NewFleetWnd(int system_id, int empire_id,
                                int selected_fleet_id = INVALID_OBJECT_ID,
                                GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE | GG::RESIZABLE);

    void            CullEmptyWnds();
    void            SetActiveFleetWnd(FleetWnd* fleet_wnd);
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
    void            FleetWndClicked(FleetWnd* fleet_wnd);                          //!< sets active FleetWnd

    bool                                    m_order_issuing_enabled;
    std::set<FleetWnd*>                     m_fleet_wnds;
    FleetWnd*                               m_active_fleet_wnd;
    std::vector<boost::signals2::connection> m_active_fleet_wnd_signals;
};

/** This is the top level Fleet UI element.  It shows a list of fleets, a
    new-fleet drop target, and a detail view of the currently selectd fleet
    (a FleetDetailPanel). */
class FleetWnd : public MapWndPopup {
public:
    /** \name Structors */ //@{
    ~FleetWnd(); ///< dtor
    //@}

    //! \name Accessors //@{
    int                     SystemID() const;                   ///< returns ID of system whose fleets are shown in this FleetWnd, which may be INVALID_OBJECT_ID if this FleetWnd isn't set to show fleets of a system
    int                     EmpireID() const;                   ///< returns ID of empire whose fleets are shown in this FleetWnd, which may be ALL_EMPIRES if this FleetWnd isn't set to show fleets of a particular empire
    bool                    ContainsFleet(int fleet_id) const;  ///< returns true if fleet with ID \a fleet_id is shown in this FleetWnd
    const std::set<int>&    FleetIDs() const;                   ///< returns IDs of all fleets shown in this FleetWnd
    std::set<int>           SelectedFleetIDs() const;           ///< returns IDs of selected fleets in this FleetWnd
    std::set<int>           SelectedShipIDs() const;            ///< returns IDs of selected ships in this FleetWnd
    NewFleetAggression      GetNewFleetAggression() const;      ///< returns this FleetWnd's setting for new fleet aggression (auto, aggressive, or passive)
    //@}

    //! \name Mutators //@{
    void                    SelectFleet(int fleet_id);                          ///< deselects any selected fleets, and selects the indicated fleet, bringing it into the fleet detail window
    void                    SetSelectedFleets(const std::set<int>& fleet_ids);  ///< deselects any selected fleets, and selects the fleets with the indicated ids
    void                    SetSelectedShips(const std::set<int>& ship_ids);    ///< deselected any selected ships, and selects the ships with the indicated ids if they are in the selected fleet.
    virtual void            SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void                    Refresh();                          ///< regenerates contents

    /** Enables, or disables if \a enable is false, issuing orders via this FleetWnd. */
    void                    EnableOrderIssuing(bool enable = true);
    //@}

    static const GG::Pt&    LastPosition();                     ///< returns the last position of the last FleetWnd that was closed
    static const GG::Pt&    LastSize();                         ///< returns the last size ... ''

    mutable boost::signals2::signal<void ()>          SelectedFleetsChangedSignal;
    mutable boost::signals2::signal<void ()>          SelectedShipsChangedSignal;
    mutable boost::signals2::signal<void (FleetWnd*)> ClickedSignal;
    mutable boost::signals2::signal<void (int)>       FleetRightClickedSignal;
    mutable boost::signals2::signal<void (int)>       ShipRightClickedSignal;

protected:
    //! \name Mutators //@{
    virtual void    CloseClicked();
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void            DoLayout();
    //@}

private:
    /** \name Structors */ //@{
    FleetWnd(const std::vector<int>& fleet_ids, bool order_issuing_enabled,
             int selected_fleet_id = INVALID_OBJECT_ID,
             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE | GG::RESIZABLE);

    FleetWnd(int system_id, int empire_id, bool order_issuing_enabled,
             int selected_fleet_id = INVALID_OBJECT_ID,
             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE | GG::RESIZABLE);
    //@}

    void            Init(int selected_fleet_id);
    void            RefreshStateChangedSignals();

    void            AddFleet(int fleet_id);     ///< adds a new fleet row to this FleetWnd's ListBox of FleetRows and updates internal fleets bookkeeping
    void            RemoveFleet(int fleet_id);  ///< removes fleet row and updates internal bookkeeping

    void            FleetSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            FleetRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    void            FleetLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    void            FleetDoubleClicked(GG::ListBox::iterator it);

    int             FleetInRow(GG::ListBox::iterator it) const;
    std::string     TitleText() const;
    void            CreateNewFleetFromDrops(const std::vector<int>& ship_ids);

    void            ShipSelectionChanged(const GG::ListBox::SelectionSet& rows);

    void            UniverseObjectDeleted(TemporaryPtr<const UniverseObject> obj);

    void            SystemChangedSlot();                    ///< responds to StateChangedSignal emitted by the system this FleetWnd is showing the contents of
    void            SetStatIconValues();          ///< sets values for multi-fleet aggregate stat icons at top of FleetWnd
    mutable boost::signals2::signal<void (FleetWnd*)> ClosingSignal;

    boost::signals2::connection  m_system_connection;

    std::set<int>       m_fleet_ids;        ///< IDs of fleets shown in this wnd (always.  set when creating wnd, either by being passed in directly, or found by checking indicated system for indicated empire's fleets.  If set directly, never updates.  If set by checking system, updates when the system has a fleet added or removed.
    int                 m_empire_id;        ///< ID of empire whose fleets are shown in this wnd.  May be ALL_EMPIRES if this FleetWnd wasn't set to shown a particular empire's fleets.
    int                 m_system_id;        ///< ID of system whose fleets are shown in this wnd.  May be INVALID_OBJECT_ID if this FleetWnd wasn't set to show a system's fleets.

    bool                m_order_issuing_enabled;

    FleetsListBox*      m_fleets_lb;
    FleetDataPanel*     m_new_fleet_drop_target;
    FleetDetailPanel*   m_fleet_detail_panel;

    static GG::Pt       s_last_position;    ///< the latest position to which any FleetWnd has been moved.  This is used to keep the place of the fleet window in single-fleetwindow mode.
    static GG::Pt       s_last_size;        ///< the latest size to which any FleetWnd has been resized.  This is used to keep the size of the fleet window in single-fleetwindow mode.
    std::vector<std::pair<MeterType, StatisticIcon*> >    m_stat_icons;   /// statistic icons and associated meter types for multi-fleet aggregate
    
    friend class FleetUIManager;
};

/** Shows info about a single ship. */
class ShipDataPanel : public GG::Control {
public:
    /** \name Structors */ //@{
    ShipDataPanel(GG::X w, GG::Y h, int ship_id);
    ~ShipDataPanel();
    //@}

    //! \name Accessors //@{
    /** Excludes border from the client area. */
    virtual GG::Pt  ClientUpperLeft() const;
    /** Excludes border from the client area. */
    virtual GG::Pt  ClientLowerRight() const;
    //@}

    //! \name Mutators //@{
    /** Renders black panel background, border with color depending on the
      *current state and a background for the ship's name text. */
    virtual void    Render();
    void            Select(bool b);
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}

private:
    double          StatValue(MeterType stat_name) const;

    void            SetShipIcon();
    void            Refresh();
    void            DoLayout();

    void            Init();

    bool                        m_initialized;

    int                         m_ship_id;
    GG::StaticGraphic*          m_ship_icon;
    GG::StaticGraphic*          m_scrap_indicator;
    GG::StaticGraphic*          m_colonize_indicator;
    GG::StaticGraphic*          m_invade_indicator;
    GG::StaticGraphic*          m_bombard_indicator;
    ScanlineControl*            m_scanline_control;
    CUILabel*                   m_ship_name_text;
    CUILabel*                   m_design_name_text;

    std::vector<std::pair<MeterType, StatisticIcon*> >    m_stat_icons;   // statistic icons and associated meter types

    bool                        m_selected;
    boost::signals2::connection  m_ship_connection;
    boost::signals2::connection  m_fleet_connection;
};

#endif // _FleetWnd_h_
