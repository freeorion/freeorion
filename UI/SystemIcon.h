#ifndef _SystemIcon_h_
#define _SystemIcon_h_

#include <GG/GGFwd.h>
#include <GG/Control.h>

#include <boost/signals2/signal.hpp>


class FleetButton;
class RotatingGraphic;

/** A label like GG::Control that displays the name of a system in the
  * color(s) of the owning empire(s).  This class is derived from GG::Control
  * because GG::ListBox::Row accepts GG::Control but not GG::Wnd being added
  * to them.  OwnerColoredSystemName are added to the list of systems on the
  * SidePanel. */
class OwnerColoredSystemName : public GG::Control {
public:
    OwnerColoredSystemName(int system_id, int font_size, bool blank_unexplored_and_none);

    void CompleteConstruction() override;
    void Render() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;
private:
    std::shared_ptr<GG::TextControl> m_text;
};

/** A control that allows interaction with a star system.  This class allows
  * user interaction with star systems on the galaxy map.  It contains the
  * graphic to display the system, along with the object ID of the
  * UniverseObject associated with it. */
class SystemIcon : public GG::Control {
public:
    /** Construct from a universe ID at specified size and position. */
    SystemIcon(GG::X x, GG::Y y, GG::X w, int system_id);
    void CompleteConstruction() override;

    /** Checks to see if point lies inside in-system fleet buttons before
        checking parent InWindow method. */
    bool InWindow(GG::Pt pt) const override;

    int SystemID() const;                           //!< returns ID of system this icon represents

    /** Returns the solid star disc texture. */
    const std::shared_ptr<GG::Texture>& DiscTexture() const;

    /** Returns the transparent star halo texture. */
    const std::shared_ptr<GG::Texture>& HaloTexture() const;

    /** Returns the alternate texture shown when icon very small. */
    const std::shared_ptr<GG::Texture>& TinyTexture() const;

    GG::Pt NthFleetButtonUpperLeft(unsigned int button_number, bool moving) const; //!< returns upper left point of moving or stationary fleetbutton number \a button_number
    int    EnclosingCircleDiameter() const;        //!< returns diameter of circle enclosing icon around which other icons can be placed and within which the mouse is over the icon

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Render() override;

    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    void RenderDisc();
    void RenderHalo(double scale_factor);
    void RenderOverlay(double zoom_factor);

    void SetSelected(bool selected = true);   //!< shows/hides the system selection indicator over this system

    void Refresh();                      //!< Resets system name text and calls RefreshFleetButtons().  Should be called after an icon is attached to the map

    void ShowName();                     //!< enables the system name text
    void HideName();                     //!< disables the system name text

    mutable boost::signals2::signal<void (int, GG::Flags< GG::ModKey > mod_keys)> MouseEnteringSignal;
    mutable boost::signals2::signal<void (int)> MouseLeavingSignal;
    mutable boost::signals2::signal<void (int)> LeftClickedSignal;
    mutable boost::signals2::signal<void (int, GG::Flags< GG::ModKey > mod_keys)> RightClickedSignal;
    mutable boost::signals2::signal<void (int)> LeftDoubleClickedSignal;
    mutable boost::signals2::signal<void (int)> RightDoubleClickedSignal;

private:
    void PositionSystemName(int pts);

    std::shared_ptr<GG::Texture> m_disc_texture;    //!< Solid star disc texture
    std::shared_ptr<GG::Texture> m_halo_texture;    //!< Transparent star halo texture
    std::shared_ptr<GG::Texture> m_tiny_texture;    //!< Alternate texture shown when icon very small
    std::shared_ptr<GG::Texture> m_overlay_texture; //!< Extra texture drawn over / behind system

    int                                 m_system_id = -1;                   //!< the System associated with this SystemIcon
    double                              m_overlay_size = 1.0;               //!< size of extra texture in universe units
    std::shared_ptr<GG::StaticGraphic>  m_tiny_graphic;                     //!< non-scaled texture shown when zoomed far enough out;
    std::shared_ptr<RotatingGraphic>    m_selection_indicator;              //!< shown to indicate system is selected in sidepanel
    std::shared_ptr<RotatingGraphic>    m_tiny_selection_indicator;         //!< non-scaled indicator shown when showing tiny graphic
    std::shared_ptr<GG::StaticGraphic>  m_mouseover_indicator;              //!< shown when the mouse cursor is over the system and the system has been explored by the client empire;
    std::shared_ptr<GG::StaticGraphic>  m_mouseover_unexplored_indicator;   //!< shown when the mouse cursor is over the system and teh system is unexplored by the client empire;
    std::shared_ptr<GG::StaticGraphic>  m_tiny_mouseover_indicator;         //!< non-scaled indicator shown when showing tiny graphic;
    bool                                m_selected = false;                 //!< is this icon presently selected / should it show m_selected_indicator
    bool                                m_showing_name = false;             //!< is the icon supposed to show its name?
    boost::signals2::scoped_connection  m_system_connection;

    std::map<int, std::shared_ptr<OwnerColoredSystemName>>  m_colored_names;//!< the controls that hold the name of the system at various font point sizes
};


#endif
