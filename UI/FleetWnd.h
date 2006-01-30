// -*- C++ -*-
#ifndef _FleetWnd_h_
#define _FleetWnd_h_

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif

#ifndef _GG_ListBox_h_
#include <GG/ListBox.h>
#endif

#ifndef _MapWnd_h_
#include "MapWnd.h"
#endif

class CUIListBox;
class Fleet;
class FleetDetailPanel;
class FleetsListBox;
class Ship;
class System;
class UniverseObject;
namespace GG {
    class TextControl;
}


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
    typedef boost::signal<void (Fleet*, FleetWnd*)> ShowingFleetSignalType;    ///< emitted to indicate to the rest of the UI that this window is showing the given fleet, so duplicates are avoided
    typedef boost::signal<void (Fleet*)>            NotShowingFleetSignalType; ///< emitted to indicate that this window is not showing the given fleet
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
    //@}

    //! \name Mutators //@{
    void SystemClicked(int system_id); ///< invoked when a system is clicked on the main map, possibly indicating that the currently-selected fleet should move there
    void AddFleet(Fleet* fleet); ///< adds a new fleet to a currently-open FletWnd
    void SelectFleet(Fleet* fleet); ///< selects the indicated fleet, bringing it into the fleet detail window
    //@}

    mutable ShowingFleetSignalType    ShowingFleetSignal;
    mutable NotShowingFleetSignalType NotShowingFleetSignal;
    
    static bool FleetWndsOpen();     ///< returns true iff one or more fleet windows are open
    static bool CloseAllFleetWnds(); ///< returns true iff fleet windows were open before it was called.  Used most often for fleet window quick-close.
    static GG::Pt LastPosition();    ///< returns the last position of the last FleetWnd that was closed
    
    typedef std::set<FleetWnd*>::const_iterator FleetWndItr;
    static FleetWndItr FleetWndBegin();
    static FleetWndItr FleetWndEnd();

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
    void        NewFleetButtonClicked();
    void        FleetDetailWndClosing(Fleet* fleet);
    Fleet*      FleetInRow(int idx) const;
    std::string TitleText() const;
    void        FleetPanelEmpty(Fleet* fleet);
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
    std::map<FleetDetailWnd*, boost::signals::connection> m_open_fleet_detail_wnd_connections;

    FleetsListBox*      m_fleets_lb;
    FleetDetailPanel*   m_fleet_detail_panel;

    boost::signals::connection  m_lb_delete_connection;
    boost::signals::connection  m_system_changed_connection;

    static std::set<FleetWnd*> s_open_fleet_wnds;
    static GG::Pt s_last_position; ///< the latest position to which any FleetWnd has been moved.  This is used to keep the place of the fleet window in single-fleetwindow mode.
};

inline std::pair<std::string, std::string> FleetWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _FleetWnd_h_
