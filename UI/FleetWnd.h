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
    //@}

    //! \name Mutators //@{
    FleetWnd*       NewFleetWnd(std::vector<Fleet*> fleets, int selected_fleet, bool read_only,
                                GG::Flags<GG::WndFlag> flags =
                                GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);
    FleetDetailWnd* NewFleetDetailWnd(FleetWnd* fleet_wnd, Fleet* fleet, bool read_only,
                                      GG::Flags<GG::WndFlag> flags =
                                      GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE);

    void            CullEmptyWnds();
    void            SetActiveFleetWnd(FleetWnd* fleet_wnd);
    bool            CloseAll();
    void            RefreshAll();
    //@}

    mutable boost::signal<void ()> ActiveFleetWndChangedSignal;                 //!< emitted when the selected FleetWnd changes
    mutable boost::signal<void ()> ActiveFleetWndSelectedFleetsChangedSignal;   //!< emitted when the selected fleets in the active FleetWnd change

    static FleetUIManager& GetFleetUIManager();

private:
    FleetUIManager();

    void            FleetWndClosing(FleetWnd* fleet_wnd);
    void            FleetDetailWndClosing(FleetWnd* fleet_wnd, FleetDetailWnd* fleet_detail_wnd);
    void            FleetWndClicked(FleetWnd* fleet_wnd);                          //!< sets active FleetWnd


    typedef std::map<FleetWnd*, std::set<FleetDetailWnd*> > FleetWndMap;

    std::set<FleetWnd*>         m_fleet_wnds;
    FleetWndMap                 m_fleet_and_detail_wnds;
    FleetWnd*                   m_active_fleet_wnd;
    boost::signals::connection  m_ative_fleet_wnd_signal;
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
    int                     SystemID() const;
    bool                    ContainsFleet(int fleet_id) const;
    std::set<Fleet*>        Fleets() const;
    std::set<Fleet*>        SelectedFleets() const;
    //@}

    //! \name Mutators //@{
    void                    AddFleet(Fleet* fleet);     ///< adds a new fleet to a currently-open FletWnd
    void                    SelectFleet(Fleet* fleet);  ///< selects the indicated fleet, bringing it into the fleet detail window
    virtual void            SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void                    Refresh();                  ///< regenerates contents
    //@}

    static const GG::Pt&    LastPosition();         ///< returns the last position of the last FleetWnd that was closed
    static const GG::Pt&    LastSize();             ///< returns the last size ... ''

    mutable boost::signal<void ()>          SelectedFleetsChangedSignal;
    mutable boost::signal<void (FleetWnd*)> ClickedSignal;

protected:
    //! \name Mutators //@{
    virtual void    CloseClicked();
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void            DoLayout();
    //@}

private:
    /** Basic ctor. */
    FleetWnd(std::vector<Fleet*> fleets, int selected_fleet, bool read_only, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);
    FleetWnd(GG::X w, GG::Y h, std::vector<Fleet*> fleets, int selected_fleet, bool read_only, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE);

    void            Init(const std::vector<Fleet*>& fleets, int selected_fleet, bool read_only);
    void            FleetSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            FleetRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    void            FleetLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    void            FleetDoubleClicked(GG::ListBox::iterator it);
    void            FleetDeleted(GG::ListBox::iterator it);
    Fleet*          FleetInRow(GG::ListBox::iterator it) const;
    std::string     TitleText() const;
    void            CreateNewFleetFromDrops(Ship* first_ship, const std::vector<int>& ship_ids);
    void            UniverseObjectDeleted(const UniverseObject *obj);
    void            SystemChangedSlot();

    mutable boost::signal<void (FleetWnd*)> ClosingSignal;

    std::set<int>           m_fleet_ids;
    int                     m_empire_id;
    int                     m_system_id;
    const bool              m_read_only;
    bool                    m_moving_fleets;
    GG::ListBox::iterator   m_current_fleet;

    std::set<boost::signals::connection>    m_misc_connections;

    FleetsListBox*          m_fleets_lb;
    FleetDataPanel*         m_new_fleet_drop_target;
    FleetDetailPanel*       m_fleet_detail_panel;

    static GG::Pt           s_last_position;    ///< the latest position to which any FleetWnd has been moved.  This is used to keep the place of the fleet window in single-fleetwindow mode.
    static GG::Pt           s_last_size;        ///< the latest size to which any FleetWnd has been resized.  This is used to keep the size of the fleet window in single-fleetwindow mode.

    friend class FleetUIManager;
};

#endif // _FleetWnd_h_
