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

//! This class is the Galaxy Setup window.  It is a modal window
//! that allows the user to choose a galaxy style, size, etc.
class GalaxySetupWnd : public GG::ModalWnd
{
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

//! \name Accessors
//!@{


//!@}

public:
//! \name Event Handlers
//!@{
    void OnChangeSize(int index);    //!< when the size radio buttons are changed
    void OnChangeType(int index);    //!< when the type radio buttons are changed

    void OnOK();        //!< when OK button is pressed
    void OnCancel();    //!< when CAncel button is pressed

//!@}

public:
//! \name Data Members
//!@{
    bool m_end_with_ok;    //!< determines whether or not we ended the dialog with OK or not

//!@}
private:
//! \name Controls
//!@{
    GG::RadioButtonGroup* m_size_buttons;    //!< The radio buttons determining size
    GG::RadioButtonGroup* m_type_buttons;    //!< The radio buttons determining type
    GG::StaticGraphic*    m_preview_image;   //!< The galaxy preview image
    GG::Edit*             m_galaxy_file;     //!< The edit control holding the filename of the galaxy generation image 
    GG::Button*           m_browse_button;   //!< Button that allows browsing for a file
    GG::Button*           m_ok;              //!< OK button
    GG::Button*           m_cancel;          //!< Cancel button

//!@}

};//GalaxySetupWnd
#endif
