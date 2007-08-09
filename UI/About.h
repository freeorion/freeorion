// -*- C++ -*-
//About.h

#ifndef _About_h_
#define _About_h_

#ifndef _GG_Wnd_h_
#include <GG/Wnd.h>
#endif

#ifndef _GG_Edit_h_
#include <GG/Edit.h>
#endif

#ifndef _GG_Button_h_
#include <GG/Button.h>
#endif

#ifndef _GG_Spin_h_
#include <GG/Spin.h>
#endif

#ifndef _GG_MultiEdit_h_
#include <GG/MultiEdit.h>
#endif

#ifndef _GG_StaticGraphic_h_
#include <GG/StaticGraphic.h>
#endif

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif

#ifndef _XMLDoc_h_
#include "../util/XMLDoc.h"
#endif

//! This is a screen showing license and vision
class About : public CUIWnd
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
    virtual void Render();    //!< drawing code
    virtual void KeyPress (GG::Key key, GG::Flags<GG::ModKey> mod_keys);
    
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
    void OnVision();   //!< when vision button is pressed

//!@}

public:
//! \name Data Members
//!@{
    bool m_end_with_done;    //!< determines whether or not we ended the dialog with OK or not
//!@}
private:
//! \name Controls
//!@{

    GG::Button*       m_done_btn;    //!< Done button
    GG::Button*       m_license;     //!< License button
    GG::Button*       m_vision;        //!< Vision button

    XMLDoc                    m_vision_doc; //!< XML document containing the project vision

    GG::MultiEdit *       m_info;       //!< Displays the license or vision information

    std::string           m_license_str;        //!< String containing the copyright license

//!@}

private:
    void Init();    //!< Attaches children and connects signals

};//About

#endif // _About_h_
