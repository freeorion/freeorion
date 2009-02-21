// -*- C++ -*-
//SystemIcon.h
#ifndef _SystemIcon_h_
#define _SystemIcon_h_

#include <GG/Button.h>
#include "CUIDrawUtil.h"

class Fleet;
class FleetButton;
class System;
namespace GG {
    class StaticGraphic;
    class TextControl;
}

/** A TextControl-like GG::Control that displays the name of a system in the color(s) of the owning empire(s). 
    This class is derived from GG::Control because GG::ListBox::Row accepts GG::Control but not GG::Wnd being
    added to them.  OwnerColoredSystemName are added to the list of systems on the SidePanel. */
class OwnerColoredSystemName : public GG::Control
{
public:
    OwnerColoredSystemName(const System* system, const boost::shared_ptr<GG::Font>& font, const std::string& format_text = "", GG::Flags<GG::WndFlag> flags = GG::Flags<GG::WndFlag>());
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
    typedef boost::signal<void (int)>   MouseEnteringSignalType;    //!< emitted when the user moves the cursor over the icon; returns the object id
    typedef boost::signal<void (int)>   MouseLeavingSignalType;     //!< emitted when the user moves the cursor off of the icon; returns the object id
    typedef boost::signal<void (int)>   LeftClickedSignalType;      //!< emitted when the user left clicks the icon; returns the objectID
    typedef boost::signal<void (int)>   RightClickedSignalType;     //!< emitted when the user right clicks the icon; returns the objectID
    typedef boost::signal<void (int)>   LeftDoubleClickedSignalType;    //!< emitted when the user left double-clicks the icon; returns the object id
    typedef boost::signal<void (FleetButton&, bool)>  FleetButtonClickedSignalType;   //!< emitted when one of the fleet buttons on this icon is clicked
    //!@}

    //! \name Structors //!@{
    SystemIcon(GG::Wnd* parent, GG::X x, GG::Y y, GG::X w, int id);     //!< construct from a universe ID at specified size and position
    ~SystemIcon();                                                      //!< dtor
    //!@}

    //! \name Accessors //!@{
    const System&       GetSystem() const;
    const FleetButton*  GetFleetButton(const Fleet* fleet) const;
    GG::Pt              FleetButtonCentre(int empire_id, bool moving) const;    //!< returns centre of fleetbutton owned by empire with id \a empire_id, or GG::Pt(INVALID_POSITION, INVALID_POSITION) if there is no such FleetButton for the specified empire.

    const boost::shared_ptr<GG::Texture>& DiscTexture() const;  //!< returns the solid star disc texture
    const boost::shared_ptr<GG::Texture>& HaloTexture() const;  //!< returns the transparent star halo texture
    const boost::shared_ptr<GG::Texture>& TinyTexture() const;  //!< returns the alternate texture shown when icon very small

    virtual bool        InWindow(const GG::Pt& pt) const;       //!< Overrides GG::Wnd::InWindow. Checks to see if point lies inside in-system fleet buttons before checking main InWindow method.
    int                 EnclosingCircleDiameter() const;        //!< returns diameter of circle enclosing icon around which other icons can be placed and within which the mouse is over the icon

    //!@}

    //! \name Mutators //!@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    Render();
    void            ManualRender(double halo_scale_factor);     //!< Draw disc and halo textures
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();
    void            SetSelected(bool selected = true);   //!< shows/hides the system selection indicator over this system

    void            Refresh();                      //!< Resets system name text and calls RefreshFleetButtons().  Should be called after an icon is attached to the map
    void            RefreshFleetButtons();          //!< Recreates all fleet buttons and fleet move lines from them

    void            DoFleetButtonLayout();          //!< arranges and resizes fleet buttons
    void            ClickFleetButton(Fleet* fleet); //!< clicks the FleetButton containing \a fleet
    void            ShowName();                     //!< enables the system name text
    void            HideName();                     //!< disables the system name text

    mutable MouseEnteringSignalType         MouseEnteringSignal;
    mutable MouseLeavingSignalType          MouseLeavingSignal;
    mutable LeftClickedSignalType           LeftClickedSignal;
    mutable RightClickedSignalType          RightClickedSignal;
    mutable LeftDoubleClickedSignalType     LeftDoubleClickedSignal;
    mutable FleetButtonClickedSignalType    FleetButtonClickedSignal;
    //!@}

    /** The size below which the "tiny" version of the star graphic is used, in pixels. */
    static const GG::X TINY_SIZE;

private:
    void    Init(); //!< common constructor tasks

    GG::Pt  NthFleetButtonUpperLeft(int button_number, bool moving) const;  //!< returns upper left point of moving or stationary fleetbutton owned by empire \a n, where n is the position in order of fleetbuttons shown starting from 1(not empire id)


    void    FleetInserted(Fleet& fleet);
    void    FleetRemoved(Fleet& fleet);
    void    FleetStateChanged();

    void    PositionSystemName();

    const System&                   m_system;               //!< the System object associated with this SystemIcon
    boost::shared_ptr<GG::Texture>  m_disc_texture;         //!< solid star disc texture
    boost::shared_ptr<GG::Texture>  m_halo_texture;         //!< transparent star halo texture
    boost::shared_ptr<GG::Texture>  m_tiny_texture;         //!< alternate texture shown when icon very small
    GG::StaticGraphic*              m_tiny_graphic;
    GG::StaticGraphic*              m_selection_indicator;  //!< shown to indicate system is selected in sidepanel
    GG::StaticGraphic*              m_mouseover_indicator;  //!< shown when the mouse cursor is over the system
    bool                            m_selected;             //!< is this icon presently selected / should it show m_selected_indicator
    OwnerColoredSystemName*         m_colored_name;         //!< the control that holds the name of the system
    bool                            m_showing_name;         //!< is the icon supposed to show its name?

    std::map<int, FleetButton*>     m_stationary_fleet_markers; //!< the fleet buttons for the fleets that are stationary in the system, indexed by Empire ID of the owner
    std::map<int, FleetButton*>     m_moving_fleet_markers;     //!< the fleet buttons for the fleets that are under orders to move out of the system, indexed by Empire ID of the owner

    std::map<const Fleet*, boost::signals::connection>  m_fleet_state_change_signals;

    struct FleetButtonClickedFunctor
    {
        FleetButtonClickedFunctor(FleetButton& fleet_btn, SystemIcon& system_icon, bool fleet_departing);
        void operator()();
        FleetButton& m_fleet_btn;
        SystemIcon& m_system_icon;
        bool m_fleet_departing;
    };
};

#endif // _SystemIcon_h_
