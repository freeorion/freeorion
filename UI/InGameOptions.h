//InGameOptions.h

#ifndef _InGameOptions_h_
#define _InGameOptions_h_

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

class InGameOptions : public CUI_Wnd
{
private:
    
public:
//! \name Structors
//!@{
    InGameOptions();   //!< default ctor
    ~InGameOptions(); //!< default dtor

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
    void OnSave();        //!< when m_save_btn button is pressed
    void OnLoad();        //!< when m_load_btn button is pressed
    void OnQuit();        //!< when m_quit_btn button is pressed
    void OnDone();        //!< when m_done_btn is pressed

//!@}

public:
//! \name Data Members
//!@{
       bool m_end_with_done;    //!< determines whether or not we ended the dialog with OK or not
       bool m_quit;		//!< If true, then quit the game
//!@}
private:
//! \name Controls
//!@{

    GG::Button*           m_save_btn;     //!< Save game button
    GG::Button*           m_load_btn;     //!< Load game button
    GG::Button*           m_done_btn;     //!< Done button
    GG::Button*           m_quit_btn;	  //!< Quit game button

//!@}

private:
    void Init();    //!< Attaches children and connects signals

};//InGameOptions
#endif
