// -*- C++ -*-
#ifndef _TurnProgressWnd_h_
#define _TurnProgressWnd_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGSignalsAndSlots_h_
#include "GGSignalsAndSlots.h"
#endif

#ifndef _GGTexture_h_
#include "GGTexture.h"
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
    TurnProgressWnd( );
    //@}
    ~TurnProgressWnd();
  
    void UpdateTurnProgress( const std::string& phase_str, const int empire_id );
    void UpdateCombatTurnProgress(const std::string& message);
    
    bool InWindow(const GG::Pt& pt) const;

private:

    GG::TextControl*   m_phase_text;
    GG::TextControl*   m_empire_text;
    GG::StaticGraphic* m_bg_graphic00; //!< one tile of the background image 
    GG::StaticGraphic* m_bg_graphic01; //!< one tile of the background image 
    GG::StaticGraphic* m_bg_graphic10; //!< one tile of the background image 
    GG::StaticGraphic* m_bg_graphic11; //!< one tile of the background image 
    
    CombatWnd*         m_combat_wnd;
};

inline std::pair<std::string, std::string> TurnProgressWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _TurnProgressWnd_h_
