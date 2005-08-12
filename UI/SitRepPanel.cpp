#include "SitRepPanel.h"
 
#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "LinkText.h"
#include "../util/MultiplayerCommon.h"

namespace {
    const int    SITREP_LB_MARGIN_X = 5;
    const int    SITREP_LB_MARGIN_Y = 5;

    bool temp_header_bool = RecordHeaderFile(SitRepPanelRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


SitRepPanel::SitRepPanel(int x, int y, int w, int h) : 
    CUI_Wnd(UserString("SITREP_PANEL_TITLE"), x, y, w, h, GG::Wnd::ONTOP | GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::RESIZABLE)
{
    TempUISoundDisabler sound_disabler;
    m_sitreps_lb = new CUIListBox(LeftBorder() + SITREP_LB_MARGIN_X, TopBorder() + SITREP_LB_MARGIN_Y,
                                  w - (LeftBorder() + SITREP_LB_MARGIN_X), h - (TopBorder() + SITREP_LB_MARGIN_Y));
    m_sitreps_lb->SetStyle(GG::LB_NOSORT);

    AttachChild(m_sitreps_lb);

    Hide();
}

void SitRepPanel::Keypress (GG::Key key, Uint32 key_mods)
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

void SitRepPanel::SizeMove(int x1, int y1, int x2, int y2)
{
    CUI_Wnd::SizeMove(x1, y1, x2, y2);
    m_sitreps_lb->SizeMove(LeftBorder() + SITREP_LB_MARGIN_X, TopBorder() + SITREP_LB_MARGIN_Y,
                           Width() - (LeftBorder() + SITREP_LB_MARGIN_X), Height() - (TopBorder() + SITREP_LB_MARGIN_Y));
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

    boost::shared_ptr<GG::Font> font = GG::App::GetApp()->GetFont(ClientUI::FONT, ClientUI::PTS);
    Uint32 format = GG::TF_LEFT | GG::TF_WORDBREAK;
    int width = m_sitreps_lb->Width() - 8;

    // loop through sitreps and display
    for (Empire::SitRepItr sitrep_it = empire->SitRepBegin(); sitrep_it != empire->SitRepEnd(); ++sitrep_it) {
        GG::ListBox::Row *row = new GG::ListBox::Row;
        LinkText* link_text = new LinkText(0, 0, width, (*sitrep_it)->GetText(), font, format, ClientUI::TEXT_COLOR);
        GG::Connect(link_text->PlanetLinkSignal, &ClientUI::ZoomToPlanet, ClientUI::GetClientUI());
        GG::Connect(link_text->SystemLinkSignal, &ClientUI::ZoomToSystem, ClientUI::GetClientUI());
        GG::Connect(link_text->FleetLinkSignal, &ClientUI::ZoomToFleet, ClientUI::GetClientUI());
        GG::Connect(link_text->ShipLinkSignal, &ClientUI::ZoomToShip, ClientUI::GetClientUI());
        GG::Connect(link_text->TechLinkSignal, &ClientUI::ZoomToTech, ClientUI::GetClientUI());
        GG::Connect(link_text->EncyclopediaLinkSignal, &ClientUI::ZoomToEncyclopediaEntry, ClientUI::GetClientUI());
        row->height = font->TextExtent(link_text->WindowText(), format, width).y;
        row->push_back(link_text);
        m_sitreps_lb->Insert(row);                
    }

    if (!m_sitreps_lb->Empty()) {
        m_sitreps_lb->BringRowIntoView(m_sitreps_lb->NumRows() - 1);
        m_sitreps_lb->BringRowIntoView(first_visible_sitrep);
    }
}
