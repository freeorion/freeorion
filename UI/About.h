//About.h

#ifndef _About_h_
#define _About_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGEdit_h_
#include "GGEdit.h"
#endif

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#ifndef __GGSpin_h_
#include "GGSpin.h"
#endif

#ifndef __GGMultiEdit_h_
#include "GGMultiEdit.h"
#endif

#ifndef _GGStaticGraphic_h_
#include "GGStaticGraphic.h"
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

//! This class is the Empire Selection window.  It is a modal window
//! that allows the user to choose a name, color, etc.
class About : public CUI_Wnd
{
private:
    
public:
//! \name Structors
//!@{
    About();   //!< default ctor
    ~About(); //!< default dtor

//!@}

//! \name Mutators
//!@{
    virtual int Render();    //!< drawing code
    
//!@}

public:
//! \name Accessors
//!@{

//!@}

public:
//! \name Event Handlers
//!@{
    void OnDone();        //!< when OK button is pressed
    void OnLicense();   //!< when license button is pressed
    void OnCredits();   //!< when credits button is pressed

//!@}

public:
//! \name Data Members
//!@{
       bool m_end_with_done;    //!< determines whether or not we ended the dialog with OK or not
//!@}
private:
//! \name Private Constants
//!@{
    static const int GALAXY_TYPES;

//!@}
//! \name Controls
//!@{

    GG::Button*           m_done_btn;              //!< Done button
    GG::Button*           m_license;     //!< License button
    GG::Button*           m_credits;     //!< Credits button

    GG::XMLDoc 		  m_credits_doc; //!< XML document containing the project credits

    GG::MultiEdit *	  m_info;	//!< Displays the license or credits information

    std::string		  m_credits_str;    //!< String containing the credits
    std::string		  m_license_str;

    int                   m_cur_color;       //!< Currently selected color
    
//!@}

private:
    void Init();    //!< Attaches children and connects signals

};//About
#endif
