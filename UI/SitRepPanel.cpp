#include "SitRepPanel.h"
 
#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"
#include "LinkText.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>


namespace {
    const int    SITREP_LB_MARGIN_X = 5;
    const int    SITREP_LB_MARGIN_Y = 5;

}


SitRepPanel::SitRepPanel(int x, int y, int w, int h) : 
    CUIWnd(UserString("SITREP_PANEL_TITLE"), x, y, w, h, GG::ONTOP | GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE)
{
    TempUISoundDisabler sound_disabler;
    m_sitreps_lb = new CUIListBox(SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y,
                                  ClientWidth() - SITREP_LB_MARGIN_X, ClientHeight() - SITREP_LB_MARGIN_Y);
    m_sitreps_lb->SetStyle(GG::LB_NOSORT);

    AttachChild(m_sitreps_lb);

    Hide();
}

void SitRepPanel::KeyPress (GG::Key key, Uint32 key_mods)
{
    switch (key) {
    case GG::GGK_RETURN:
    case GG::GGK_KP_ENTER:
    case GG::GGK_ESCAPE:
    case GG::GGK_F2: {
        Hide();
        break;
    }
    default:
        break;
    }
}

void SitRepPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    CUIWnd::SizeMove(ul, lr);
    m_sitreps_lb->SizeMove(GG::Pt(SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y),
                           GG::Pt(ClientWidth() - SITREP_LB_MARGIN_X, ClientHeight() - SITREP_LB_MARGIN_Y));
    Update();
}

void SitRepPanel::OnClose()
{
    Hide();
}

void SitRepPanel::Update()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup(HumanClientApp::GetApp()->EmpireID());

    if (!empire)
        return;

    int first_visible_sitrep = m_sitreps_lb->FirstRowShown();
    m_sitreps_lb->Clear();

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS);
    Uint32 format = GG::TF_LEFT | GG::TF_WORDBREAK;
    int width = m_sitreps_lb->Width() - 8;

    // loop through sitreps and display
    for (Empire::SitRepItr sitrep_it = empire->SitRepBegin(); sitrep_it != empire->SitRepEnd(); ++sitrep_it) {
        LinkText* link_text = new LinkText(0, 0, width, (*sitrep_it)->GetText(), font, format, ClientUI::TEXT_COLOR);
        GG::ListBox::Row *row = new GG::ListBox::Row(link_text->Width(), link_text->Height(), "");
        GG::Connect(link_text->PlanetLinkSignal, &ClientUI::ZoomToPlanet, ClientUI::GetClientUI());
        GG::Connect(link_text->SystemLinkSignal, &ClientUI::ZoomToSystem, ClientUI::GetClientUI());
        GG::Connect(link_text->FleetLinkSignal, &ClientUI::ZoomToFleet, ClientUI::GetClientUI());
        GG::Connect(link_text->ShipLinkSignal, &ClientUI::ZoomToShip, ClientUI::GetClientUI());
        GG::Connect(link_text->TechLinkSignal, &ClientUI::ZoomToTech, ClientUI::GetClientUI());
        GG::Connect(link_text->EncyclopediaLinkSignal, &ClientUI::ZoomToEncyclopediaEntry, ClientUI::GetClientUI());
        row->push_back(link_text);
        m_sitreps_lb->Insert(row);                
    }

    if (!m_sitreps_lb->Empty()) {
        m_sitreps_lb->BringRowIntoView(m_sitreps_lb->NumRows() - 1);
        m_sitreps_lb->BringRowIntoView(first_visible_sitrep);
    }
}
