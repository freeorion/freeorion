#include "SitRepPanel.h"

#include "CUIControls.h"
#include "LinkText.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>

#include <boost/lexical_cast.hpp>


namespace {
    const GG::X SITREP_LB_MARGIN_X(5);
    const GG::Y SITREP_LB_MARGIN_Y(5);

    void HandleLinkClick(const std::string& link_type, const std::string& data) {
        using boost::lexical_cast;
        try {
            if (link_type == VarText::PLANET_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToPlanet(lexical_cast<int>(data));
            } else if (link_type == VarText::SYSTEM_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToSystem(lexical_cast<int>(data));
            } else if (link_type == VarText::FLEET_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToFleet(lexical_cast<int>(data));
            } else if (link_type == VarText::SHIP_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToShip(lexical_cast<int>(data));
            } else if (link_type == VarText::BUILDING_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToBuilding(lexical_cast<int>(data));

            } else if (link_type == VarText::EMPIRE_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToEmpire(lexical_cast<int>(data));
            } else if (link_type == VarText::DESIGN_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToShipDesign(lexical_cast<int>(data));

            } else if (link_type == VarText::TECH_TAG) {
                ClientUI::GetClientUI()->ZoomToTech(data);
            } else if (link_type == VarText::BUILDING_TYPE_TAG) {
                ClientUI::GetClientUI()->ZoomToBuildingType(data);
            } else if (link_type == VarText::SPECIAL_TAG) {
                ClientUI::GetClientUI()->ZoomToSpecial(data);
            } else if (link_type == VarText::SHIP_HULL_TAG) {
                ClientUI::GetClientUI()->ZoomToShipHull(data);
            } else if (link_type == VarText::SHIP_PART_TAG) {
                ClientUI::GetClientUI()->ZoomToShipPart(data);
            } else if (link_type == VarText::SPECIES_TAG) {
                ClientUI::GetClientUI()->ZoomToSpecies(data);

            } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
                ClientUI::GetClientUI()->ZoomToEncyclopediaEntry(data);
            }
        } catch (const boost::bad_lexical_cast&) {
            Logger().errorStream() << "SitrepPanel.cpp HandleLinkClick caught lexical cast exception for link type: " << link_type << " and data: " << data;
        }
    }
}


SitRepPanel::SitRepPanel(GG::X x, GG::Y y, GG::X w, GG::Y h) : 
    CUIWnd(UserString("SITREP_PANEL_TITLE"), x, y, w, h, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE)
{
    Sound::TempUISoundDisabler sound_disabler;
    m_sitreps_lb = new CUIListBox(SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y,
                                  ClientWidth() - SITREP_LB_MARGIN_X, ClientHeight() - SITREP_LB_MARGIN_Y);
    m_sitreps_lb->SetStyle(GG::LIST_NOSORT);

    AttachChild(m_sitreps_lb);
    SetChildClippingMode(DontClip);

    Hide();
}

void SitRepPanel::KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
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
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    CUIWnd::SizeMove(ul, lr);
    m_sitreps_lb->SizeMove(GG::Pt(SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y),
                           GG::Pt(ClientWidth() - SITREP_LB_MARGIN_X, ClientHeight() - SITREP_LB_MARGIN_Y));
    if (Visible() && old_size != GG::Wnd::Size())
        Update();
}

void SitRepPanel::OnClose()
{
    Hide();
}

void SitRepPanel::CloseClicked()
{
    ClosingSignal();
}

void SitRepPanel::Update()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup(HumanClientApp::GetApp()->EmpireID());

    if (!empire)
        return;

    m_sitreps_lb->Clear();

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    GG::Flags<GG::TextFormat> format = GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK;
    GG::X width = m_sitreps_lb->Width() - 8;

    // loop through sitreps and display
    for (Empire::SitRepItr sitrep_it = empire->SitRepBegin(); sitrep_it != empire->SitRepEnd(); ++sitrep_it) {
        LinkText* link_text = new LinkText(GG::X0, GG::Y0, width, (*sitrep_it)->GetText() + " ", font, format, ClientUI::TextColor());
        GG::Connect(link_text->LinkClickedSignal,       &HandleLinkClick);
        GG::Connect(link_text->LinkDoubleClickedSignal, &HandleLinkClick);
        GG::Connect(link_text->LinkRightClickedSignal,  &HandleLinkClick);
        GG::ListBox::Row *row = new GG::ListBox::Row(link_text->Width(), link_text->Height(), "");
        row->push_back(link_text);
        m_sitreps_lb->Insert(row);
    }
}
