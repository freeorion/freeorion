// -*- C++ -*-
#ifndef _TurnProgressWnd_h_
#define _TurnProgressWnd_h_

#ifndef _GG_Wnd_h_
#include <GG/Wnd.h>
#endif

#ifndef _GG_SignalsAndSlots_h_
#include <GG/SignalsAndSlots.h>
#endif

#ifndef _GG_Texture_h_
#include <GG/Texture.h>
#endif


namespace GG {
  class TextControl;
  class StaticGraphic;
}

class CombatWnd;

class TurnProgressWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    TurnProgressWnd();
    ~TurnProgressWnd();
    //@}
  
    void UpdateTurnProgress(const std::string& phase_str, const int empire_id);
    void UpdateCombatTurnProgress(const std::string& message);
    
    virtual bool InWindow(const GG::Pt& pt) const;
    virtual void Render();

private:
    GG::TextControl*   m_phase_text;
    GG::TextControl*   m_empire_text;
    CombatWnd*         m_combat_wnd;
    std::vector<std::vector<GG::StaticGraphic*> > m_bg_graphics;
};

inline std::string TurnProgressWndRevision()
{return "$Id$";}

#endif // _TurnProgressWnd_h_
