// -*- C++ -*-
#ifndef _About_h_
#define _About_h_

#include <GG/Wnd.h>
#include <GG/Button.h>
#include <GG/MultiEdit.h>
#include "CUIWnd.h"
#include "../util/XMLDoc.h"

//! This is a screen showing license and vision
class About : public CUIWnd {
public:
//! \name Structors
//!@{
    About();
    ~About();
//!@}

//! \name Mutators
//!@{
    virtual void Render();    //!< drawing code
    virtual void KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);
//!@}

//! \name Event Handlers
//!@{
    void OnDone();      //!< when OK button is pressed
    void OnLicense();   //!< when license button is pressed
    void OnVision();    //!< when vision button is pressed
//!@}

//! \name Data Members
//!@{
    bool m_end_with_done;    //!< determines whether or not we ended the dialog with OK or not
//!@}

private:
//! \name Controls
//!@{
    GG::Button*     m_done_btn;     //!< Done button
    GG::Button*     m_license;      //!< License button
    GG::Button*     m_vision;       //!< Vision button
    XMLDoc          m_vision_doc;   //!< XML document containing the project vision
    GG::MultiEdit*  m_info;         //!< Displays the license or vision information
    std::string     m_license_str;  //!< String containing the copyright license

    void Init();    //!< Attaches children and connects signals
};

#endif // _About_h_
