//EmpireSelect.h

#ifndef _EmpireSelect_h_
#define _EmpireSelect_h_

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

#ifndef _GGStaticGraphic_h_
#include "GGStaticGraphic.h"
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

//! This class is the Empire Selection window.  It is a modal window
//! that allows the user to choose a name, color, etc.
class EmpireSelect : public CUI_Wnd
{
private:
    
public:
//! \name Structors
//!@{
    EmpireSelect();   //!< default ctor
    ~EmpireSelect(); //!< default dtor

//!@}

//! \name Mutators
//!@{
    virtual bool Render();    //!< drawing code
    
//!@}

public:
//! \name Accessors
//!@{
    std::string EmpireName() const; //!< Returns the arbitrarily chosen name of the empire
    int EmpireColor();	//!< Returns the color player selected for his/her empire

//!@}

public:
//! \name Event Handlers
//!@{
    void OnOK();        //!< when OK button is pressed
    void OnCancel();    //!< when Cancel button is pressed
    void OnLeftArrow(); //!< when left arrow is pressed
    void OnRightArrow();//!< when right arrow is pressed

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

//!@}
//! \name Controls
//!@{

    GG::Button*           m_ok;              //!< OK button
    GG::Button*           m_cancel;          //!< Cancel button

    GG::Button*           m_left_select;     //!< Select previous empire color
    GG::Button*           m_right_select;    //!< Select next empire color

    GG::Edit*             m_empire_name;     //!< The edit control to enter the chosen arbitrary name of the empire 
    GG::Texture           m_arrows;          //!< Arrow images for selector button
    GG::SubTexture*       m_la_sub;          //!< Left arrow subtexture
    GG::SubTexture*       m_la_hover_sub;    //!< Left arrow subtexture when mouse hovers over
    GG::SubTexture*       m_la_pressed_sub;  //!< Left arrow subtexture when mouse is pressed
    GG::SubTexture*       m_ra_sub;          //!< Right arrow subtexture
    GG::SubTexture*       m_ra_hover_sub;    //!< Right arrow subtexture when mouse hovers over
    GG::SubTexture*       m_ra_pressed_sub;  //!< Right arrow subtexture when mouse is pressed

    int                   m_cur_color;       //!< Currently selected color
    
//!@}

private:
    void Init();    //!< Attaches children and connects signals
    GG::Clr SelectColor(int colnum);	//!< Takes a number and returns a color for that number

};//EmpireSelect
#endif
