//GalaxySetupWnd.h

#ifndef _GalaxySetupWnd_h_
#define _GalaxySetupWnd_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGEdit_h_
#include "GGEdit.h"
#endif

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#ifndef _GGStaticGraphic_h_
#include "GGStaticGraphic.h"
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

//! This class is the Galaxy Setup window.  It is a modal window
//! that allows the user to choose a galaxy style, size, etc.
class GalaxySetupWnd : public CUI_Wnd
{
private:
    //! enum for galaxy sizes
    enum
    {
        VERY_SMALL = 0,
        SMALL      = 1,
        MEDIUM     = 2,
        LARGE      = 3,
        VERY_LARGE = 4,
        ENORMOUS   = 5
    };//enum
    
    //! enum for galaxy types
    enum
    {       
        TWO_ARM    = 0,
        THREE_ARM  = 1,
        FOUR_ARM   = 2,
        CLUSTER    = 3,        
        //more added 6/24/2003
        ELLIPTICAL = 4,
        IRREGULAR  = 5,
        FROM_FILE  = 6
    };//enum
    
public:
//! \name Structors
//!@{
    GalaxySetupWnd();   //!< default ctor
    GalaxySetupWnd(const GG::XMLElement &elem);  //!< construction from an XML element
    ~GalaxySetupWnd(); //!< default dtor

//!@}

//! \name Mutators
//!@{
    virtual int Render();    //!< drawing code
    
    virtual GG::XMLElement XMLEncode() const; //!< encode to XML
//!@}

public:
//! \name Accessors
//!@{
    //! Returns the size of the galaxy: VERY_SMALL, SMALL, etc.
    inline int GalaxySize() const {return m_size_buttons->CheckedButton();}
    int GalaxyShape() const; //!< Returns the shape of the galaxy: TWO_ARM, THREE_ARM, etc.
    std::string GalaxyFile() const; //!< Returns the filename of the image-based galaxy file if it exists

//!@}

public:
//! \name Event Handlers
//!@{
    void OnChangeSize(int index);    //!< when the size radio buttons are changed
    void OnChangeType(int index);    //!< when the type radio buttons are changed

    void OnBrowse();    //!< when Browse button is pressed
    void OnOK();        //!< when OK button is pressed
    void OnCancel();    //!< when CAncel button is pressed

//!@}

public:
//! \name Data Members
//!@{
    bool m_end_with_ok;    //!< determines whether or not we ended the dialog with OK or not

//!@}
private:
//! \name Private Constants
//!@{
    static const int GALAXY_TYPES;
    static const int TYPE_ROW_HEIGHT;

//!@}
//! \name Controls
//!@{
    GG::RadioButtonGroup* m_size_buttons;    //!< The radio buttons determining size
    GG::RadioButtonGroup* m_type_buttons;    //!< The radio buttons determining type
    GG::StaticGraphic*    m_preview_image;   //!< The galaxy preview image
    GG::Edit*             m_galaxy_file;     //!< The edit control holding the filename of the galaxy generation image 
    GG::Button*           m_browse_button;   //!< Button that allows browsing for a file
    GG::Button*           m_ok;              //!< OK button
    GG::Button*           m_cancel;          //!< Cancel button
    
    std::vector<GG::StateButton*> m_type_vector; //!< vector storing all type radio buttons
    std::vector<boost::shared_ptr<GG::Texture> > m_textures; //!< textures for galaxy previews

//!@}

private:
    void Init();    //!< Attaches children and connects signals

};//GalaxySetupWnd
#endif
