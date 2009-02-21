// -*- C++ -*-
#ifndef _TurnProgressWnd_h_
#define _TurnProgressWnd_h_

#include <GG/Texture.h>
#include <GG/Wnd.h>


namespace GG {
  class TextControl;
  class StaticGraphic;
}

class TurnProgressWnd : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    TurnProgressWnd();
    ~TurnProgressWnd();
    //@}
  
    virtual void Render();

    void UpdateTurnProgress(const std::string& phase_str, int empire_id);

    void HideAll();
    void ShowAll();

private:
    GG::TextControl*   m_phase_text;
    GG::TextControl*   m_empire_text;
    GG::StaticGraphic* m_splash;
    GG::StaticGraphic* m_logo;
};

#endif // _TurnProgressWnd_h_
