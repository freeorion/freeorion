#include "TurnProgressWnd.h"

#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"

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
    UserString("TURN_PROGRESS_WND");

    boost::shared_ptr<GG::Texture> texture00 = GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "splash00.png");
    boost::shared_ptr<GG::Texture> texture01 = GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "splash01.png");
    boost::shared_ptr<GG::Texture> texture10 = GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "splash10.png");
    boost::shared_ptr<GG::Texture> texture11 = GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "splash11.png");
    int total_width = texture00->DefaultWidth() + texture01->DefaultWidth();
    int total_height = texture00->DefaultHeight() + texture10->DefaultHeight();
    double x_scale_factor = GG::App::GetApp()->AppWidth() / static_cast<double>(total_width);
    double y_scale_factor = GG::App::GetApp()->AppHeight() / static_cast<double>(total_height);
    m_bg_graphic00 = new GG::StaticGraphic(0, 0,
                                           static_cast<int>(texture00->DefaultWidth() * x_scale_factor), static_cast<int>(texture00->DefaultHeight() * y_scale_factor),
                                           texture00, GG::GR_FITGRAPHIC);
    m_bg_graphic01 = new GG::StaticGraphic(m_bg_graphic00->LowerRight().x, 0,
                                           static_cast<int>(texture01->DefaultWidth() * x_scale_factor), static_cast<int>(texture01->DefaultHeight() * y_scale_factor),
                                           texture01, GG::GR_FITGRAPHIC);
    m_bg_graphic10 = new GG::StaticGraphic(0, m_bg_graphic00->LowerRight().y,
                                           static_cast<int>(texture10->DefaultWidth() * x_scale_factor), static_cast<int>(texture10->DefaultHeight() * y_scale_factor),
                                           texture10, GG::GR_FITGRAPHIC);
    m_bg_graphic11 = new GG::StaticGraphic(m_bg_graphic00->LowerRight().x, m_bg_graphic00->LowerRight().y,
                                           static_cast<int>(texture11->DefaultWidth() * x_scale_factor), static_cast<int>(texture11->DefaultHeight() * y_scale_factor),
                                           texture11, GG::GR_FITGRAPHIC);
    GG::App::GetApp()->Register(m_bg_graphic00);
    GG::App::GetApp()->Register(m_bg_graphic01);
    GG::App::GetApp()->Register(m_bg_graphic10);
    GG::App::GetApp()->Register(m_bg_graphic11);

    m_phase_text = new GG::TextControl(0, 20, PROGRESS_WND_WIDTH, static_cast<int>(ClientUI::PTS * 2.0 + 4), "", ClientUI::FONT,  static_cast<int>(ClientUI::PTS * 2.0), ClientUI::TEXT_COLOR, GG::TF_CENTER | GG::TF_VCENTER);

    m_empire_text = new GG::TextControl(0, 50, PROGRESS_WND_WIDTH, static_cast<int>(ClientUI::PTS * 2.0 + 4), "", ClientUI::FONT, static_cast<int>(ClientUI::PTS * 2.0), ClientUI::TEXT_COLOR, GG::TF_CENTER | GG::TF_VCENTER);

    AttachChild(m_phase_text);
    AttachChild(m_empire_text);
}

TurnProgressWnd::~TurnProgressWnd( )
{
    GG::App::GetApp()->Remove(m_bg_graphic00);
    GG::App::GetApp()->Remove(m_bg_graphic01);
    GG::App::GetApp()->Remove(m_bg_graphic10);
    GG::App::GetApp()->Remove(m_bg_graphic11);
    delete m_bg_graphic00;
    delete m_bg_graphic01;
    delete m_bg_graphic10;
    delete m_bg_graphic11;
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

