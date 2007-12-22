// -*- C++ -*-
#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include "CUIWnd.h"

class CUIButton;
class ShipDesign;

/** Lets the player design ships */
class DesignWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    DesignWnd(int w, int h);
    //@}

    /** \name Mutators */ //@{
    void Reset();
    void Sanitize();

    void Render();
    //@}

private:
    void AddDesign();
    bool ValidateCurrentDesign();       ///< returns true if ship currently being designed is complete and can be added to the empire and universe

    CUIButton*  m_add_design_button;
};

#endif // _DesignWnd_h_
