//CUIControls.h

#ifndef _CUIControls_h_
#define _CUIControls_h_

//include relevant stuff from GG

#ifndef _GGMenu_h_
#include "GGMenu.h"
#endif

#ifndef _GGButton_
#include "GGButton.h"
#endif

#ifndef _GGListbox_h_
#include "GGListbox.h"
#endif

//! \file All CUI_* classes are FreeOrion-style controls incorporating 
//! the visual theme the project requires.  Implementation may
//! depend on graphics and design team specifications.  They extend
//! GG controls.

class CUI_PopupMenu : public GG::PopupMenu
{
    //TODO: implementation
public:
    int Render();

}; //CUI_PopupMenu

class CUI_StateButton : public GG::StateButton
{
    //TODO: implementation
public:
    int Render();
}; //CUI_StateButton

class CUI_Button : public GG::Button
{
    //TODO: implementation
public:
    int Render();
}; //CUI_Button

class CUI_ListBox : public GG::ListBox
{
    //TODO: implementation
public:
    int Render();
}; //CUI_ListBox

#endif
