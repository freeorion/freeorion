// -*- C++ -*-
#ifndef _FleetWindow_h_
#define _FleetWindow_h_

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

#ifndef _GGListBox_h_
#include "GGListBox.h"
#endif

#ifndef _MapWnd_h_
#include "MapWnd.h"
#endif

class CUIButton;
class CUIListBox;
class Fleet;
class Ship;
class System;
namespace GG {
class TextControl;
}


class FleetsWnd;

class FleetDetailPanel : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (Fleet*)>      PanelEmptySignalType;    ///< emitted when the panel is empty (no ships)
    typedef boost::signal<Fleet* (int)>       NeedNewFleetSignalType;  ///< emitted when ships are dragged and dropped into a null fleet
    //@}

    /** \name Slot Types */ //@{
    typedef PanelEmptySignalType::slot_type   PanelEmptySlotType;      ///< type of functor(s) invoked on a PanelEmptySignalType
    typedef NeedNewFleetSignalType::slot_type NeedNewFleetSlotType;    ///< type of functor(s) invoked on a CreateNewFleetSignalType
    //@}

    /** \name Structors */ //@{
    FleetDetailPanel(int x, int y, Fleet* fleet, bool read_only, Uint32 flags = 0); ///< ctor
    FleetDetailPanel(const GG::XMLElement& elem); ///< ctor that constructs a FleetViewPanel object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a FleetViewPanel object
    //@}

    /** \name Accessors */ //@{
    int          GetShipIDOfListRow(int row_idx) const; ///< returns the ID number of the ship in row \a row_idx of the ships listbox
    Fleet*       GetFleet() const {return m_fleet;} ///< returns the currently-displayed fleet (may be 0)

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a FleetViewPanel object

    PanelEmptySignalType&   PanelEmptySignal() const    {return m_panel_empty_sig;}
    NeedNewFleetSignalType& NeedNewFleetSignal() const  {return m_need_new_fleet_sig;}
    //@}

    //! \name Mutators //@{
    void SetFleet(Fleet* fleet); ///< sets the currently-displayed Fleet (may be null)
    //@}

protected:
    //! \name Mutators //@{
    virtual void CloseClicked();
    //@}

private:
    void        Init();
    void        AttachSignalChildren();
    void        DetachSignalChildren();
    void        Refresh();
    void        ShipBrowsed(int row_idx);
    void        ShipDroppedIntoList(int row_idx, const GG::ListBox::Row* row);
    void        ShipRightClicked(int row_idx, const GG::ListBox::Row* row, const GG::Pt& pt);
    std::string DestinationText() const;
    std::string ShipStatusText(int ship_id) const;

    Fleet*                      m_fleet;
    const bool                  m_read_only;
    boost::signals::connection  m_fleet_connection;

    GG::TextControl*            m_destination_text;
    CUIListBox*                 m_ships_lb;
    GG::TextControl*            m_ship_status_text;

    mutable PanelEmptySignalType   m_panel_empty_sig;
    mutable NeedNewFleetSignalType m_need_new_fleet_sig;
};

class FleetDetailWnd : public CUI_Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (FleetDetailWnd*)>        ClosingSignalType;      ///< emitted when this window is about to close
    typedef boost::signal<Fleet* (FleetDetailWnd*, int)> NeedNewFleetSignalType; ///< emitted when ships are dragged and dropped into a null fleet
    //@}

    /** \name Slot Types */ //@{
    typedef ClosingSignalType::slot_type                 ClosingSlotType;      ///< type of functor(s) invoked on a ClosingSignalType
    typedef NeedNewFleetSignalType::slot_type            NeedNewFleetSlotType; ///< type of functor(s) invoked on a NeedNewFleetSignalType
    //@}

    /** \name Structors */ //@{
    FleetDetailWnd(int x, int y, Fleet* fleet, bool read_only, Uint32 flags = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE | MINIMIZABLE); ///< basic ctor
    FleetDetailWnd(const GG::XMLElement& elem); ///< ctor that constructs a FleetContentsWnd object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a FleetContentsWnd object
    ~FleetDetailWnd(); ///< dtor
    //@}

    //! \name Accessors //@{
    FleetDetailPanel&       GetFleetDetailPanel() const {return *m_fleet_panel;} ///< returns the internally-held fleet panel for theis window

    NeedNewFleetSignalType& NeedNewFleetSignal() const  {return m_need_new_fleet_sig;}
    ClosingSignalType&      ClosingSignal() const       {return m_closing_sig;}
    //@}

