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

//! This is a screen showing license and credits
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
    virtual bool Render();    //!< drawing code
    virtual void Keypress (GG::Key key, Uint32 key_mods);
    
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
//! \name Controls
//!@{

    GG::Button*           m_done_btn;    //!< Done button
    GG::Button*           m_license;     //!< License button
    GG::Button*           m_credits;     //!< Credits button

    GG::XMLDoc 		  m_credits_doc; //!< XML document containing the project credits

    GG::MultiEdit *	  m_info;	//!< Displays the license or credits information

    std::string		  m_credits_str;    //!< String containing the credits
    std::string		  m_license_str;

//!@}

private:
    void Init();    //!< Attaches children and connects signals

};//About
#endif
