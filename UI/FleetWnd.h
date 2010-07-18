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
class FleetDetailWnd;
class FleetsListBox;
class Ship;
class System;
class UniverseObject;

/** Manages the lifetimes of FleetWnds and FleetDetailWnds.  A 1-many
    relationshp exists between FleetWnds and FleetDetailWnds, and this manager
    ensures that:
    - closing a FleetWnd closes all its associated FleetDetailWnds;
    - opening a FleetDetailWnd does not result in two FleetDetailWnds showing the same fleet; and
    - empty FleetWnds and FleetDetailWnds are closed.
    These elements of functionality are very difficult to get right without an
    external manager like this one -- most of the UI elements update themselves
    when their underlying Fleet(s) change, and so any propagating changes to a
    Fleet(s) tend to cause crashes (e.g. when an empty FleetDetailWnds is deleted
    for being empty before its Fleet signals it to update). */
class FleetUIManager
{
public:
    typedef std::set<FleetWnd*>::const_iterator iterator;

    //! \name Accessors //@{
    bool            empty() const;
    iterator        begin() const;
    iterator        end() const;
    FleetWnd*       ActiveFleetWnd() const;
    FleetWnd*       WndForFleet(const Fleet* fleet) const;
    std::size_t     OpenDetailWnds(FleetWnd* fleet_wnd) const;
    int             SelectedShipID() const;
    //@}

    //! \name Mutators //@{
    FleetWnd*       NewFleetWnd(const std::vector<int>& fleet_ids, bool read_only,
                                int selected_fleet_id = UniverseObject::INVALID_OBJECT_ID,
                                GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);

    FleetWnd*       NewFleetWnd(int system_id, int empire_id, bool read_only,
                                int selected_fleet_id = UniverseObject::INVALID_OBJECT_ID,
                                GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);

    FleetDetailWnd* NewFleetDetailWnd(FleetWnd* fleet_wnd, int fleet_id, bool read_only,
                                      GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE);

    void            CullEmptyWnds();
    void            SetActiveFleetWnd(FleetWnd* fleet_wnd);
    bool            CloseAll();
    void            RefreshAll();
    //@}

    mutable boost::signal<void ()> ActiveFleetWndChangedSignal;                 //!< emitted when the selected FleetWnd changes
    mutable boost::signal<void ()> ActiveFleetWndSelectedFleetsChangedSignal;   //!< emitted when the selected fleets in the active FleetWnd change
    mutable boost::signal<void ()> ActiveFleetWndSelectedShipsChangedSignal;    //!< emitted when the selected ships in the active FleetWnd change

    static FleetUIManager& GetFleetUIManager();

private:
    FleetUIManager();

    void            FleetWndClosing(FleetWnd* fleet_wnd);
    void            FleetDetailWndClosing(FleetWnd* fleet_wnd, FleetDetailWnd* fleet_detail_wnd);
    void            FleetWndClicked(FleetWnd* fleet_wnd);                          //!< sets active FleetWnd


    typedef std::map<FleetWnd*, std::set<FleetDetailWnd*> > FleetWndMap;

    std::set<FleetWnd*>                     m_fleet_wnds;
    FleetWndMap                             m_fleet_and_detail_wnds;
    FleetWnd*                               m_active_fleet_wnd;
    std::vector<boost::signals::connection> m_active_fleet_wnd_signals;
};

/** This is the top level Fleet UI element.  It shows a list of fleets, a
    new-fleet drop target, and a detail view of the currently selectd fleet (a
    FleetDetailPanel). */
class FleetWnd : public MapWndPopup
{
public:
    /** \name Structors */ //@{
    ~FleetWnd(); ///< dtor
    //@}

