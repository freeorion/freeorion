// -*- C++ -*-
//SystemIcon.h
#ifndef _SystemIcon_h_
#define _SystemIcon_h_

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#ifndef _CUIDrawUtil_h_
#include "CUIDrawUtil.h"
#endif

class FleetButton;
class System;
namespace GG {
class StaticGraphic;
class TextControl;
}

/** a GUI control that allows interaction with a star system.
    This class allows user interaction with star systems
    on the galaxy map.  It contains the graphic to display the 
    system, along with the object ID of the UniverseObject associated
    with it. */
class SystemIcon : public GG::Control
{
public:
    using Wnd::SizeMove;

    //! \name Signal Types //!@{
    typedef boost::signal<void (int)> BrowsedSignalType; //!< emitted when the user moves the cursor over the icon; returns the object id
    typedef boost::signal<void (int)> LeftClickedSignalType; //!< emitted when the user left clicks the icon; returns the objectID
    typedef boost::signal<void (int)> RightClickedSignalType; //!< emitted when the user right clicks the icon; returns the objectID
    typedef boost::signal<void (int)> LeftDoubleClickedSignalType; //!< emitted when the user left double-clicks the icon; returns the object id
    //!@}

    //! \name Slot Types //!@{
    typedef BrowsedSignalType::slot_type BrowsedSlotType; //!< type of functor invoked when the user moves over the system
    typedef LeftClickedSignalType::slot_type LeftClickedSlotType; //!< type of functor invoked when the user left clicks
    typedef RightClickedSignalType::slot_type RightClickedSlotType; //!< type of functor invoked when the user right clicks
    typedef LeftDoubleClickedSignalType::slot_type LeftDoubleClickedSlotType; //!< type of functor invoked when the user left double-clicks
    //!@}

    //! \name Structors //!@{
    SystemIcon(int id, double zoom); //!< construct from a universe ID, to be placed in a MapWnd at zoom level \a zoom
    ~SystemIcon();      //!< dtor
    //!@}

    //! \name Accessors //!@{
    const System&  GetSystem() const {return m_system;}
    //!@}

    //! \name Mutators //!@{
    virtual void   SizeMove(int x1, int y1, int x2, int y2);
    virtual int    Render() {return 1;}
    virtual int    LClick(const GG::Pt& pt, Uint32 keys);
    virtual int    RClick(const GG::Pt& pt, Uint32 keys);
    virtual int    LDoubleClick(const GG::Pt& pt, Uint32 keys);

    void           ShowName(); //!< enables the system name text
    void           HideName(); //!< disables the system name text

    BrowsedSignalType&           BrowsedSignal()           {return m_browse_signal;}
    LeftClickedSignalType&       LeftClickedSignal()       {return m_left_click_signal;}
    RightClickedSignalType&      RightClickedSignal()      {return m_right_click_signal;}
    LeftDoubleClickedSignalType& LeftDoubleClickedSignal() {return m_left_double_click_signal;}
    //!@}

private:
    void CreateFleetButtons();
    void Refresh();
    void PositionSystemName();

    const System&       m_system;         //!< the System object associated with this SystemIcon
    GG::StaticGraphic*  m_static_graphic; //!< the control used to render the displayed texture
    GG::TextControl*    m_name;           //!< the control that holds the name of the system

    std::map<int, FleetButton*> m_stationary_fleet_markers; //!< the fleet buttons for the fleets that are stationary in the system, indexed by Empire ID of the owner
    std::map<int, FleetButton*> m_moving_fleet_markers;     //!< the fleet buttons for the fleets that are under orders to move out of the system, indexed by Empire ID of the owner

    BrowsedSignalType           m_browse_signal;
    LeftClickedSignalType       m_left_click_signal;
    RightClickedSignalType      m_right_click_signal;
    LeftDoubleClickedSignalType m_left_double_click_signal;

    friend class FleetButtonClickedFunctor;
};

#endif
