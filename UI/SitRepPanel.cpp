#include "SitRepPanel.h"

#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"
#include "GGDrawUtil.h"

namespace {
const int    SITREP_LB_MARGIN_X = 10;
const int    SITREP_LB_MARGIN_Y = 45;
const int    SITREP_LB_HEIGHT = 200;
const int    SITREP_CLOSE_MARGIN_Y = 5;
const int    SITREP_CLOSE_WIDTH = 60;
const int    SITREP_TITLE_MARGIN_Y = 15;
}


SitRepPanel::SitRepPanel(int x, int y, int w, int h) : 
    CUI_Wnd(ClientUI::String("SITREP_PANEL_TITLE"), x, y, w, h, GG::Wnd::ONTOP | GG::Wnd::CLICKABLE | GG::Wnd::RESIZABLE ),
    m_title(new GG::TextControl( 0, SITREP_TITLE_MARGIN_Y, w, static_cast<int>(ClientUI::PTS * 1.75 + 4), ClientUI::String("SITREP_PANEL_TITLE"), ClientUI::FONT, static_cast<int>(ClientUI::PTS * 1.75), GG::TF_CENTER | GG::TF_VCENTER, ClientUI::TEXT_COLOR) )
{
    AttachChild(m_title);

    m_sitreps_lb = new CUIListBox( SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y, w-(SITREP_LB_MARGIN_X*2), SITREP_LB_HEIGHT );
    m_sitreps_lb->SetStyle(GG::LB_NOSORT);

    AttachChild(m_sitreps_lb);

    // create buttons
   m_close = new CUIButton( w-SITREP_LB_MARGIN_X-SITREP_CLOSE_WIDTH, SITREP_LB_MARGIN_Y+SITREP_LB_HEIGHT+SITREP_CLOSE_MARGIN_Y, SITREP_CLOSE_WIDTH, ClientUI::String("SITREP_PANEL_CLOSE" ) );
    
    //attach buttons
    AttachChild(m_close);

    //connect signals and slots
    GG::Connect(m_close->ClickedSignal(), &SitRepPanel::OnClose, this);

    Hide();
}


void SitRepPanel::OnClose( )
{
  Hide();
}


void SitRepPanel::Update( )
{
  Empire *pEmpire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->PlayerID() );

  m_sitreps_lb->Clear();

  // loop through sitreps and display
  for ( Empire::ConstSitRepItr sitrep_it = pEmpire->SitRepBegin(); sitrep_it != pEmpire->SitRepEnd(); ++sitrep_it )
  {
    GG::ListBox::Row *row = new GG::ListBox::Row;
    row->push_back( (*sitrep_it)->m_text, ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    m_sitreps_lb->Insert(row);                
  }
  

  Show( );
}