    //! \name Accessors //@{
    int                     SystemID() const;                   ///< returns ID of system whose fleets are shown in this FleetWnd, which may be UniverseObject::INVALID_OBJECT_ID if this FleetWnd isn't set to show fleets of a system
    int                     EmpireID() const;                   ///< returns ID of empire whose fleets are shown in this FleetWnd, which may be ALL_EMPIRES if this FleetWnd isn't set to show fleets of a particular empire
    bool                    ContainsFleet(int fleet_id) const;  ///< returns true if fleet with ID \a fleet_id is shown in this FleetWnd
    const std::set<int>&    FleetIDs() const;                   ///< returns IDs of all fleets shown in this FleetWnd
    std::set<int>           SelectedFleetIDs() const;           ///< returns IDs of selected fleets in this FleetWnd
    std::set<int>           SelectedShipIDs() const;            ///< returns IDs of selected ships in this FleetWnd
    //@}

    //! \name Mutators //@{
    void                    SelectFleet(int fleet_id);                          ///< deselects any selected fleets, and selects the indicated fleet, bringing it into the fleet detail window
    void                    SetSelectedFleets(const std::set<int>& fleet_ids);  ///< deselects any selected fleets, and selects the fleets with the indicated ids
    virtual void            SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void                    Refresh();                          ///< regenerates contents
    //@}

    static const GG::Pt&    LastPosition();                     ///< returns the last position of the last FleetWnd that was closed
    static const GG::Pt&    LastSize();                         ///< returns the last size ... ''

    mutable boost::signal<void ()>          SelectedFleetsChangedSignal;
    mutable boost::signal<void ()>          SelectedShipsChangedSignal;
    mutable boost::signal<void (FleetWnd*)> ClickedSignal;

protected:
    //! \name Mutators //@{
    virtual void    CloseClicked();
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void            DoLayout();
    //@}

private:
    /** \name Structors */ //@{
    FleetWnd(const std::vector<int>& fleet_ids, bool read_only,
             int selected_fleet_id = UniverseObject::INVALID_OBJECT_ID,
             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);

    FleetWnd(int system_id, int empire_id, bool read_only,
             int selected_fleet_id = UniverseObject::INVALID_OBJECT_ID,
             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);
    //@}

    void            Init(int selected_fleet_id);

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

    void            UniverseObjectDeleted(const UniverseObject *obj);

    void            SystemChangedSlot();                    ///< responds to StateChangedSignal emitted by the system this FleetWnd is showing the contents of
    void            SystemFleetInsertedSlot(Fleet& fleet);  ///< responds to FleetInsertedSignal emitted by the system ...
    void            SystemFleetRemovedSlot(Fleet& fleet);   ///< responds to FleetRemovedSignal ...

    mutable boost::signal<void (FleetWnd*)> ClosingSignal;

    std::set<int>           m_fleet_ids;        ///< IDs of fleets shown in this wnd (always.  set when creating wnd, either by being passed in directly, or found by checking indicated system for indicated empire's fleets.  If set directly, never updates.  If set by checking system, updates when the system has a fleet added or removed.
    int                     m_empire_id;        ///< ID of empire whose fleets are shown in this wnd.  May be ALL_EMPIRES if this FleetWnd wasn't set to shown a particular empire's fleets.
    int                     m_system_id;        ///< ID of system whose fleets are shown in this wnd.  May be UniverseObject::INVALID_OBJECT_ID if this FleetWnd wasn't set to show a system's fleets.

    bool                    m_read_only;

    FleetsListBox*          m_fleets_lb;
    FleetDataPanel*         m_new_fleet_drop_target;
    FleetDetailPanel*       m_fleet_detail_panel;

    static GG::Pt           s_last_position;    ///< the latest position to which any FleetWnd has been moved.  This is used to keep the place of the fleet window in single-fleetwindow mode.
    static GG::Pt           s_last_size;        ///< the latest size to which any FleetWnd has been resized.  This is used to keep the size of the fleet window in single-fleetwindow mode.

    friend class FleetUIManager;
};

#endif // _FleetWnd_h_
