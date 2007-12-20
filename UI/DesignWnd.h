// -*- C++ -*-
#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include "CUIWnd.h"

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
};

#endif // _DesignWnd_h_
