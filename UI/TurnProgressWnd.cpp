#include "TurnProgressWnd.h"

#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>


namespace {
    const int PROGRESS_WND_HEIGHT = 100;

}


////////////////////////////////////////////////
// TurnProgressWnd
////////////////////////////////////////////////
TurnProgressWnd::TurnProgressWnd() : 
    GG::Wnd(0, (GG::GUI::GetGUI()->AppHeight() - PROGRESS_WND_HEIGHT) / 2,
            GG::GUI::GetGUI()->AppWidth(), PROGRESS_WND_HEIGHT,  GG::ONTOP | GG::CLICKABLE),
    m_splash(new GG::StaticGraphic(0, 0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(),
                                   ClientUI::GetTexture(ClientUI::ArtDir() / "splash.png"),
                                   GG::GR_FITGRAPHIC)),
    m_logo(new GG::StaticGraphic(0, 0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight() / 10,
                                 ClientUI::GetTexture(ClientUI::ArtDir() / "logo.png"),
                                 GG::GR_FITGRAPHIC | GG::GR_PROPSCALE))
{
    SetText(UserString("TURN_PROGRESS_WND"));

    GG::GUI::GetGUI()->Register(m_splash);
    GG::GUI::GetGUI()->Register(m_logo);

    m_phase_text = new GG::TextControl(0, 20, Width(), ClientUI::Pts() * 2 + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts() * 2), ClientUI::TextColor(), GG::TF_CENTER | GG::TF_VCENTER);
    m_empire_text = new GG::TextControl(0, 50, Width(), ClientUI::Pts() * 2 + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts() * 2), ClientUI::TextColor(), GG::TF_CENTER | GG::TF_VCENTER);

    AttachChild(m_phase_text);
    AttachChild(m_empire_text);
}

TurnProgressWnd::~TurnProgressWnd()
{
    delete m_logo;
    delete m_splash;
}

void TurnProgressWnd::Render()
{
    GG::Pt ul = m_phase_text->UpperLeft(), lr = (m_empire_text->Empty() ? m_phase_text : m_empire_text)->LowerRight();
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_SHADOW, GG::CLR_ZERO, 0);
}

void TurnProgressWnd::UpdateTurnProgress(const std::string& phase_str, int empire_id)
{
    *m_phase_text << phase_str;
    Empire* empire = Empires().Lookup(empire_id);
    if (empire) {
        *m_empire_text << empire->Name();
        GG::Clr text_color = Empires().Lookup(empire_id)->Color();
        m_empire_text->SetColor(text_color);
    } else {
        *m_empire_text << "";
    }  
}
