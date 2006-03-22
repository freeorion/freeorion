// -*- C++ -*-
#ifndef _CombatWnd_h_
#define _CombatWnd_h_

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif

namespace GG {
  class TextControl;
  class StaticGraphic;
}

class CUIListBox;

class CombatWnd : public CUIWnd
{
  public:
    enum { WIDTH = 800, HEIGHT = 300}; 

    /** \name Structors */ //@{
    CombatWnd(int x,int y);
    ~CombatWnd();
    //@}

    void UpdateCombatTurnProgress(const std::string& message);

  private:
    CUIListBox*  m_combats_lb;

};

#endif // _CombatWnd_h_