protected:
    //! \name Mutators //@{
    virtual void CloseClicked();
    //@}

private:
    Fleet*      PanelNeedsNewFleet(int ship_id) {return m_need_new_fleet_sig(this, ship_id);}
    void        AttachSignalChildren();
    void        DetachSignalChildren();
    std::string TitleText() const;

    FleetDetailPanel*  m_fleet_panel;

    mutable ClosingSignalType      m_closing_sig;
    mutable NeedNewFleetSignalType m_need_new_fleet_sig;
};


class FleetWnd : public MapWndPopup
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (Fleet*)>         ShowingFleetSignalType;    ///< emitted to indicate to the rest of the UI that this window is showing the given fleet, so duplicates are avoided
    typedef boost::signal<void (Fleet*)>         NotShowingFleetSignalType; ///< emitted to indicate that this window is not showing the given fleet
    //@}

    /** \name Slot Types */ //@{
    typedef ShowingFleetSignalType::slot_type    ShowingFleetSlotType;      ///< type of functor(s) invoked on a ShowingFleetSignalType
    typedef NotShowingFleetSignalType::slot_type NotShowingFleetSlotType;   ///< type of functor(s) invoked on a NotShowingFleetSignalType
    //@}

    /** \name Structors */ //@{
    /** constructs a fleet window for fleets in transit between systems */
    FleetWnd(int x, int y, std::vector<Fleet*> fleets, bool read_only, Uint32 flags = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE | MINIMIZABLE);

    FleetWnd(const GG::XMLElement& elem); ///< ctor that constructs a FleetsWnd object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a FleetsWnd object
    ~FleetWnd(); ///< dtor
    //@}

    //! \name Mutators //@{
    ShowingFleetSignalType&                       ShowingFleetSignal() const     {return m_showing_fleet_sig;}
    NotShowingFleetSignalType&                    NotShowingFleetSignal() const  {return m_not_showing_fleet_sig;}
    //@}

    //! \name Mutators //@{
    void SystemClicked(int system_id); ///< invoked when a system is clicked on the main map, possibly indicating that the currently-selected fleet should move there
    //@}

protected:
    //! \name Mutators //@{
    virtual void CloseClicked();
    //@}

private:
    void        Init(const std::vector<Fleet*>& fleet_ids);
    void        AttachSignalChildren();
    void        DetachSignalChildren();
    void        FleetBrowsed(int row_idx);
    void        FleetSelectionChanged(const std::set<int>& rows);
    void        FleetRightClicked(int row_idx, const GG::ListBox::Row* row, const GG::Pt& pt);
    void        FleetDoubleClicked(int row_idx, const GG::ListBox::Row* row);
    void        FleetDeleted(int row_idx);
    void        NewFleetButtonClicked();
    Fleet*      NewFleetWndReceivedShip(FleetDetailWnd* fleet_wnd, int ship_id);
    void        FleetDetailWndClosing(FleetDetailWnd* wnd);
    Fleet*      FleetInRow(int idx) const;
    std::string TitleText() const;
    void        DeleteFleet(Fleet* fleet);
    void        RemoveEmptyFleets();

    const int           m_empire_id;
    const bool          m_read_only;
    bool                m_moving_fleets;

    int                 m_current_fleet;

    std::map<Fleet*, FleetDetailWnd*> m_open_fleet_windows;
    std::set<FleetDetailWnd*>         m_new_fleet_windows;

    CUIListBox*         m_fleets_lb;
    FleetDetailPanel*   m_fleet_detail_panel;
    CUIButton*          m_new_fleet_button;

    mutable ShowingFleetSignalType    m_showing_fleet_sig;
    mutable NotShowingFleetSignalType m_not_showing_fleet_sig;

    static int          s_new_fleet_count;
};

#endif
