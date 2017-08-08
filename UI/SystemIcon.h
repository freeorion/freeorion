#ifndef _SystemIcon_h_
#define _SystemIcon_h_

#include <GG/GGFwd.h>
#include <GG/Control.h>

#include <boost/signals2/signal.hpp>


/** @content_tag{CTRL_SHIPYARD} Building is to be treated as a shipyard when formatting containing objects
 * 
 * For objects containing a building with this tag:
 * * Planets have an underlined name in the sidepanel
 * * Systems have their name underlined on the map and sidepanel
 */
const std::string TAG_SHIPYARD = "CTRL_SHIPYARD";

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
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
private:
    std::shared_ptr<GG::TextControl> m_text;
};

/** A control that allows interaction with a star system.  This class allows
  * user interaction with star systems on the galaxy map.  It contains the
  * graphic to display the system, along with the object ID of the
  * UniverseObject associated with it. */
class SystemIcon : public GG::Control {
public:
    //! \name Structors //!@{
    /** Construct from a universe ID at specified size and position. */
    SystemIcon(GG::X x, GG::Y y, GG::X w, int system_id);

    ~SystemIcon();
    //!@}
    void CompleteConstruction() override;

    //! \name Accessors //!@{
    /** Checks to see if point lies inside in-system fleet buttons before
        checking parent InWindow method. */
    bool InWindow(const GG::Pt& pt) const override;

    int             SystemID() const;                           //!< returns ID of system this icon represents

    /** Returns the solid star disc texture. */
    const std::shared_ptr<GG::Texture>& DiscTexture() const;

    /** Returns the transparent star halo texture. */
    const std::shared_ptr<GG::Texture>& HaloTexture() const;

    /** Returns the alternate texture shown when icon very small. */
    const std::shared_ptr<GG::Texture>& TinyTexture() const;

    GG::Pt          NthFleetButtonUpperLeft(unsigned int button_number, bool moving) const; //!< returns upper left point of moving or stationary fleetbutton number \a button_number
    int             EnclosingCircleDiameter() const;        //!< returns diameter of circle enclosing icon around which other icons can be placed and within which the mouse is over the icon
    //!@}

    //! \name Mutators //!@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Render() override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseLeave() override;

    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    void            RenderDisc();
    void            RenderHalo(double scale_factor);
    void            RenderOverlay(double zoom_factor);

    void            SetSelected(bool selected = true);   //!< shows/hides the system selection indicator over this system

    void            Refresh();                      //!< Resets system name text and calls RefreshFleetButtons().  Should be called after an icon is attached to the map

    void            ShowName();                     //!< enables the system name text
    void            HideName();                     //!< disables the system name text

    mutable boost::signals2::signal<void (int, GG::Flags< GG::ModKey > mod_keys)> MouseEnteringSignal;
    mutable boost::signals2::signal<void (int)> MouseLeavingSignal;
    mutable boost::signals2::signal<void (int)> LeftClickedSignal;
    mutable boost::signals2::signal<void (int, GG::Flags< GG::ModKey > mod_keys)> RightClickedSignal;
    mutable boost::signals2::signal<void (int)> LeftDoubleClickedSignal;
    mutable boost::signals2::signal<void (int)> RightDoubleClickedSignal;
    //!@}

private:
    void            PositionSystemName();

    int                             m_system_id;                //!< the System associated with this SystemIcon

    /** Solid star disc texture. */
    std::shared_ptr<GG::Texture> m_disc_texture;

    /** Transparent star halo texture. */
    std::shared_ptr<GG::Texture> m_halo_texture;

    /** Alternate texture shown when icon very small. */
    std::shared_ptr<GG::Texture> m_tiny_texture;

    /** Extra texture drawn over / behind system. */
    std::shared_ptr<GG::Texture> m_overlay_texture;

    double                          m_overlay_size;             //!< size of extra texture in universe units
    std::shared_ptr<GG::StaticGraphic>              m_tiny_graphic;             //!< non-scaled texture shown when zoomed far enough out;
    std::shared_ptr<RotatingGraphic>                m_selection_indicator;      //!< shown to indicate system is selected in sidepanel
    std::shared_ptr<RotatingGraphic>                m_tiny_selection_indicator; //!< non-scaled indicator shown when showing tiny graphic
    std::shared_ptr<GG::StaticGraphic>              m_mouseover_indicator;      //!< shown when the mouse cursor is over the system and the system has been explored by the client empire;
    std::shared_ptr<GG::StaticGraphic>              m_mouseover_unexplored_indicator; //!< shown when the mouse cursor is over the system and teh system is unexplored by the client empire;
    std::shared_ptr<GG::StaticGraphic>              m_tiny_mouseover_indicator; //!< non-scaled indicator shown when showing tiny graphic;
    bool                            m_selected;                 //!< is this icon presently selected / should it show m_selected_indicator
    std::shared_ptr<OwnerColoredSystemName>         m_colored_name;             //!< the control that holds the name of the system
    bool                            m_showing_name;             //!< is the icon supposed to show its name?

    boost::signals2::connection     m_system_connection;
};

#endif // _SystemIcon_h_
