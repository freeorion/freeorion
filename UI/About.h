// -*- C++ -*-
#ifndef _About_h_
#define _About_h_

#include <GG/Button.h>
#include <GG/MultiEdit.h>
#include "CUIWnd.h"

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
    virtual void KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);
//!@}

//! \name Event Handlers
//!@{
    void OnDone();      //!< when 'Done' button is pressed
    void OnLicense();   //!< when 'License' button is pressed
    void OnVision();    //!< when 'Vision' button is pressed
//!@}

private:
    void DoLayout(void);

//! \name Controls
//!@{
    GG::Button*     m_done_btn;     //!< Done button
    GG::Button*     m_license;      //!< License button
    GG::Button*     m_vision;       //!< Vision button
    GG::MultiEdit*  m_info;         //!< Displays the license or vision information
    std::string     m_license_str;  //!< String containing the copyright license
};

#endif // _About_h_
