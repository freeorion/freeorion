#include "TurnProgressWnd.h"

#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"

#include "CombatWnd.h"

namespace {
    const int PROGRESS_WND_WIDTH = 400; 
    const int PROGRESS_WND_HEIGHT = 100; 

    bool temp_header_bool = RecordHeaderFile(TurnProgressWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


////////////////////////////////////////////////
// TurnProgressWnd
////////////////////////////////////////////////
TurnProgressWnd::TurnProgressWnd( ) : 
     GG::Wnd( (GG::App::GetApp()->AppWidth()-PROGRESS_WND_WIDTH)/2, (GG::App::GetApp()->AppHeight()-PROGRESS_WND_HEIGHT)/2, PROGRESS_WND_WIDTH, PROGRESS_WND_HEIGHT,  GG::Wnd::ONTOP | GG::Wnd::CLICKABLE ),
      m_combat_wnd(0)

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

bool TurnProgressWnd::InWindow(const GG::Pt& pt) const
{ return GG::Wnd::InWindow(pt) || (m_combat_wnd && m_combat_wnd->InWindow(pt));}

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

void TurnProgressWnd::UpdateCombatTurnProgress(const std::string& message)
{
  if(!m_combat_wnd)
  {
    m_combat_wnd = new CombatWnd((Width()-CombatWnd::WIDTH)/2,Height()/*(Height()-CombatWnd::HEIGHT)/2*/);
    AttachChild(m_combat_wnd);
  }
  m_combat_wnd->UpdateCombatTurnProgress(message);
}

