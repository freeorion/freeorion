#include "MultiplayerLobbyWnd.h"

#include "CUIControls.h"
#include "GGButton.h"
#include "GGDrawUtil.h"
#include "dialogs/GGFileDlg.h"
#include "GGStaticGraphic.h"
#include "GGTextControl.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Message.h"

namespace {
const int    LOBBY_WND_WIDTH = 800;
const int    LOBBY_WND_HEIGHT = 600;
const int    CONTROL_MARGIN = 5; // gap to leave between controls in the window
const int    CHAT_WIDTH = 250;
const int    RADIO_BN_HT = ClientUI::PTS + 4;
const int    RADIO_BN_SPACING = RADIO_BN_HT + 10;
const GG::Pt PREVIEW_SZ(248, 186);
GG::Pt       g_preview_ul;
const int    PREVIEW_MARGIN = 3;
}

MultiplayerLobbyWnd::MultiplayerLobbyWnd(bool host) : 
    CUI_Wnd(ClientUI::String("MPLOBBY_WINDOW_TITLE"), (GG::App::GetApp()->AppWidth() - LOBBY_WND_WIDTH) / 2, 
            (GG::App::GetApp()->AppHeight() - LOBBY_WND_HEIGHT) / 2, LOBBY_WND_WIDTH, LOBBY_WND_HEIGHT, 
            GG::Wnd::CLICKABLE | GG::Wnd::MODAL),
    m_result(false),
    m_host(host),
    m_chat_box(0),
    m_chat_input_edit(0),
    m_galaxy_size_buttons(0),
    m_galaxy_type_buttons(0),
    m_galaxy_preview_image(0),
    m_galaxy_image_file_edit(0),
    m_image_file_browse_bn(0),
    m_players_lb(0),
    m_start_game_bn(0),
    m_cancel_bn(0)
{
    int x = LeftBorder() + CONTROL_MARGIN;
    m_chat_input_edit = new CUIEdit(x, Height() - BottomBorder() - (ClientUI::PTS + 10) - CONTROL_MARGIN, CHAT_WIDTH - x, ClientUI::PTS + 10, "");
    m_chat_box = new CUIMultiEdit(x, TopBorder() + CONTROL_MARGIN, CHAT_WIDTH - x, m_chat_input_edit->UpperLeft().y - TopBorder() - 2 * CONTROL_MARGIN, "", 
                                  GG::TF_LINEWRAP | GG::MultiEdit::READ_ONLY | GG::MultiEdit::TERMINAL_STYLE);
    m_chat_box->SetMaxLinesOfHistory(250);

    m_galaxy_size_buttons = new GG::RadioButtonGroup(CHAT_WIDTH + CONTROL_MARGIN, TopBorder() + CONTROL_MARGIN);
    m_galaxy_size_buttons->AddButton(new CUIStateButton(0, 0 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_VERYSMALL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_size_buttons->AddButton(new CUIStateButton(0, 1 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_SMALL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_size_buttons->AddButton(new CUIStateButton(0, 2 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_MEDIUM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_size_buttons->AddButton(new CUIStateButton(0, 3 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_LARGE"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_size_buttons->AddButton(new CUIStateButton(0, 4 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_VERYLARGE"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_size_buttons->AddButton(new CUIStateButton(0, 5 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_ENORMOUS"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    
    m_galaxy_type_buttons = new GG::RadioButtonGroup(CHAT_WIDTH + 150 + CONTROL_MARGIN, TopBorder() + CONTROL_MARGIN);
    m_galaxy_type_buttons->AddButton(new CUIStateButton(0, 0 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_2ARM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_type_buttons->AddButton(new CUIStateButton(0, 1 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_3ARM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_type_buttons->AddButton(new CUIStateButton(0, 2 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_4ARM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_type_buttons->AddButton(new CUIStateButton(0, 3 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_CLUSTER"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_type_buttons->AddButton(new CUIStateButton(0, 4 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_ELLIPTICAL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_type_buttons->AddButton(new CUIStateButton(0, 5 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_IRREGULAR"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_galaxy_type_buttons->AddButton(new CUIStateButton(0, 6 * RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_FROMFILE"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));

    g_preview_ul = GG::Pt(Width() - RightBorder() - PREVIEW_SZ.x - CONTROL_MARGIN - PREVIEW_MARGIN, TopBorder() + CONTROL_MARGIN + PREVIEW_MARGIN);
    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_galaxy_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::SG_FITGRAPHIC);
    m_galaxy_image_file_edit = new CUIEdit(g_preview_ul.x - PREVIEW_MARGIN, m_galaxy_preview_image->LowerRight().y + CONTROL_MARGIN + PREVIEW_MARGIN, 175, 
                                           ClientUI::PTS + 10, "");
    m_image_file_browse_bn = new CUIButton(m_galaxy_image_file_edit->LowerRight().x + CONTROL_MARGIN, 
                                           m_galaxy_preview_image->LowerRight().y + CONTROL_MARGIN + PREVIEW_MARGIN, 
                                           Width() - RightBorder() - m_galaxy_image_file_edit->LowerRight().x  - 2 * CONTROL_MARGIN, ClientUI::String("BROWSE_BTN"));

    x = CHAT_WIDTH + CONTROL_MARGIN;
    int y = m_galaxy_image_file_edit->LowerRight().y + CONTROL_MARGIN;
    m_players_lb = new CUIListBox(x, y, Width() - RightBorder() - CONTROL_MARGIN - x, m_chat_input_edit->UpperLeft().y - CONTROL_MARGIN - y);

    if (m_host)
        m_start_game_bn = new CUIButton(0, 0, 125, ClientUI::String("START_GAME_BN"));
    m_cancel_bn = new CUIButton(0, 0, 125, ClientUI::String("CANCEL"));
    m_cancel_bn->MoveTo(Width() - RightBorder() - m_cancel_bn->Width() - CONTROL_MARGIN, Height() - BottomBorder() - m_cancel_bn->Height() - CONTROL_MARGIN);
    if (m_host)
        m_start_game_bn->MoveTo(m_cancel_bn->UpperLeft().x - CONTROL_MARGIN - m_start_game_bn->Width(), Height() - BottomBorder() - m_cancel_bn->Height() - CONTROL_MARGIN);

    Init();

    if (!m_host) {
        DisableControls();
    }

    HumanClientApp::GetApp()->SetLobby(this);
}

MultiplayerLobbyWnd::MultiplayerLobbyWnd(const GG::XMLElement& elem) : 
    CUI_Wnd(elem.Child("CUI_Wnd"))
{
    // TODO : implement if needed
}

MultiplayerLobbyWnd::~MultiplayerLobbyWnd()
{
    HumanClientApp::GetApp()->SetLobby(0);
}

int MultiplayerLobbyWnd::Render()
{
    CUI_Wnd::Render();
    GG::Pt image_ul = g_preview_ul + ClientUpperLeft(), image_lr = image_ul + PREVIEW_SZ;
    GG::FlatRectangle(image_ul.x - PREVIEW_MARGIN, image_ul.y - PREVIEW_MARGIN, image_lr.x + PREVIEW_MARGIN, image_lr.y + PREVIEW_MARGIN, 
                      GG::CLR_BLACK, ClientUI::WND_INNER_BORDER_COLOR, 1);
    return 1;
}

int MultiplayerLobbyWnd::Keypress(GG::Key key, Uint32 key_mods)
{
    if ((key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) && GG::App::GetApp()->FocusWnd() == m_chat_input_edit) {
        int receiver = -1; // all players by default
        std::string text = m_chat_input_edit->WindowText() + "\n";
        HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyChatMessage(HumanClientApp::GetApp()->PlayerID(), receiver, text));
        m_chat_input_edit->SetText("");
        *m_chat_box += text;
    }
    return 1;
}

void MultiplayerLobbyWnd::HandleMessage(const Message& msg)
{
    switch (msg.Type()) {
    case Message::LOBBY_UPDATE: {
        std::stringstream stream(msg.GetText());
        GG::XMLDoc doc;
        doc.ReadDoc(stream);
        if (doc.root_node.ContainsChild("sender")) {
            int sender = boost::lexical_cast<int>(doc.root_node.Child("sender").Attribute("value"));
            std::map<int, std::string>::iterator it = m_player_names.find(sender);
            *m_chat_box += (it != m_player_names.end() ? ("[" + it->second + "] ") : "[unknown] ");
            *m_chat_box += doc.root_node.Child("text").Text();
        } else if (doc.root_node.ContainsChild("abort_game")) {
            ClientUI::MessageBox(ClientUI::String("MPLOBBY_HOST_ABORTED_GAME"));
            m_result = false;
            CUI_Wnd::Close();
        } else if (doc.root_node.ContainsChild("exit_lobby")) {
            int player_id = boost::lexical_cast<int>(doc.root_node.Child("exit_lobby").Attribute("id"));
            std::string player_name = m_player_names[player_id];
            for (int i = 0; i < m_players_lb->NumRows(); ++i) {
                if (player_name == m_players_lb->GetRow(i)[0]->WindowText()) {
                    m_players_lb->Delete(i);
                    break;
                }
            }
            m_player_IDs.erase(player_name);
            m_player_names.erase(player_id);
        } else {
            if (doc.root_node.ContainsChild("galaxy_size")) {
                int galaxy_size = boost::lexical_cast<int>(doc.root_node.Child("galaxy_size").Attribute("value"));
                m_galaxy_size_buttons->SetCheck((galaxy_size - 50) / 50);
            }
            if (doc.root_node.ContainsChild("galaxy_image_filename")) {
                m_galaxy_image_file_edit->SetText(doc.root_node.Child("galaxy_image_filename").Text());
            }
            if (doc.root_node.ContainsChild("galaxy_type")) {
                m_galaxy_type_buttons->SetCheck(-1);
                m_galaxy_type_buttons->SetCheck(boost::lexical_cast<int>(doc.root_node.Child("galaxy_type").Attribute("value")));
            }
            if (doc.root_node.ContainsChild("players")) {
                m_player_IDs.clear();
                m_player_names.clear();
                m_players_lb->Clear();
                for (int i = 0; i < doc.root_node.Child("players").NumChildren(); ++i) {
                    m_player_IDs[doc.root_node.Child("players").Child(i).Tag()] = i;
                    m_player_names[i] = doc.root_node.Child("players").Child(i).Tag();
                    GG::ListBox::Row row;
                    row.push_back(m_player_names[i], ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
                    m_players_lb->Insert(row);
                }
            }
        }
        break;
    }

    case Message::GAME_START: {
        m_result = true;
        CUI_Wnd::Close();
        break;
    }

    default:
        GG::App::GetApp()->Logger().errorStream() << "MultiplayerLobbyWnd::HandleMessage : Received an unknown message type \"" << msg.Type() << "\".";
        break;
    }
}

void MultiplayerLobbyWnd::Init()
{
    AttachSignalChildren();

    Connect(m_galaxy_size_buttons->ButtonChangedSignal(), &MultiplayerLobbyWnd::GalaxySizeClicked, this);
    Connect(m_galaxy_type_buttons->ButtonChangedSignal(), &MultiplayerLobbyWnd::GalaxyTypeClicked, this);
    Connect(m_image_file_browse_bn->ClickedSignal(), &MultiplayerLobbyWnd::BrowseClicked, this);
    Connect(m_players_lb->SelChangedSignal(), &MultiplayerLobbyWnd::PlayerSelected, this);
    if (m_host)
        Connect(m_start_game_bn->ClickedSignal(), &MultiplayerLobbyWnd::StartGameClicked, this);
    Connect(m_cancel_bn->ClickedSignal(), &MultiplayerLobbyWnd::CancelClicked, this);

    // create and load textures
    m_textures.clear();
    for (int i = 0; i < ClientUniverse::GALAXY_SHAPES; ++i)
        m_textures.push_back(boost::shared_ptr<GG::Texture>(new GG::Texture()));
    m_textures[ClientUniverse::SPIRAL_2]->Load(ClientUI::ART_DIR + "gp_spiral2.png");
    m_textures[ClientUniverse::SPIRAL_3]->Load(ClientUI::ART_DIR + "gp_spiral3.png");
    m_textures[ClientUniverse::SPIRAL_4]->Load(ClientUI::ART_DIR + "gp_spiral4.png");
    m_textures[ClientUniverse::CLUSTER]->Load(ClientUI::ART_DIR + "gp_cluster.png");
    m_textures[ClientUniverse::ELLIPTICAL]->Load(ClientUI::ART_DIR + "gp_elliptical.png");
    m_textures[ClientUniverse::IRREGULAR]->Load(ClientUI::ART_DIR + "gp_irregular.png");
    
    // default settings (medium and 2-arm spiral)
    m_galaxy_size_buttons->SetCheck(2);
    m_galaxy_type_buttons->SetCheck(ClientUniverse::SPIRAL_2);
}

void MultiplayerLobbyWnd::AttachSignalChildren()
{
    AttachChild(m_chat_box);
    AttachChild(m_chat_input_edit);
    AttachChild(m_galaxy_size_buttons);
    AttachChild(m_galaxy_type_buttons);
    AttachChild(m_galaxy_preview_image);
    AttachChild(m_galaxy_image_file_edit);
    AttachChild(m_image_file_browse_bn);
    AttachChild(m_players_lb);
    AttachChild(m_start_game_bn);
    AttachChild(m_cancel_bn);
}
 
void MultiplayerLobbyWnd::DetachSignalChildren()
{
    DetachChild(m_chat_box);
    DetachChild(m_chat_input_edit);
    DetachChild(m_galaxy_size_buttons);
    DetachChild(m_galaxy_type_buttons);
    DetachChild(m_galaxy_preview_image);
    DetachChild(m_galaxy_image_file_edit);
    DetachChild(m_image_file_browse_bn);
    DetachChild(m_players_lb);
    DetachChild(m_start_game_bn);
    DetachChild(m_cancel_bn);
}

void MultiplayerLobbyWnd::GalaxySizeClicked(int idx)
{
    // disable invalid galaxy shapes for the chosen galaxy size
    switch (idx) {
    case 0:
        if (-1 < m_galaxy_type_buttons->CheckedButton() && m_galaxy_type_buttons->CheckedButton() <= ClientUniverse::SPIRAL_4)
            m_galaxy_type_buttons->SetCheck(ClientUniverse::CLUSTER);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_2);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_3);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_4);
        break;
    case 1:
        if (-1 < m_galaxy_type_buttons->CheckedButton() && 
            (m_galaxy_type_buttons->CheckedButton() == ClientUniverse::SPIRAL_3 ||
             m_galaxy_type_buttons->CheckedButton() == ClientUniverse::SPIRAL_4))
            m_galaxy_type_buttons->SetCheck(ClientUniverse::SPIRAL_2);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_2, false);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_3);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_4);
        break;
    case 2:
        if (-1 < m_galaxy_type_buttons->CheckedButton() && m_galaxy_type_buttons->CheckedButton() == ClientUniverse::SPIRAL_4)
            m_galaxy_type_buttons->SetCheck(ClientUniverse::SPIRAL_3);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_2, false);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_3, false);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_4);
        break;
    default:
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_2, false);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_3, false);
        m_galaxy_type_buttons->DisableButton(ClientUniverse::SPIRAL_4, false);
        break;
    }

    if (m_host) {
        int player_id = HumanClientApp::GetApp()->PlayerID();
        if (player_id != -1)
            HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyUpdateMessage(player_id, LobbyUpdateDoc()));
    } else {
        DisableControls();
    }
}

void MultiplayerLobbyWnd::GalaxyTypeClicked(int idx)
{
    if (m_galaxy_preview_image) {
        DeleteChild(m_galaxy_preview_image);
        m_galaxy_preview_image = 0;
    }

    if (m_galaxy_type_buttons->CheckedButton() == ClientUniverse::FROM_FILE) {
        try {
            boost::shared_ptr<GG::Texture> tex(new GG::Texture());
            tex->Load(m_galaxy_image_file_edit->WindowText());
            m_galaxy_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, tex, GG::SG_FITGRAPHIC);
            AttachChild(m_galaxy_preview_image);
        } catch (const std::exception& e) {
            GG::App::GetApp()->Logger().alert("MultiplayerLobbyWnd::GalaxyTypeClicked : Invalid texture file specified.");
        }
        m_galaxy_image_file_edit->Disable(false);
        m_image_file_browse_bn->Disable(false);
    } else {
        if (idx != -1) {
            m_galaxy_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, m_textures[idx], GG::SG_FITGRAPHIC);
            AttachChild(m_galaxy_preview_image);
        }
        m_galaxy_image_file_edit->Disable();
        m_image_file_browse_bn->Disable();
    }

    if (m_host) {
        int player_id = HumanClientApp::GetApp()->PlayerID();
        if (player_id != -1)
            HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyUpdateMessage(player_id, LobbyUpdateDoc()));
    } else {
        DisableControls();
    }
}

void MultiplayerLobbyWnd::BrowseClicked()
{
    // filter for graphics files
    std::vector<std::pair<std::string, std::string> > file_types;
    file_types.push_back(std::make_pair(ClientUI::String("GSETUP_GRAPHICS_FILES") + " (PNG, Targa, JPG, BMP)", 
                                        "*.png, *.PNG, *.tga, *.TGA, *.jpg, *.JPG, *.bmp, *.BMP"));

    GG::FileDlg dlg(m_galaxy_image_file_edit->WindowText(), false, false, file_types, ClientUI::FONT, ClientUI::PTS, GG::Clr(100, 100, 100, 255), 
                    GG::CLR_WHITE, GG::CLR_BLACK); 
    dlg.Run();    
    if (!dlg.Result().empty()) {
        m_galaxy_image_file_edit->SetText(*dlg.Result().begin());
        m_galaxy_type_buttons->SetCheck(-1);
        m_galaxy_type_buttons->SetCheck(ClientUniverse::FROM_FILE);

        if (m_host) {
            int player_id = HumanClientApp::GetApp()->PlayerID();
            if (player_id != -1)
                HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyUpdateMessage(player_id, LobbyUpdateDoc()));
        } else {
            DisableControls();
        }
    }
}

void MultiplayerLobbyWnd::PlayerSelected(const std::set<int>& selections)
{
}

void MultiplayerLobbyWnd::StartGameClicked()
{
    //check to see if we have a valid image if file is checked
    bool failed = false;
    if (m_galaxy_type_buttons->CheckedButton() == ClientUniverse::FROM_FILE) {
        try {
            boost::shared_ptr<GG::Texture> tex(new GG::Texture);
            tex->Load(m_galaxy_image_file_edit->WindowText());
        } catch (const std::exception& e) {
            ClientUI::MessageBox("\"" + m_galaxy_image_file_edit->WindowText() + "\" " + ClientUI::String("GSETUP_ERR_INVALID_GRAPHICS"));
            failed = true;
        }
    }

    if (!failed) {
        HumanClientApp::GetApp()->NetworkCore().SendMessage(HostGameMessage(HumanClientApp::GetApp()->PlayerID(), HumanClientApp::GetApp()->PlayerName()));
        m_result = true;
        CUI_Wnd::Close();
    }
}

void MultiplayerLobbyWnd::CancelClicked()
{
    GG::XMLDoc doc;
    int player_id = HumanClientApp::GetApp()->PlayerID();
    if (m_host) { // tell everyone the game is off
        doc.root_node.AppendChild(GG::XMLElement("abort_game"));
        HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyUpdateMessage(player_id, doc));
    } else { // tell everyone we've left
        doc.root_node.AppendChild(GG::XMLElement("exit_lobby"));
        HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyUpdateMessage(player_id, doc));
    }
    m_result = false;
    CUI_Wnd::Close();
}

void MultiplayerLobbyWnd::DisableControls()
{
    for (int i = 0; i < m_galaxy_size_buttons->NumButtons(); ++i)
        m_galaxy_size_buttons->DisableButton(i);
    for (int i = 0; i < m_galaxy_type_buttons->NumButtons(); ++i)
        m_galaxy_type_buttons->DisableButton(i);
    m_galaxy_image_file_edit->Disable();
    m_image_file_browse_bn->Disable();

}

GG::XMLDoc MultiplayerLobbyWnd::LobbyUpdateDoc() const
{
    GG::XMLDoc retval;
    GG::XMLElement temp("galaxy_size");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(50 * m_galaxy_size_buttons->CheckedButton() + 50));
    retval.root_node.AppendChild(temp);

    temp = GG::XMLElement("galaxy_type");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_galaxy_type_buttons->CheckedButton()));
    retval.root_node.AppendChild(temp);

    retval.root_node.AppendChild(GG::XMLElement("galaxy_image_filename", m_galaxy_image_file_edit->WindowText()));

    return retval;
}
