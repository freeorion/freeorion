#include "TurnProgressWnd.h"

#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>


////////////////////////////////////////////////
// TurnProgressWnd
////////////////////////////////////////////////
TurnProgressWnd::TurnProgressWnd() : 
    GG::Wnd(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(),
            GG::ONTOP | GG::INTERACTIVE),
    m_splash(new GG::StaticGraphic(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(),
                                   ClientUI::GetTexture(ClientUI::ArtDir() / "splash.png"),
                                   GG::GRAPHIC_FITGRAPHIC, GG::INTERACTIVE)),
    m_logo(new GG::StaticGraphic(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight() / 10,
                                 ClientUI::GetTexture(ClientUI::ArtDir() / "logo.png"),
                                 GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE))
{
    SetName(UserString("TURN_PROGRESS_WND"));

    m_splash->AttachChild(m_logo);
    GG::GUI::GetGUI()->Register(m_splash);

    GG::Y text_top = (GG::GUI::GetGUI()->AppHeight() - 100) / 2;

    m_phase_text = new GG::TextControl(GG::X0, text_top + GG::Y(20), Width(), GG::Y(ClientUI::Pts() * 2 + 4), "", ClientUI::GetFont(ClientUI::Pts() * 2), ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
    m_empire_text = new GG::TextControl(GG::X0, text_top + GG::Y(50), Width(), GG::Y(ClientUI::Pts() * 2 + 4), "", ClientUI::GetFont(ClientUI::Pts() * 2), ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);

    AttachChild(m_phase_text);
    AttachChild(m_empire_text);
}

TurnProgressWnd::~TurnProgressWnd()
{ delete m_splash; }

void TurnProgressWnd::Render()
{
    GG::Pt ul = m_phase_text->UpperLeft(), lr = (m_empire_text->Empty() ? m_phase_text : m_empire_text)->LowerRight();
    GG::FlatRectangle(ul, lr, GG::CLR_SHADOW, GG::CLR_ZERO, 0);
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

void TurnProgressWnd::HideAll()
{
    Hide();
    GG::GUI::GetGUI()->Remove(m_splash);
}

void TurnProgressWnd::ShowAll()
{
    Show();
    GG::GUI::GetGUI()->Register(m_splash);
}
