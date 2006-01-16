// -*- C++ -*-
//GalaxySetupWnd.h

#ifndef _GalaxySetupWnd_h_
#define _GalaxySetupWnd_h_

#ifndef _Universe_h_
#include "../universe/Universe.h"
#endif

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif

#ifndef _CUISpin_h_
#include "CUISpin.h"
#endif

namespace GG {
    class RadioButtonGroup;
    class StaticGraphic;
    class TextControl;
    class Texture;
}
class CUIButton;
class CUIStateButton;
class EmpireColorSelector;

/** Encapsulates the galaxy setup options so that they may be reused in the GalaxySetupWnd and the MultiplayerLobbyWnd. */
class GalaxySetupPanel : public GG::Control
{
public:
    enum {DEFAULT_WIDTH = 305};

    /** \name Signal Types */ //@{
    typedef boost::signal<void ()>                               SettingsChangedSignalType; ///< emitted when the any of the settings controls changes
    typedef boost::signal<void (boost::shared_ptr<GG::Texture>)> ImageChangedSignalType;    ///< emitted when the galaxy preview image changes
    //@}
   
    /** \name Slot Types */ //@{
    typedef SettingsChangedSignalType::slot_type SettingsChangedSlotType; ///< type of functor(s) invoked on a SettingsChangedSignalType
    typedef ImageChangedSignalType::slot_type    ImageChangedSlotType;    ///< type of functor(s) invoked on a ImageChangedSignalType
    //@}

    /** \name Structors*/ //!@{
    GalaxySetupPanel(int x, int y, int w = DEFAULT_WIDTH);
    //!@}

    /** \name Accessors*/ //!@{
    int                         Systems() const;      //!< Returns the number of star systems to use in generating the galaxy
    Universe::Shape             GalaxyShape() const;  //!< Returns the shape of the galaxy
    Universe::Age               GalaxyAge() const;    //!< Returns the age of the galaxy
    Universe::StarlaneFrequency  StarlaneFrequency() const;  //!< Returns the frequency of starlanes in the galaxy
    Universe::PlanetDensity     PlanetDensity() const;      //!< Returns the density of planets within systems
    Universe::SpecialsFrequency  SpecialsFrequency() const;  //!< Returns the rarity of planetary and system specials

    boost::shared_ptr<GG::Texture> PreviewImage() const;  //!< Returns the current preview image texture

    /** encodes the values of the controls in an XMLElement with the tag "universe_params", so they can be sent to the server 
        as universe creation parameters. */
    XMLElement XMLEncode() const;

    mutable SettingsChangedSignalType SettingsChangedSignal; ///< the settings changed signal object for this GalaxySetupPanel
    mutable ImageChangedSignalType    ImageChangedSignal;    ///< the image changed signal object for this GalaxySetupPanel
    //!@}

    /** \name Mutators*/ //!@{
    virtual void Render() {}
    virtual void Disable(bool b = true);

    void SetFromXML(const XMLElement& elem); ///< sets the controls from an XMLElement created in a previous call to XMLEncode()
    //!@}

private:
    void Init();
    void AttachSignalChildren();
    void DetachSignalChildren();
    void SettingChanged(int);
    void ShapeChanged(int index);

    CUISpin<int>*         m_stars_spin;          //!< The number of stars to include in the galaxy
    CUIDropDownList*      m_galaxy_shapes_list;  //!< The possible shapes for the galaxy
    CUIDropDownList*      m_galaxy_ages_list;    //!< The possible ages for the galaxy
    CUIDropDownList*      m_starlane_freq_list;  //!< The frequency of starlanes in the galaxy
    CUIDropDownList*      m_planet_density_list; //!< The density of planets in each system
    CUIDropDownList*      m_specials_freq_list;  //!< The frequency of specials in systems and on planets
    
    std::vector<boost::shared_ptr<GG::Texture> > m_textures; //!< textures for galaxy previews
};

//! This class is the Galaxy Setup window.  It is a modal window
//! that allows the user to choose a galaxy style, size, etc.
class GalaxySetupWnd : public CUIWnd
{
public:
    /** \name Structors*/ //!@{
    GalaxySetupWnd();   //!< default ctor
    //!@}

    /** \name Accessors*/ //!@{
    /** returns true iff the dialog is finished running and it was closed with the "OK" button */
    bool                    EndedWithOk() const {return m_done && m_ended_with_ok;}

    /** returns the panel containing all the user-chosen options. */
    const GalaxySetupPanel& Panel()      const  {return *m_galaxy_setup_panel;}
    const std::string&      EmpireName() const;
    GG::Clr                 EmpireColor() const;
    //!@}

    /** \name Mutators*/ //!@{
    virtual void Render();    //!< drawing code
    virtual void Keypress (GG::Key key, Uint32 key_mods);
    //!@}

private:
    void Init();
    void AttachSignalChildren();
    void DetachSignalChildren();
    void PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image);
    void EmpireNameChanged(const std::string& name);
    void OkClicked();
    void CancelClicked();

    bool m_ended_with_ok;    //!< indicates whether or not we ended the dialog with OK or not

    GalaxySetupPanel*     m_galaxy_setup_panel;    //!< The GalaxySetupPanel that does most of the work of the dialog
    GG::TextControl*      m_empire_name_label;
    CUIEdit*              m_empire_name_edit;
    GG::TextControl*      m_empire_color_label;
    EmpireColorSelector*  m_empire_color_selector;
    GG::StaticGraphic*    m_preview_image;         //!< The galaxy shape preview image
    CUIButton*            m_ok;                    //!< OK button
    CUIButton*            m_cancel;                //!< Cancel button

    GG::Pt                m_preview_ul;
};

inline std::pair<std::string, std::string> GalaxySetupWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _GalaxySetupWnd_h_
