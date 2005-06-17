#include "MultiplayerLobbyWnd.h"

#include "CUIControls.h"
#include "GGButton.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "GGTextControl.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Message.h"
#include "../util/MultiplayerCommon.h"

namespace {
    std::vector<SaveGameEmpireData> g_save_game_empire_data; ///< the save-game data for all empires in a saved game that is being started, indexed by save-game empire ID

    const int PLAYER_ROW_HEIGHT = 22;
    const int EMPIRE_NAME_WIDTH = 170;

    struct PlayerRow : GG::ListBox::Row
    {
        typedef boost::signal<void ()> DataChangedSignalType;

        PlayerRow() {}
        PlayerRow(const PlayerSetupData& player_data_) : player_data(player_data_) {}

        PlayerSetupData       player_data;
        DataChangedSignalType data_changed_sig;
    };

    struct NewGamePlayerRow : PlayerRow
    {
        NewGamePlayerRow(const std::string& player_name, const PlayerSetupData& player_data_, bool disabled) : 
            PlayerRow(player_data_)
        {
            push_back(player_name, ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
            CUIEdit* edit = new CUIEdit(0, 0, EMPIRE_NAME_WIDTH, PLAYER_ROW_HEIGHT, player_data.empire_name, ClientUI::FONT, ClientUI::PTS, GG::CLR_ZERO, ClientUI::TEXT_COLOR, GG::CLR_ZERO);
            push_back(edit);
            EmpireColorSelector* color_selector = new EmpireColorSelector(PLAYER_ROW_HEIGHT);
            color_selector->SelectColor(player_data.empire_color);
            push_back(color_selector);
            height = PLAYER_ROW_HEIGHT + 6;

            if (disabled) {
                edit->Disable();
                color_selector->Disable();
            } else {
                Connect(edit->EditedSignal(), &NewGamePlayerRow::NameChanged, this);
                Connect(color_selector->ColorChangedSignal(), &NewGamePlayerRow::ColorChanged, this);
            }
        }

    private:
        void NameChanged(const std::string& str)
        {
            player_data.empire_name = str;
            data_changed_sig();
        }
        void ColorChanged(const GG::Clr& clr)
        {
            player_data.empire_color = clr;
            data_changed_sig();
        }
    };

