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

class TurnProgressWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    TurnProgressWnd( );
    //@}
    ~TurnProgressWnd();

    void UpdateTurnProgress( const std::string& phase_str, const int empire_id );

private:

    GG::TextControl*  m_phase_text;
    GG::TextControl*  m_empire_text;
    GG::StaticGraphic* m_bg_graphic; //!< the background image 
    
};

#endif // _TurnProgressWnd_h_
