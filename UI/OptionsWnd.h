// -*- C++ -*-
//OptionsWnd.h

#ifndef _OptionsWnd_h_
#define _OptionsWnd_h_

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

#ifndef _GGTextControl_h_
#include <GGTextControl.h> 
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

//! This is a screen showing license and credits
class OptionsWnd : public CUI_Wnd
{
private:
    
public:
//! \name Structors
//!@{
    OptionsWnd();   //!< default ctor
    ~OptionsWnd(); //!< default dtor

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
	void OnMusic(bool checked);   	//!< updates the client when a music setting is changed
    //void OnMusic(GG::StateButton::CheckedSignalType& CS);   	//!< updates the client when a music setting is changed

//!@}

public:
//! \name Data Members
//!@{
       bool m_end_with_done;    //!< determines whether or not we ended the dialog with OK or not
//!@}
private:
//! \name Controls
//!@{

    GG::Button*      m_done_btn;    //!< Done button
    GG::StateButton*      m_music;     //!< Music enabled/disabled

    GG::TextControl*	  m_audio_str;    //!< Audio title string
    //std::string		  m_license_str;

//!@}

private:
    void Init();    //!< Attaches children and connects signals

};//OptionsWnd

inline std::pair<std::string, std::string> OptionsWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _OptionsWnd_h_
