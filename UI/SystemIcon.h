// -*- C++ -*-
//SystemIcon.h
#ifndef _SystemIcon_h_
#define _SystemIcon_h_

#ifndef _GG_Button_h_
#include <GG/Button.h>
#endif

#ifndef _CUIDrawUtil_h_
#include "CUIDrawUtil.h"
#endif


class Fleet;
class FleetButton;
class System;
namespace GG {
class StaticGraphic;
class TextControl;
}

/** A TextControl-like GG::Control that displays the name of a system in the color(s) of the owning empire(s). */
class OwnerColoredSystemName : public GG::Control
{
public:
    OwnerColoredSystemName(const System* system, const boost::shared_ptr<GG::Font>& font, const std::string& format_text = "", Uint32 flags = 0);
    virtual void Render();

private:
    std::vector<GG::TextControl*> m_subcontrols;
};

/** a GUI control that allows interaction with a star system.  This class allows user interaction with star systems on
    the galaxy map.  It contains the graphic to display the system, along with the object ID of the UniverseObject
    associated with it. */
class SystemIcon : public GG::Control
{
public:
    //! \name Signal Types //!@{
    typedef boost::signal<void (int)> MouseEnteringSignalType; //!< emitted when the user moves the cursor over the icon; returns the object id
    typedef boost::signal<void (int)> MouseLeavingSignalType; //!< emitted when the user moves the cursor off of the icon; returns the object id
    typedef boost::signal<void (int)> LeftClickedSignalType; //!< emitted when the user left clicks the icon; returns the objectID
    typedef boost::signal<void (int)> RightClickedSignalType; //!< emitted when the user right clicks the icon; returns the objectID
    typedef boost::signal<void (int)> LeftDoubleClickedSignalType; //!< emitted when the user left double-clicks the icon; returns the object id
    //!@}

    //! \name Slot Types //!@{
    typedef MouseEnteringSignalType::slot_type MouseEnteringSlotType; //!< type of functor invoked when the user moves over the system
    typedef MouseLeavingSignalType::slot_type MouseLeavingSlotType; //!< type of functor invoked when the user moves off of the system
    typedef LeftClickedSignalType::slot_type LeftClickedSlotType; //!< type of functor invoked when the user left clicks
    typedef RightClickedSignalType::slot_type RightClickedSlotType; //!< type of functor invoked when the user right clicks
    typedef LeftDoubleClickedSignalType::slot_type LeftDoubleClickedSlotType; //!< type of functor invoked when the user left double-clicks
    //!@}

    //! \name Structors //!@{
    SystemIcon(int id, double zoom); //!< construct from a universe ID, to be placed in a MapWnd at zoom level \a zoom
    ~SystemIcon();      //!< dtor
    //!@}

    //! \name Accessors //!@{
    const System&      GetSystem() const;
    const FleetButton* GetFleetButton(Fleet* fleet) const;
    //!@}

    //! \name Mutators //!@{
    virtual void   SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void   Render() {}
    virtual void   LClick(const GG::Pt& pt, Uint32 keys);
    virtual void   RClick(const GG::Pt& pt, Uint32 keys);
    virtual void   LDoubleClick(const GG::Pt& pt, Uint32 keys);
    virtual void   MouseEnter(const GG::Pt& pt, Uint32 keys);
    virtual void   MouseLeave();
    void           SetSelected(bool selected = true);   //!< shows/hides the system selection indicator over this system

    void           Refresh(); //!< sets up the icon's fleet buttons, generates fleet movement lines, etc.  Should be called after an icon is attached to the map
    void           ClickFleetButton(Fleet* fleet); //!< clicks the FleetButton containing \a fleet
    void           ShowName(); //!< enables the system name text
    void           HideName(); //!< disables the system name text

    mutable MouseEnteringSignalType     MouseEnteringSignal;
    mutable MouseLeavingSignalType      MouseLeavingSignal;
    mutable LeftClickedSignalType       LeftClickedSignal;
    mutable RightClickedSignalType      RightClickedSignal;
    mutable LeftDoubleClickedSignalType LeftDoubleClickedSignal;
    //!@}

private:
    void CreateFleetButtons();
    void PositionSystemName();
    void FleetCreatedOrDestroyed(const Fleet&);

    const System&                   m_system;         //!< the System object associated with this SystemIcon
    GG::StaticGraphic*              m_static_graphic; //!< the control used to render the displayed texture
    GG::StaticGraphic*              m_selection_indicator;  //!< shown to indicate system is selected in sidepanel
    bool                            m_selected;             //!< is this icon presently selected / should it show m_selected_indicator
    OwnerColoredSystemName*         m_name;           //!< the control that holds the name of the system
    GG::Clr                         m_default_star_color;

    std::map<int, FleetButton*> m_stationary_fleet_markers; //!< the fleet buttons for the fleets that are stationary in the system, indexed by Empire ID of the owner
    std::map<int, FleetButton*> m_moving_fleet_markers;     //!< the fleet buttons for the fleets that are under orders to move out of the system, indexed by Empire ID of the owner

    friend class FleetButtonClickedFunctor;
};

#endif // _SystemIcon_h_
