// -*- C++ -*-
//GalaxySetupWnd.h

#ifndef _GalaxySetupWnd_h_
#define _GalaxySetupWnd_h_

#ifndef _Universe_h_
#include "../universe/Universe.h"
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

namespace GG {
class RadioButtonGroup;
class StaticGraphic;
class Texture;
}
class CUIButton;
class CUIEdit;
class CUIStateButton;

//! This class is the Galaxy Setup window.  It is a modal window
//! that allows the user to choose a galaxy style, size, etc.
class GalaxySetupWnd : public CUI_Wnd
{
public:
    /** \name Structors*/ //!@{
    GalaxySetupWnd();   //!< default ctor
    GalaxySetupWnd(const GG::XMLElement &elem);  //!< construction from an XML element
    virtual ~GalaxySetupWnd(); //!< dtor
    //!@}

    /** \name Accessors*/ //!@{
    bool                  EndedWithOk() const {return m_done && m_ended_with_ok;} ///< retursn true iff the dialog is finished running and it was closed with the "OK" button
    int                   Systems() const;     ///< number of star systems to use in generating the galaxy
    Universe::Shape         GalaxyShape() const; //!< Returns the shape of the galaxy: TWO_ARM, THREE_ARM, etc.
    std::string           GalaxyFile() const;  //!< Returns the filename of the image-based galaxy file if it exists

    virtual GG::XMLElement XMLEncode() const;  //!< encode to XML
    //!@}

    /** \name Mutators*/ //!@{
    virtual int Render();    //!< drawing code

    void OnChangeSize(int index);    //!< when the size radio buttons are changed
    void OnChangeType(int index);    //!< when the type radio buttons are changed

    void OnBrowse();    //!< when Browse button is pressed
    void OnOK();        //!< when OK button is pressed
    void OnCancel();    //!< when CAncel button is pressed
    //!@}

private:
    void Init();    //!< Attaches children and connects signals
    void AttachSignalChildren();
    void DetachSignalChildren();

    bool m_ended_with_ok;    //!< determines whether or not we ended the dialog with OK or not

    GG::RadioButtonGroup* m_size_buttons;    //!< The radio buttons determining size
    GG::RadioButtonGroup* m_type_buttons;    //!< The radio buttons determining type
    GG::StaticGraphic*    m_preview_image;   //!< The galaxy preview image
    CUIEdit*              m_galaxy_file;     //!< The edit control holding the filename of the galaxy generation image 
    CUIButton*            m_browse_button;   //!< Button that allows browsing for a file
    CUIButton*            m_ok;              //!< OK button
    CUIButton*            m_cancel;          //!< Cancel button
    
    std::vector<boost::shared_ptr<GG::Texture> > m_textures; //!< textures for galaxy previews
};

#endif
