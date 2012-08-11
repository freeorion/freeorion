#include "SitRepPanel.h"

#include "CUIControls.h"
#include "LinkText.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "../util/OptionsDB.h"

#include <GG/DrawUtil.h>

#include <boost/lexical_cast.hpp>


namespace {
            /** Adds options related to SitRepPanel to Options DB. */
    void AddOptions(OptionsDB& db) {
        db.Add("verbose-sitrep", "OPTIONS_DB_VERBOSE_SITREP_DESC",  false,  Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

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
            } else if (link_type == VarText::PREDEFINED_DESIGN_TAG) {
                if (const ShipDesign* design = GetPredefinedShipDesign(data))
                    ClientUI::GetClientUI()->ZoomToShipDesign(design->ID());

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

    ////////////////////////////////////////////////
    // SitRepRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to display SitReps. */
    class SitRepRow : public GG::ListBox::Row {
    public:
        SitRepRow(GG::X w, GG::Y h, const SitRepEntry& sitrep) :
            GG::ListBox::Row(w, h, ""),
            m_sitrep_entry(sitrep)/*,
            m_panel(0)*/
        {
            SetName("SitRepRow");
            SetChildClippingMode(ClipToClient);
            //m_panel = new ShipDataPanel(w, h, m_ship_id);
            //push_back(m_panel);
        }

        void            SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "ShipRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            //if (!empty() && old_size != Size() && m_panel)
            //    m_panel->Resize(Size());
        }

        const SitRepEntry&  GetSitRepEntry() const { return m_sitrep_entry; }

    private:
        SitRepEntry     m_sitrep_entry;
        //ShipDataPanel*  m_panel;
    };


    GG::ListBox::Row* SitRepRow(const SitRepEntry& sitrep, GG::X ) {
        //boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        //GG::Flags<GG::TextFormat> format = GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK;
        //GG::X width = m_sitreps_lb->Width() - 8;

        //LinkText* link_text = new LinkText(GG::X0, GG::Y0, width, sitrep->GetText() + " ", font, format, ClientUI::TextColor());
        //GG::Connect(link_text->LinkClickedSignal,       &HandleLinkClick);
        //GG::Connect(link_text->LinkDoubleClickedSignal, &HandleLinkClick);
        //GG::Connect(link_text->LinkRightClickedSignal,  &HandleLinkClick);
        //GG::ListBox::Row *row = new GG::ListBox::Row(link_text->Width(), link_text->Height(), "");
        //row->push_back(link_text);
        return 0;
    }
}

SitRepPanel::SitRepPanel(GG::X x, GG::Y y, GG::X w, GG::Y h) :
    CUIWnd(UserString("SITREP_PANEL_TITLE"), x, y, w, h, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE),
    m_sitreps_lb(0),
    m_prev_turn_button(0),
    m_next_turn_button(0),
    m_last_turn_button(0),
    m_showing_turn(INVALID_GAME_TURN)
{
    Sound::TempUISoundDisabler sound_disabler;
    m_sitreps_lb = new CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1);
    m_sitreps_lb->SetStyle(GG::LIST_NOSORT);

    AttachChild(m_sitreps_lb);
    SetChildClippingMode(DontClip);

    m_prev_turn_button = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("BACK"));
    AttachChild(m_prev_turn_button);
    m_next_turn_button = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("NEXT"));
    AttachChild(m_next_turn_button);
    m_last_turn_button = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("LAST"));
    AttachChild(m_last_turn_button);

    GG::Connect(m_prev_turn_button->ClickedSignal,  &SitRepPanel::PrevClicked,  this);
    GG::Connect(m_next_turn_button->ClickedSignal,  &SitRepPanel::NextClicked,  this);
    GG::Connect(m_last_turn_button->ClickedSignal,  &SitRepPanel::LastClicked,  this);

    DoLayout();
    Hide();
}

void SitRepPanel::DoLayout() {
    GG::X BUTTON_WIDTH(ClientUI::Pts()*4);
    GG::Y BUTTON_HEIGHT = m_last_turn_button->Height();
    int PAD(3);

    GG::Pt button_ul(ClientWidth() - GG::X(INNER_BORDER_ANGLE_OFFSET) - BUTTON_WIDTH,
                     ClientHeight() - BUTTON_HEIGHT);

    m_last_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_next_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_prev_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);

    m_sitreps_lb->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(ClientWidth(), button_ul.y));
}

void SitRepPanel::KeyPress (GG::Key key, boost::uint32_t key_code_point,
                            GG::Flags<GG::ModKey> mod_keys)
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

void SitRepPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    CUIWnd::SizeMove(ul, lr);
    if (old_size != GG::Wnd::Size()) {
        DoLayout();
        Update();
    }
}

void SitRepPanel::OnClose()
{ Hide(); }

void SitRepPanel::CloseClicked()
{ ClosingSignal(); }

void SitRepPanel::PrevClicked() {
    m_showing_turn = std::max(1, m_showing_turn - 1);
    Update();
}

void SitRepPanel::NextClicked() {
    m_showing_turn = std::min(CurrentTurn(), std::max(1, m_showing_turn + 1));
    Update();
}

void SitRepPanel::LastClicked() {
    m_showing_turn = CurrentTurn();
    Update();
}

void SitRepPanel::Update() {
    m_sitreps_lb->Clear();

    if (m_showing_turn == INVALID_GAME_TURN)
        this->SetName(UserString("SITREP_PANEL_TITLE"));
    else
        this->SetName(boost::io::str(FlexibleFormat(UserString("SITREP_PANEL_TITLE_TURN")) % m_showing_turn));

    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    GG::Flags<GG::TextFormat> format = GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK;
    GG::X width = m_sitreps_lb->Width() - 8;

    // loop through sitreps and display
    for (Empire::SitRepItr sitrep_it = empire->SitRepBegin(); sitrep_it != empire->SitRepEnd(); ++sitrep_it) {
        if (!GetOptionsDB().Get<bool>("verbose-sitrep")) {
            if (!sitrep_it->Validate())
                continue;
        }
        if (m_showing_turn != INVALID_GAME_TURN && m_showing_turn != sitrep_it->GetTurn())
            continue;
        LinkText* link_text = new LinkText(GG::X0, GG::Y0, width, sitrep_it->GetText() + " ", font, format, ClientUI::TextColor());
        GG::Connect(link_text->LinkClickedSignal,       &HandleLinkClick);
        GG::Connect(link_text->LinkDoubleClickedSignal, &HandleLinkClick);
        GG::Connect(link_text->LinkRightClickedSignal,  &HandleLinkClick);
        GG::ListBox::Row *row = new GG::ListBox::Row(link_text->Width(), link_text->Height(), "");
        row->push_back(link_text);
        m_sitreps_lb->Insert(row);
    }

    if (CurrentTurn() >= 1 && m_showing_turn > 1) {
        m_prev_turn_button->Disable(false);
    } else {
        m_prev_turn_button->Disable();
    }

    if (CurrentTurn() >= 1 && m_showing_turn < CurrentTurn()) {
        m_next_turn_button->Disable(false);
        m_last_turn_button->Disable(false);
    } else {
        m_next_turn_button->Disable();
        m_last_turn_button->Disable();
    }
}

void SitRepPanel::ShowSitRepsForTurn(int turn)
{ m_showing_turn = turn; }
