#include "SitRepPanel.h"
 
#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "LinkText.h"
#include "../util/MultiplayerCommon.h"

namespace {
    const int    SITREP_LB_MARGIN_X = 10;
    const int    SITREP_LB_MARGIN_Y = 45;
    const int    SITREP_LB_HEIGHT = 200;
    const int    SITREP_CLOSE_MARGIN_Y = 5;
    const int    SITREP_CLOSE_WIDTH = 60;
    const int    SITREP_TITLE_MARGIN_Y = 15;

    bool temp_header_bool = RecordHeaderFile(SitRepPanelRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


SitRepPanel::SitRepPanel(int x, int y, int w, int h) : 
    CUI_Wnd(ClientUI::String("SITREP_PANEL_TITLE"), x, y, w, h, GG::Wnd::ONTOP | GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE/* | CUI_Wnd::MINIMIZABLE*/),
    m_title(new GG::TextControl(0, SITREP_TITLE_MARGIN_Y, w, static_cast<int>(ClientUI::PTS * 1.75 + 4), ClientUI::String("SITREP_PANEL_TITLE"), 
                                ClientUI::FONT, static_cast<int>(ClientUI::PTS * 1.75), ClientUI::TEXT_COLOR, GG::TF_CENTER | GG::TF_VCENTER))
{
    TempUISoundDisabler sound_disabler;

    AttachChild(m_title);

    m_sitreps_lb = new CUIListBox(SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y, w - (SITREP_LB_MARGIN_X * 2), SITREP_LB_HEIGHT);
    m_sitreps_lb->SetStyle(GG::LB_NOSORT);

    AttachChild(m_sitreps_lb);

    // create buttons
    m_close = new CUIButton(w - SITREP_LB_MARGIN_X - SITREP_CLOSE_WIDTH, SITREP_LB_MARGIN_Y + SITREP_LB_HEIGHT + SITREP_CLOSE_MARGIN_Y, SITREP_CLOSE_WIDTH, 
                            ClientUI::String("CLOSE"));

    //attach buttons
    AttachChild(m_close);

    //connect signals and slots
    GG::Connect(m_close->ClickedSignal(), &SitRepPanel::OnClose, this);

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

void SitRepPanel::OnClose()
{
    Hide();
}

void SitRepPanel::Update()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup(HumanClientApp::GetApp()->EmpireID());

    m_sitreps_lb->Clear();

    // loop through sitreps and display
    for (Empire::SitRepItr sitrep_it = empire->SitRepBegin(); sitrep_it != empire->SitRepEnd(); ++sitrep_it) {
        GG::ListBox::Row *row = new GG::ListBox::Row;
        LinkText* link_text = new LinkText(0, 0, (*sitrep_it)->GetText(), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
        GG::Connect(link_text->PlanetLinkSignal(), &ClientUI::ZoomToPlanet, ClientUI::GetClientUI());
        GG::Connect(link_text->SystemLinkSignal(), &ClientUI::ZoomToSystem, ClientUI::GetClientUI());
        GG::Connect(link_text->FleetLinkSignal(), &ClientUI::ZoomToFleet, ClientUI::GetClientUI());
        GG::Connect(link_text->ShipLinkSignal(), &ClientUI::ZoomToShip, ClientUI::GetClientUI());
        GG::Connect(link_text->TechLinkSignal(), &ClientUI::ZoomToTech, ClientUI::GetClientUI());
        GG::Connect(link_text->EncyclopediaLinkSignal(), &ClientUI::ZoomToEncyclopediaEntry, ClientUI::GetClientUI());
        row->push_back(link_text);
        m_sitreps_lb->Insert(row);                
    }
}