    struct LoadGamePlayerRow : PlayerRow
    {
        LoadGamePlayerRow(const std::string& player_name, const PlayerSetupData& player_data_, bool disabled) : 
            PlayerRow(player_data_)
        {
            push_back(player_name, ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
            CUIDropDownList* empire_list = new CUIDropDownList(0, 0, EMPIRE_NAME_WIDTH, PLAYER_ROW_HEIGHT, 5 * PLAYER_ROW_HEIGHT);
            empire_list->SetStyle(GG::LB_NOSORT);
            for (unsigned int i = 0; i < g_save_game_empire_data.size(); ++i) {
                empire_list->Insert(new CUISimpleDropDownListRow(g_save_game_empire_data[i].name));
                if (g_save_game_empire_data[i].name == player_data_.empire_name) {
                    empire_list->Select(i);
                    player_data.save_game_empire_id = g_save_game_empire_data[i].id;
                }
            }
            push_back(empire_list);
            m_color_selector = new EmpireColorSelector(PLAYER_ROW_HEIGHT);
            if (0 <= player_data.save_game_empire_id)
                player_data.empire_color = g_save_game_empire_data[player_data.save_game_empire_id].color;
            m_color_selector->SelectColor(player_data.empire_color);
            push_back(m_color_selector);
            push_back(0 <= player_data.save_game_empire_id ? g_save_game_empire_data[player_data.save_game_empire_id].player_name : "", 
                      ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
            height = PLAYER_ROW_HEIGHT + 6;

            m_color_selector->Disable();

            if (disabled) {
                empire_list->Disable();
            } else {
                Connect(empire_list->SelChangedSignal(), &LoadGamePlayerRow::EmpireChanged, this);
            }
        }

    private:
        void EmpireChanged(int i)
        {
            player_data.empire_name = g_save_game_empire_data[i].name;
            player_data.empire_color = g_save_game_empire_data[i].color;
            player_data.save_game_empire_id = g_save_game_empire_data[i].id;
            m_color_selector->SelectColor(player_data.empire_color);
            operator[](3)->SetText(g_save_game_empire_data[i].player_name);
            data_changed_sig();
        }

        EmpireColorSelector* m_color_selector;
    };

    const int    LOBBY_WND_WIDTH = 800;
    const int    LOBBY_WND_HEIGHT = 600;
    const int    CONTROL_MARGIN = 5; // gap to leave between controls in the window
    const int    GALAXY_SETUP_PANEL_WIDTH = 250;
    const int    SAVED_GAMES_LIST_ROW_HEIGHT = 22;
    const int    SAVED_GAMES_LIST_DROP_HEIGHT = 10 * SAVED_GAMES_LIST_ROW_HEIGHT;
    const int    CHAT_WIDTH = 250;
    const int    RADIO_BN_HT = ClientUI::PTS + 4;
    const int    RADIO_BN_SPACING = RADIO_BN_HT + 10;
    GG::Pt       g_preview_ul;
    const GG::Pt PREVIEW_SZ(248, 186);
    const int    PREVIEW_MARGIN = 3;

    bool temp_header_bool = RecordHeaderFile(MultiplayerLobbyWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

MultiplayerLobbyWnd::MultiplayerLobbyWnd(bool host) : 
    CUI_Wnd(UserString("MPLOBBY_WINDOW_TITLE"), (GG::App::GetApp()->AppWidth() - LOBBY_WND_WIDTH) / 2, 
            (GG::App::GetApp()->AppHeight() - LOBBY_WND_HEIGHT) / 2, LOBBY_WND_WIDTH, LOBBY_WND_HEIGHT, 
            GG::Wnd::CLICKABLE | GG::Wnd::MODAL),
    m_result(false),
    m_host(host),
    m_chat_box(0),
    m_chat_input_edit(0),
    m_new_load_game_buttons(0),
    m_galaxy_setup_panel(0),
    m_saved_games_list(0),
    m_preview_image(0),
    m_players_lb(0),
    m_start_game_bn(0),
    m_cancel_bn(0)
{
    TempUISoundDisabler sound_disabler;

    int x = LeftBorder() + CONTROL_MARGIN;
    m_chat_input_edit = new CUIEdit(x, Height() - BottomBorder() - (ClientUI::PTS + 10) - CONTROL_MARGIN, CHAT_WIDTH - x, ClientUI::PTS + 10, "");
    m_chat_box = new CUIMultiEdit(x, TopBorder() + CONTROL_MARGIN, CHAT_WIDTH - x, m_chat_input_edit->UpperLeft().y - TopBorder() - 2 * CONTROL_MARGIN, "", 
                                  GG::TF_LINEWRAP | GG::MultiEdit::READ_ONLY | GG::MultiEdit::TERMINAL_STYLE);
    m_chat_box->SetMaxLinesOfHistory(250);

    m_new_load_game_buttons = new GG::RadioButtonGroup(CHAT_WIDTH + CONTROL_MARGIN, TopBorder() + CONTROL_MARGIN);
    m_new_load_game_buttons->AddButton(new CUIStateButton(0, 0, 100, RADIO_BN_HT, UserString("NEW_GAME_BN"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));

    m_galaxy_setup_panel = new GalaxySetupPanel(CHAT_WIDTH + 2 * CONTROL_MARGIN, m_new_load_game_buttons->LowerRight().y, GALAXY_SETUP_PANEL_WIDTH);

    m_new_load_game_buttons->AddButton(new CUIStateButton(0, m_galaxy_setup_panel->LowerRight().y, 100, RADIO_BN_HT, UserString("LOAD_GAME_BN"), 
                                                          GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));

    m_saved_games_list = new CUIDropDownList(CHAT_WIDTH + 2 * CONTROL_MARGIN, m_new_load_game_buttons->LowerRight().y + CONTROL_MARGIN, 
                                             GALAXY_SETUP_PANEL_WIDTH, SAVED_GAMES_LIST_ROW_HEIGHT, SAVED_GAMES_LIST_DROP_HEIGHT);
    m_saved_games_list->SetStyle(GG::LB_NOSORT);

    g_preview_ul = GG::Pt(Width() - RightBorder() - PREVIEW_SZ.x - CONTROL_MARGIN - PREVIEW_MARGIN, TopBorder() + CONTROL_MARGIN + PREVIEW_MARGIN);
    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::GR_FITGRAPHIC);

    x = CHAT_WIDTH + CONTROL_MARGIN;
    int y = std::max(m_saved_games_list->LowerRight().y, m_preview_image->LowerRight().y) + CONTROL_MARGIN;
    m_players_lb = new CUIListBox(x, y, Width() - RightBorder() - CONTROL_MARGIN - x, m_chat_input_edit->UpperLeft().y - CONTROL_MARGIN - y);
    m_players_lb->SetStyle(GG::LB_NOSORT | GG::LB_NOSEL);

    if (m_host)
        m_start_game_bn = new CUIButton(0, 0, 125, UserString("START_GAME_BN"));
    m_cancel_bn = new CUIButton(0, 0, 125, UserString("CANCEL"));
    m_cancel_bn->MoveTo(Width() - RightBorder() - m_cancel_bn->Width() - CONTROL_MARGIN, Height() - BottomBorder() - m_cancel_bn->Height() - CONTROL_MARGIN);
    if (m_host)
        m_start_game_bn->MoveTo(m_cancel_bn->UpperLeft().x - CONTROL_MARGIN - m_start_game_bn->Width(), Height() - BottomBorder() - m_cancel_bn->Height() - CONTROL_MARGIN);

    Init();

    if (!m_host) {
        for (int i = 0; i < m_new_load_game_buttons->NumButtons(); ++i) {
            m_new_load_game_buttons->DisableButton(i);
        }
        m_galaxy_setup_panel->Disable();
        m_saved_games_list->Disable();
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

bool MultiplayerLobbyWnd::Render()
{
    CUI_Wnd::Render();
    GG::Pt image_ul = g_preview_ul + ClientUpperLeft(), image_lr = image_ul + PREVIEW_SZ;
    GG::FlatRectangle(image_ul.x - PREVIEW_MARGIN, image_ul.y - PREVIEW_MARGIN, image_lr.x + PREVIEW_MARGIN, image_lr.y + PREVIEW_MARGIN, 
                      GG::CLR_BLACK, ClientUI::WND_INNER_BORDER_COLOR, 1);
    return true;
}

void MultiplayerLobbyWnd::Keypress(GG::Key key, Uint32 key_mods)
{
    if ((key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) && GG::App::GetApp()->FocusWnd() == m_chat_input_edit) {
        int receiver = -1; // all players by default
        std::string text = m_chat_input_edit->WindowText();
        HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyChatMessage(HumanClientApp::GetApp()->PlayerID(), receiver, text));
        m_chat_input_edit->SetText("");
        *m_chat_box += m_player_names[HumanClientApp::GetApp()->PlayerID()] + ": " + text + "\n";
    } else if (!m_start_game_bn->Disabled() && (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER)) {
        StartGameClicked();
    } else if (key == GG::GGK_ESCAPE) {
        CancelClicked();
    }
}

void MultiplayerLobbyWnd::HandleMessage(const Message& msg)
{
    switch (msg.Type()) {
    case Message::LOBBY_UPDATE: {
        std::stringstream stream(msg.GetText());
        GG::XMLDoc doc;
        doc.ReadDoc(stream);
        if (doc.root_node.ContainsChild("sender")) {
            int sender = boost::lexical_cast<int>(doc.root_node.Child("sender").Text());
            std::map<int, std::string>::iterator it = m_player_names.find(sender);
            *m_chat_box += (it != m_player_names.end() ? (it->second + ": ") : "[unknown]: ");
            *m_chat_box += doc.root_node.Child("text").Text() + "\n";
        } else if (doc.root_node.ContainsChild("abort_game")) {
            ClientUI::MessageBox(UserString("MPLOBBY_HOST_ABORTED_GAME"), true);
            m_result = false;
            CUI_Wnd::CloseClicked();
        } else if (doc.root_node.ContainsChild("exit_lobby")) {
            int player_id = boost::lexical_cast<int>(doc.root_node.Child("exit_lobby").Child("id").Text());
            std::string player_name = m_player_names[player_id];
            for (int i = 0; i < m_players_lb->NumRows(); ++i) {
                if (player_name == m_players_lb->GetRow(i)[0]->WindowText()) {
                    m_players_lb->Delete(i);
                    break;
                }
            }
            m_player_IDs.erase(player_name);
            m_player_names.erase(player_id);
            if (m_host)
                m_start_game_bn->Disable(!CanStart());
        } else { // regular update
            bool loading_game = false;

            if (doc.root_node.ContainsChild("new_game")) {
                m_new_load_game_buttons->SetCheck(0);
            } else if (doc.root_node.ContainsChild("load_game")) {
                m_new_load_game_buttons->SetCheck(1);
                loading_game = true;
            }

            if (doc.root_node.ContainsChild("universe_params")) {
                m_galaxy_setup_panel->SetFromXML(doc.root_node.Child("universe_params"));
            }

            if (doc.root_node.ContainsChild("save_games")) {
                m_saved_games_list->Clear();
                std::vector<std::string> files = GG::Tokenize(doc.root_node.Child("save_games").Text());
                for (unsigned int i = 0; i < files.size(); ++i) {
                    m_saved_games_list->Insert(new CUISimpleDropDownListRow(files[i]));
                }
            }

            if (doc.root_node.ContainsChild("save_game_empire_data")) {
                g_save_game_empire_data.clear();
                for (int i = 0; i < doc.root_node.Child("save_game_empire_data").NumChildren(); ++i) {
                    g_save_game_empire_data.push_back(SaveGameEmpireData(doc.root_node.Child("save_game_empire_data").Child(i)));
                }
            }

            if (doc.root_node.ContainsChild("players")) {
                m_player_IDs.clear();
                m_player_names.clear();
                m_player_setup_data.clear();
                for (int i = 0; i < doc.root_node.Child("players").NumChildren(); ++i) {
                    int id = boost::lexical_cast<int>(doc.root_node.Child("players").Child(i).Child("id").Text());
                    m_player_IDs[doc.root_node.Child("players").Child(i).Tag()] = id;
                    m_player_names[id] = doc.root_node.Child("players").Child(i).Tag();
                    m_player_setup_data.push_back(std::make_pair(PlayerSetupData(doc.root_node.Child("players").Child(i).Child("PlayerSetupData")), id));
                }
                PopulatePlayerList(loading_game);
            }

            if (doc.root_node.ContainsChild("save_file")) {
                int save_file_idx = boost::lexical_cast<int>(doc.root_node.Child("save_file").Text());
                if (save_file_idx != m_saved_games_list->CurrentItemIndex()) {
                    m_saved_games_list->Select(save_file_idx);
                }
            }

            PopulatePlayerList(loading_game);

            if (m_host)
                m_start_game_bn->Disable(!CanStart());
        }
        break;
    }

    case Message::GAME_START: {
        m_result = true;
        CUI_Wnd::CloseClicked();
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

    if (m_host) {
        Connect(m_new_load_game_buttons->ButtonChangedSignal(), &MultiplayerLobbyWnd::NewLoadClicked, this);
        Connect(m_galaxy_setup_panel->SettingsChangedSignal(), &MultiplayerLobbyWnd::GalaxySetupPanelChanged, this);
        Connect(m_saved_games_list->SelChangedSignal(), &MultiplayerLobbyWnd::SaveGameChanged, this);
    }
    Connect(m_galaxy_setup_panel->ImageChangedSignal(), &MultiplayerLobbyWnd::PreviewImageChanged, this);
    if (m_host)
        Connect(m_start_game_bn->ClickedSignal(), &MultiplayerLobbyWnd::StartGameClicked, this);
    Connect(m_cancel_bn->ClickedSignal(), &MultiplayerLobbyWnd::CancelClicked, this);

    // default settings (new game)
    m_new_load_game_buttons->SetCheck(0);
    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());
}

void MultiplayerLobbyWnd::AttachSignalChildren()
{
    AttachChild(m_chat_box);
    AttachChild(m_chat_input_edit);
    AttachChild(m_new_load_game_buttons);
    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_saved_games_list);
    AttachChild(m_preview_image);
    AttachChild(m_players_lb);
    AttachChild(m_start_game_bn);
    AttachChild(m_cancel_bn);
}
 
void MultiplayerLobbyWnd::DetachSignalChildren()
{
    DetachChild(m_chat_box);
    DetachChild(m_chat_input_edit);
    DetachChild(m_new_load_game_buttons);
    DetachChild(m_galaxy_setup_panel);
    DetachChild(m_saved_games_list);
    DetachChild(m_preview_image);
    DetachChild(m_players_lb);
    DetachChild(m_start_game_bn);
    DetachChild(m_cancel_bn);
}

void MultiplayerLobbyWnd::NewLoadClicked(int idx)
{
    switch (idx) {
    case 0:
        m_galaxy_setup_panel->Disable(false);
        m_saved_games_list->Disable();
        PopulatePlayerList(false);
        break;
    case 1:
        m_galaxy_setup_panel->Disable();
        m_saved_games_list->Disable(false);
        PopulatePlayerList(true);
        break;
    default:
        break;
    }

    SendUpdate();
}

void MultiplayerLobbyWnd::GalaxySetupPanelChanged()
{
    SendUpdate();
}

void MultiplayerLobbyWnd::SaveGameChanged(int idx)
{
    SendUpdate();
}

void MultiplayerLobbyWnd::PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image)
{
    if (m_preview_image) {
        DeleteChild(m_preview_image);
        m_preview_image = 0;
    }
    m_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, new_image, GG::GR_FITGRAPHIC);
    AttachChild(m_preview_image);
}

void MultiplayerLobbyWnd::PlayerDataChanged()
{
    if (m_host)
        m_start_game_bn->Disable(!CanStart());
    SendUpdate();
}

void MultiplayerLobbyWnd::StartGameClicked()
{
    HumanClientApp::GetApp()->NetworkCore().SendMessage(HostGameMessage(HumanClientApp::GetApp()->PlayerID(), HumanClientApp::GetApp()->PlayerName()));
    m_result = true;
    CUI_Wnd::CloseClicked();
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
    CUI_Wnd::CloseClicked();
}

void MultiplayerLobbyWnd::PopulatePlayerList(bool loading_game)
{
    m_players_lb->Clear();

    for (unsigned int i = 0; i < m_player_setup_data.size(); ++i) {
        int id = m_player_setup_data[i].second;
        if (loading_game) {
            m_player_setup_data[i].first.save_game_empire_id = -1; // reset this, in case it has some old value that makes no sense
            LoadGamePlayerRow* row = 
                new LoadGamePlayerRow(m_player_names[i],
                                      m_player_setup_data[i].first,
                                      !m_host && id != HumanClientApp::GetApp()->PlayerID());
            m_players_lb->Insert(row);
            Connect(row->data_changed_sig, &MultiplayerLobbyWnd::PlayerDataChanged, this);
        } else {
            NewGamePlayerRow* row = 
                new NewGamePlayerRow(m_player_names[id], 
                                     m_player_setup_data[i].first,
                                     !m_host && id != HumanClientApp::GetApp()->PlayerID());
            m_players_lb->Insert(row);
            Connect(row->data_changed_sig, &MultiplayerLobbyWnd::PlayerDataChanged, this);
        }
    }

    if (loading_game) {
        m_players_lb->SetNumCols(4);
        m_players_lb->SetColAlignment(2, GG::LB_RIGHT);
        m_players_lb->SetColAlignment(3, GG::LB_RIGHT);
    } else {
        m_players_lb->SetNumCols(3);
        m_players_lb->SetColAlignment(2, GG::LB_RIGHT);
    }
}

void MultiplayerLobbyWnd::SendUpdate()
{
    int player_id = HumanClientApp::GetApp()->PlayerID();
    if (player_id != -1) {
        HumanClientApp::GetApp()->NetworkCore().SendMessage(LobbyUpdateMessage(player_id, LobbyUpdateDoc()));
    }
}

bool MultiplayerLobbyWnd::PlayerDataAcceptable() const
{
    std::set<std::string> empire_names;
    std::set<int> empire_colors;
    for (int i = 0; i < m_players_lb->NumRows(); ++i) {
        const PlayerRow& row = dynamic_cast<const PlayerRow&>(m_players_lb->GetRow(i));
        if (row.player_data.empire_name.empty())
            return false;
        empire_names.insert(row.player_data.empire_name);
        empire_colors.insert(row.player_data.empire_color.i);
    }
    return static_cast<int>(empire_names.size()) == m_players_lb->NumRows() &&
	static_cast<int>(empire_colors.size()) == m_players_lb->NumRows();
}

bool MultiplayerLobbyWnd::CanStart() const
{
    return PlayerDataAcceptable() && 1 < m_players_lb->NumRows();
}

GG::XMLDoc MultiplayerLobbyWnd::LobbyUpdateDoc() const
{
    GG::XMLDoc retval;
    retval.root_node.AppendChild(GG::XMLElement(m_new_load_game_buttons->CheckedButton() ? "load_game" : "new_game"));
    retval.root_node.AppendChild(m_galaxy_setup_panel->XMLEncode());
    retval.root_node.AppendChild(GG::XMLElement("save_file", boost::lexical_cast<std::string>(m_saved_games_list->CurrentItemIndex())));
    retval.root_node.AppendChild(GG::XMLElement("players"));
    for (int i = 0; i < m_players_lb->NumRows(); ++i) {
        const PlayerRow* row = dynamic_cast<const PlayerRow*>(&m_players_lb->GetRow(i));
        retval.root_node.LastChild().AppendChild(row->player_data.XMLEncode());
    }
    return retval;
}
