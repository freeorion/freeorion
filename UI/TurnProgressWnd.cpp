#include "TurnProgressWnd.h"

#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"

namespace {
const int PROGRESS_WND_WIDTH = 400; 
const int PROGRESS_WND_HEIGHT = 100; 

}


////////////////////////////////////////////////
// TurnProgressWnd
////////////////////////////////////////////////
TurnProgressWnd::TurnProgressWnd( ) : 
     GG::Wnd( (GG::App::GetApp()->AppWidth()-PROGRESS_WND_WIDTH)/2, (GG::App::GetApp()->AppHeight()-PROGRESS_WND_HEIGHT)/2, PROGRESS_WND_WIDTH, PROGRESS_WND_HEIGHT,  GG::Wnd::ONTOP | GG::Wnd::CLICKABLE )

{
  ClientUI::String("TURN_PROGRESS_WND");

  m_bg_graphic = new GG::StaticGraphic(0, 0, GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight(), 
                                             GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "splash01.png"));
  GG::App::GetApp()->Register(m_bg_graphic);

  m_phase_text = new GG::TextControl(0, 20, PROGRESS_WND_WIDTH, static_cast<int>(ClientUI::PTS * 2.0 + 4), "", ClientUI::FONT,  static_cast<int>(ClientUI::PTS * 2.0), GG::TF_CENTER | GG::TF_VCENTER, ClientUI::TEXT_COLOR);

  m_empire_text = new GG::TextControl(0, 50, PROGRESS_WND_WIDTH, static_cast<int>(ClientUI::PTS * 2.0 + 4), "", ClientUI::FONT, static_cast<int>(ClientUI::PTS * 2.0), GG::TF_CENTER | GG::TF_VCENTER, ClientUI::TEXT_COLOR);

  AttachChild(m_phase_text);
  AttachChild(m_empire_text);

}

TurnProgressWnd::~TurnProgressWnd( )
{
  GG::App::GetApp()->Remove(m_bg_graphic);
  delete m_bg_graphic;
}


void TurnProgressWnd::UpdateTurnProgress( const std::string& phase_str, int empire_id )
{
  *m_phase_text << phase_str;

  Empire* empire = HumanClientApp::Empires().Lookup(empire_id);

  if ( empire )
  {
    *m_empire_text << empire->Name();

    GG::Clr text_color= HumanClientApp::Empires().Lookup(empire_id)->Color();
    m_empire_text->SetColor(text_color);

  }
  else
  {
    *m_empire_text << "";
  }  

}
