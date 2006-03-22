#include "TurnProgressWnd.h"

#include "CombatWnd.h"
#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "Splash.h"

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
    m_combat_wnd(0)
{
    SetText(UserString("TURN_PROGRESS_WND"));

    LoadSplashGraphics(m_bg_graphics);

    m_phase_text = new GG::TextControl(0, 20, Width(), ClientUI::PTS * 2 + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS * 2), ClientUI::TEXT_COLOR, GG::TF_CENTER | GG::TF_VCENTER);
    m_empire_text = new GG::TextControl(0, 50, Width(), ClientUI::PTS * 2 + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS * 2), ClientUI::TEXT_COLOR, GG::TF_CENTER | GG::TF_VCENTER);

    AttachChild(m_phase_text);
    AttachChild(m_empire_text);
}

TurnProgressWnd::~TurnProgressWnd()
{
    for (unsigned int y = 0; y < m_bg_graphics.size(); ++y) {
        for (unsigned int x = 0; x < m_bg_graphics[y].size(); ++x) {
            delete m_bg_graphics[y][x];
        }
    }
}

bool TurnProgressWnd::InWindow(const GG::Pt& pt) const
{
    return GG::Wnd::InWindow(pt) || (m_combat_wnd && m_combat_wnd->InWindow(pt));
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

void TurnProgressWnd::UpdateCombatTurnProgress(const std::string& message)
{
    if (!m_combat_wnd) {
        m_combat_wnd = new CombatWnd((Width() - CombatWnd::WIDTH) / 2, Height());
        AttachChild(m_combat_wnd);
    }
    m_combat_wnd->UpdateCombatTurnProgress(message);
}

