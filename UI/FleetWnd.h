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

class FleetDetailWnd : public CUIWnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (Fleet*)>    ClosingSignalType;    ///< emitted when the detail wnd is closing
    typedef boost::signal<void (Fleet*)>    PanelEmptySignalType; ///< emitted when the detail wnd's fleet detail panel is empty
    //@}

    /** \name Slot Types */ //@{
    typedef ClosingSignalType::slot_type    ClosingSlotType;      ///< type of functor(s) invoked on a ClosingSignalType
    typedef PanelEmptySignalType::slot_type PanelEmptySlotType;   ///< type of functor(s) invoked on a PanelEmptySignalType
    //@}

    /** \name Structors */ //@{
    FleetDetailWnd(int x, int y, Fleet* fleet, bool read_only, Uint32 flags = GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE); ///< basic ctor
    ~FleetDetailWnd(); ///< virtual dtor
    //@}

    void CloseClicked();

    mutable ClosingSignalType    ClosingSignal;
    mutable PanelEmptySignalType PanelEmptySignal;

private:
    std::string TitleText() const;

    FleetDetailPanel* m_fleet_panel;
};


class FleetWnd : public MapWndPopup
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (Fleet*, FleetWnd*)> ShowingFleetSignalType;     ///< emitted to indicate to the rest of the UI that this window is showing the given fleet
    typedef boost::signal<void (Fleet*)>            NotShowingFleetSignalType;  ///< emitted to indicate that this window is not showing the given fleet
    typedef boost::signal<void (FleetWnd*)>         ClosingSignalType;          ///< emitted when the window is closing, due to last fleet being deleted, or being closed manually by user
    //@}

    /** \name Slot Types */ //@{
    typedef ShowingFleetSignalType::slot_type       ShowingFleetSlotType;      ///< type of functor(s) invoked on a ShowingFleetSignalType
    typedef NotShowingFleetSignalType::slot_type    NotShowingFleetSlotType;   ///< type of functor(s) invoked on a NotShowingFleetSignalType
    //@}

    /** \name Structors */ //@{
    /** constructs a fleet window for fleets in transit between systems */
    FleetWnd(int x, int y, std::vector<Fleet*> fleets, int selected_fleet, bool read_only, Uint32 flags = GG::CLICKABLE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);
    ~FleetWnd(); ///< dtor
    //@}

    //! \name Accessors //@{
    int SystemID() const;
    bool ContainsFleet(int fleet_id) const;
    std::set<Fleet*> Fleets() const;
    std::set<Fleet*> SelectedFleets() const;
    //@}

    //! \name Mutators //@{
    void AddFleet(Fleet* fleet); ///< adds a new fleet to a currently-open FletWnd
    void SelectFleet(Fleet* fleet); ///< selects the indicated fleet, bringing it into the fleet detail window
    //@}

    mutable ShowingFleetSignalType      ShowingFleetSignal;
    mutable NotShowingFleetSignalType   NotShowingFleetSignal;
    mutable ClosingSignalType           ClosingSignal;
    
    static GG::Pt LastPosition();    ///< returns the last position of the last FleetWnd that was closed

protected:
    //! \name Mutators //@{
    virtual void CloseClicked();
    //@}

private:
    void        Init(const std::vector<Fleet*>& fleet_ids, int selected_fleet);
    void        FleetSelectionChanged(const std::set<int>& rows);
    void        FleetRightClicked(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt);
    void        FleetDoubleClicked(int row_idx, GG::ListBox::Row* row);
    void        FleetDeleted(int row_idx, GG::ListBox::Row* row);
    void        FleetDetailWndClosing(Fleet* fleet);
    Fleet*      FleetInRow(int idx) const;
    std::string TitleText() const;
    void        DeleteFleet(Fleet* fleet);
    void        CreateNewFleetFromDrops(Ship* first_ship, const std::vector<int>& ship_ids);
    void        UniverseObjectDeleted(const UniverseObject *);
    void        SystemChangedSlot();

    const int           m_empire_id;
    int                 m_system_id;
    const bool          m_read_only;
    bool                m_moving_fleets;

    int                 m_current_fleet;

    std::map<Fleet*, FleetDetailWnd*> m_open_fleet_detail_wnds;

    std::set<boost::signals::connection>                        m_misc_connections;

    FleetsListBox*      m_fleets_lb;
    FleetDataPanel*     m_new_fleet_drop_target;
    FleetDetailPanel*   m_fleet_detail_panel;

    
    static GG::Pt s_last_position; ///< the latest position to which any FleetWnd has been moved.  This is used to keep the place of the fleet window in single-fleetwindow mode.
};

#endif // _FleetWnd_h_
