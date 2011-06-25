#include "MultiplayerLobbyWnd.h"

#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Message.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Serialize.h"

#include <GG/Layout.h>

#if defined(_MSC_VER)
  // HACK! this keeps VC 7.x from barfing when it sees "typedef __int64 int64_t;"
  // in boost/cstdint.h when compiling under windows
#  if defined(int64_t)
#    undef int64_t
#  endif
#elif defined(WIN32)
  // HACK! this keeps gcc 3.x from barfing when it sees "typedef long long uint64_t;"
  // in boost/cstdint.h when compiling under windows
#  define BOOST_MSVC -1
#endif

#include <boost/cast.hpp>
#include <boost/serialization/vector.hpp>

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>


namespace {
    const GG::Y PLAYER_ROW_HEIGHT(22);
    const GG::Y ROW_HEIGHT_PAD(6);
    const GG::X EMPIRE_NAME_WIDTH(150);

    // Shows information about a single player in the mulitplayer lobby.
    // This inclues whether the player is a human or AI player, or an observer,
    // the player's name, empire's name and colour, and the starting species.
    // Can also be used to allow some of that information to be changed by
    // players or the host.
    struct PlayerRow : GG::ListBox::Row {
        PlayerRow() :
            m_player_data(),
            m_player_id(Networking::INVALID_PLAYER_ID)
        {}
        PlayerRow(const PlayerSetupData& player_data, int player_id) :
            m_player_data(player_data),
            m_player_id(player_id)
        {}

        PlayerSetupData         m_player_data;
        int                     m_player_id;
        boost::signal<void ()>  DataChangedSignal;
    };

    // indicates and allows manipulation of player type
    class TypeSelector : public CUIDropDownList {
    private:
        // fills player type selection droplist in player row
        class TypeRow : public GG::DropDownList::Row {
        public:
            TypeRow() :
                GG::DropDownList::Row(),
                type(Networking::INVALID_CLIENT_TYPE)
            {
                push_back(UserString("NO_PLAYER"), ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());
            }
            TypeRow(Networking::ClientType type_, bool show_add_drop = false) :
                GG::DropDownList::Row(GG::X1, GG::Y1, "PlayerTypeSelectorRow"),
                type(type_)
            {
                switch (type) {
                case Networking::CLIENT_TYPE_AI_PLAYER:
                    if (show_add_drop)
                        push_back(UserString("ADD_AI_PLAYER"), ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());
                    else
                        push_back(UserString("AI_PLAYER"), ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());
                    break;
                case Networking::CLIENT_TYPE_HUMAN_OBSERVER:
                    push_back(UserString("OBSERVER"), ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());
                    break;
                case Networking::CLIENT_TYPE_HUMAN_PLAYER:
                    push_back(UserString("HUMAN_PLAYER"), ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());
                    break;
                default:
                    if (show_add_drop)
                        push_back(UserString("DROP_PLAYER"), ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());
                    else
                        push_back(UserString("NO_PLAYER"), ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());
                }
            }

            Networking::ClientType type;
        };

    public:
        TypeSelector() :
            CUIDropDownList(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::Y1)
        {}

        TypeSelector(GG::X w, GG::Y h, Networking::ClientType client_type, bool disabled) :
            CUIDropDownList(GG::X0, GG::Y0, w, std::max(GG::Y1, h - 8), h)
        {
            SetStyle(GG::LIST_NOSORT);
            if (client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
                if (disabled) {
                    // For AI players on non-hosts, have "AI" shown (disabled)
                    Insert(new TypeRow(Networking::CLIENT_TYPE_AI_PLAYER));      // static "AI" display
                    Select(0);
                } else {
                    // For AI players on host, have "AI" shown on droplist, with "Drop" shown as alternate selection to remove the AI
                    Insert(new TypeRow(Networking::CLIENT_TYPE_AI_PLAYER));      // "AI" display
                    Insert(new TypeRow(Networking::INVALID_CLIENT_TYPE, true));  // "Drop" option
                    SetDropHeight(h * 3);
                    Select(0);
                }
            } else if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
                       client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
            {
                if (disabled) {
                    // For human players on other players non-host, have "AI" or "Observer" indicator (disabled)
                    Insert(new TypeRow(client_type));
                    Select(0);
                } else {
                    // For human players on host, have "Player", "Observer", and "Drop" options.  TODO: have "Ban" option.
                    // TODO: For human players on own non-host, have "Player" and "Observer" options
                    Insert(new TypeRow(Networking::CLIENT_TYPE_HUMAN_PLAYER));   // "Human" display / option
                    Insert(new TypeRow(Networking::CLIENT_TYPE_HUMAN_OBSERVER)); // "Observer" display / option
                    Insert(new TypeRow(Networking::INVALID_CLIENT_TYPE, true));  // "Drop" option
                    SetDropHeight(h * 4);
                    if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                        Select(0);
                    else
                        Select(1);
                }
            } else {
                if (disabled) {
                    // For empty row on non-host, should probably have no row... could also put "None" (disabled)
                    Insert(new TypeRow(Networking::INVALID_CLIENT_TYPE));
                    Select(0);
                } else {
                    // For empty row on host, have "None" and "Add AI" options on droplist
                    Insert(new TypeRow(Networking::INVALID_CLIENT_TYPE));        // "None" display
                    Insert(new TypeRow(Networking::CLIENT_TYPE_AI_PLAYER, true));// "Add AI" option
                    SetDropHeight(h * 3);
                    Select(0);
                }
            }

            GG::Connect(SelChangedSignal, &TypeSelector::SelectionChanged, this);
        }

        void SelectionChanged(GG::DropDownList::iterator it)
        {
            const GG::ListBox::Row* row = 0;
            if (it != this->end())
                row = *it;
            if (!row)
                return;

            const TypeRow* type_row = boost::polymorphic_downcast<const TypeRow*>(row);
            if (!type_row)
                return;

            TypeChangedSignal(type_row->type);
        }

        mutable boost::signal<void (Networking::ClientType)> TypeChangedSignal;
    };


    // Row for indicating / manipulating info about a player when creating a new game
    struct NewGamePlayerRow : PlayerRow {
        NewGamePlayerRow(const PlayerSetupData& player_data, int player_id, bool disabled) :
            PlayerRow(player_data, player_id)
        {
            // human / AI / observer indicator / selector
            TypeSelector* type_drop = new TypeSelector(GG::X(90), PLAYER_ROW_HEIGHT, player_data.m_client_type, disabled);
            push_back(type_drop);
            if (disabled)
                type_drop->Disable();
            else
                GG::Connect(type_drop->TypeChangedSignal,           &NewGamePlayerRow::PlayerTypeChanged,   this);

            // player name text
            push_back(player_data.m_player_name, ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());

            // empire name editable text
            CUIEdit* edit = new CUIEdit(GG::X0, GG::Y0, EMPIRE_NAME_WIDTH, m_player_data.m_empire_name,
                                        ClientUI::GetFont(), GG::CLR_ZERO, ClientUI::TextColor(), GG::CLR_ZERO);
            push_back(edit);
            if (disabled)
                edit->Disable();
            else
                GG::Connect(edit->EditedSignal,                     &NewGamePlayerRow::EmpireNameChanged,   this);

            // empire colour selector
            EmpireColorSelector* color_selector = new EmpireColorSelector(PLAYER_ROW_HEIGHT);
            color_selector->SelectColor(m_player_data.m_empire_color);
            push_back(color_selector);
            if (disabled)
                color_selector->Disable();
            else
                GG::Connect(color_selector->ColorChangedSignal,     &NewGamePlayerRow::ColorChanged,        this);

            // species selector
            SpeciesSelector* species_selector = new SpeciesSelector(EMPIRE_NAME_WIDTH, PLAYER_ROW_HEIGHT);
            push_back(species_selector);
            if (disabled)
                species_selector->Disable();
            else
                GG::Connect(species_selector->SpeciesChangedSignal, &NewGamePlayerRow::SpeciesChanged,      this);
        }

    private:
        void PlayerTypeChanged(Networking::ClientType type) {
            m_player_data.m_client_type = type;
            DataChangedSignal();
        }
        void EmpireNameChanged(const std::string& str) {
            m_player_data.m_empire_name = str;
            DataChangedSignal();
        }
        void ColorChanged(const GG::Clr& clr) {
            m_player_data.m_empire_color = clr;
            DataChangedSignal();
        }
        void SpeciesChanged(const std::string& str) {
            m_player_data.m_starting_species_name = str;
            DataChangedSignal();
        }
    };

    struct LoadGamePlayerRow : PlayerRow {
        LoadGamePlayerRow(const PlayerSetupData& player_data, int player_id, const std::map<int, SaveGameEmpireData>& save_game_empire_data, bool disabled) :
            PlayerRow(player_data, player_id),
            m_empire_list(0),
            m_save_game_empire_data(save_game_empire_data)
        {
            Resize(GG::Pt(EMPIRE_NAME_WIDTH, PLAYER_ROW_HEIGHT + ROW_HEIGHT_PAD));

            // player name text
            push_back(player_data.m_player_name, ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());

            // droplist to select empire
            m_empire_list = new CUIDropDownList(GG::X0, GG::Y0, EMPIRE_NAME_WIDTH, PLAYER_ROW_HEIGHT, 5 * PLAYER_ROW_HEIGHT);
            m_empire_list->SetStyle(GG::LIST_NOSORT);
            std::map<int, SaveGameEmpireData>::const_iterator save_game_empire_it = m_save_game_empire_data.end();
            for (std::map<int, SaveGameEmpireData>::const_iterator it = m_save_game_empire_data.begin(); it != m_save_game_empire_data.end(); ++it) {
                // insert row into droplist of empires for this player row
                m_empire_list->Insert(new CUISimpleDropDownListRow(it->second.m_empire_name));

                // attempt to choose a default empire to be selected in this
                // player row.  if this empire row matches this player data's
                // save gamge empire id, or if this empire row's player name
                // matches this player data's player name, select the row
                if ((it->first == m_player_data.m_save_game_empire_id) ||
                    (it->second.m_player_name == m_player_data.m_player_name))
                {
                    m_empire_list->Select(--m_empire_list->end());
                    m_player_data.m_empire_name =           it->second.m_empire_name;
                    m_player_data.m_empire_color =          it->second.m_color;
                    m_player_data.m_save_game_empire_id =   it->second.m_empire_id;
                    save_game_empire_it = it;
                }
            }
            push_back(m_empire_list);

            // empire colour selector (disabled, so acts as colour indicator)
            m_color_selector = new EmpireColorSelector(PLAYER_ROW_HEIGHT);
            m_color_selector->SelectColor(m_player_data.m_empire_color);
            push_back(m_color_selector);

            // original empire player name from saved game
            push_back(save_game_empire_it != m_save_game_empire_data.end() ? save_game_empire_it->second.m_player_name : "",
                      ClientUI::Font(), ClientUI::Pts(), ClientUI::TextColor());

            m_color_selector->Disable();

            if (disabled)
                m_empire_list->Disable();
            else
                Connect(m_empire_list->SelChangedSignal, &LoadGamePlayerRow::EmpireChanged, this);
        }

    private:
        void EmpireChanged(GG::DropDownList::iterator selected_it) {
            assert(selected_it != m_empire_list->end());
            std::map<int, SaveGameEmpireData>::const_iterator it = m_save_game_empire_data.begin();
            std::advance(it, m_empire_list->IteratorToIndex(selected_it));
            m_player_data.m_empire_name =           it->second.m_empire_name;
            m_player_data.m_empire_color =          it->second.m_color;
            m_player_data.m_save_game_empire_id =   it->second.m_empire_id;
            m_color_selector->SelectColor(m_player_data.m_empire_color);
            boost::polymorphic_downcast<GG::TextControl*>(operator[](3))->SetText(it->second.m_player_name);
            DataChangedSignal();
        }

        EmpireColorSelector*                     m_color_selector;
        CUIDropDownList*                         m_empire_list;
        const std::map<int, SaveGameEmpireData>& m_save_game_empire_data;
    };

    const GG::X     LOBBY_WND_WIDTH(800);
    const GG::Y     LOBBY_WND_HEIGHT(600);
    const int       CONTROL_MARGIN = 5; // gap to leave between controls in the window
    const GG::X     GALAXY_SETUP_PANEL_WIDTH(250);
    const GG::Y     SAVED_GAMES_LIST_ROW_HEIGHT(22);
    const GG::Y     SAVED_GAMES_LIST_DROP_HEIGHT(10 * SAVED_GAMES_LIST_ROW_HEIGHT);
    const GG::X     CHAT_WIDTH(250);
    GG::Pt          g_preview_ul;
    const GG::Pt    PREVIEW_SZ(GG::X(248), GG::Y(186));
    const int       PREVIEW_MARGIN = 3;
}

MultiplayerLobbyWnd::MultiplayerLobbyWnd(bool host,
                                         const CUIButton::ClickedSignalType::slot_type& start_game_callback,
                                         const CUIButton::ClickedSignalType::slot_type& cancel_callback) :
    CUIWnd(UserString("MPLOBBY_WINDOW_TITLE"),
           (GG::GUI::GetGUI()->AppWidth() - LOBBY_WND_WIDTH) / 2,
           (GG::GUI::GetGUI()->AppHeight() - LOBBY_WND_HEIGHT) / 2,
           LOBBY_WND_WIDTH, LOBBY_WND_HEIGHT,
           GG::ONTOP | GG::INTERACTIVE),
    m_host(host),
    m_chat_box(0),
    m_chat_input_edit(0),
    m_new_load_game_buttons(0),
    m_galaxy_setup_panel(0),
    m_saved_games_list(0),
    m_preview_image(0),
    m_players_lb_player_name_column_label(0),
    m_players_lb_empire_name_column_label(0),
    m_players_lb_empire_colour_column_label(0),
    m_players_lb_species_or_original_player_label(0),
    m_players_lb(0),
    m_start_game_bn(0),
    m_cancel_bn(0),
    m_start_conditions_text(0)
{
    Sound::TempUISoundDisabler sound_disabler;

    GG::X x(CONTROL_MARGIN);
    m_chat_input_edit = new CUIEdit(x, ClientHeight() - (ClientUI::Pts() + 10) - 2 * CONTROL_MARGIN, CHAT_WIDTH - x, "");
    m_chat_box = new CUIMultiEdit(x, GG::Y(CONTROL_MARGIN), CHAT_WIDTH - x, m_chat_input_edit->UpperLeft().y - 2 * CONTROL_MARGIN, "",
                                  GG::MULTI_LINEWRAP | GG::MULTI_READ_ONLY | GG::MULTI_TERMINAL_STYLE);
    m_chat_box->SetMaxLinesOfHistory(250);

    const GG::Y RADIO_BN_HT(ClientUI::Pts() + 4);

    m_galaxy_setup_panel = new GalaxySetupPanel(CHAT_WIDTH + 2*CONTROL_MARGIN, RADIO_BN_HT, GALAXY_SETUP_PANEL_WIDTH);

    m_new_load_game_buttons =
        new GG::RadioButtonGroup(CHAT_WIDTH + CONTROL_MARGIN, GG::Y(CONTROL_MARGIN),
                                 GG::X(Value(m_galaxy_setup_panel->LowerRight().y + 100)), m_galaxy_setup_panel->LowerRight().y + RADIO_BN_HT - CONTROL_MARGIN,
                                 GG::VERTICAL);
    m_new_load_game_buttons->AddButton(
        new CUIStateButton(GG::X0, GG::Y0, GG::X(100), RADIO_BN_HT, UserString("NEW_GAME_BN"), GG::FORMAT_LEFT, GG::SBSTYLE_3D_RADIO));
    m_new_load_game_buttons->AddButton(
        new CUIStateButton(GG::X0, GG::Y0, GG::X(100), RADIO_BN_HT, UserString("LOAD_GAME_BN"), GG::FORMAT_LEFT, GG::SBSTYLE_3D_RADIO));

    m_saved_games_list = new CUIDropDownList(CHAT_WIDTH + 2 * CONTROL_MARGIN, m_new_load_game_buttons->LowerRight().y + CONTROL_MARGIN,
                                             GALAXY_SETUP_PANEL_WIDTH, SAVED_GAMES_LIST_ROW_HEIGHT, SAVED_GAMES_LIST_DROP_HEIGHT);
    m_saved_games_list->SetStyle(GG::LIST_NOSORT);

    g_preview_ul = GG::Pt(ClientWidth() - PREVIEW_SZ.x - CONTROL_MARGIN - PREVIEW_MARGIN, GG::Y(CONTROL_MARGIN + PREVIEW_MARGIN));
    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::GRAPHIC_FITGRAPHIC);

    x = CHAT_WIDTH + CONTROL_MARGIN;
    GG::Y y = std::max(m_saved_games_list->LowerRight().y, m_preview_image->LowerRight().y) + CONTROL_MARGIN;
    const GG::Y TEXT_HEIGHT = GG::Y(ClientUI::Pts() * 3/2);


    m_players_lb_player_type_label = new GG::TextControl(GG::X0, GG::Y0, EMPIRE_NAME_WIDTH, TEXT_HEIGHT,
                                                         UserString("MULTIPLAYER_PLAYER_LIST_TYPES"),
                                                         ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_player_name_column_label = new GG::TextControl(GG::X0, GG::Y0, EMPIRE_NAME_WIDTH, TEXT_HEIGHT,
                                                                UserString("MULTIPLAYER_PLAYER_LIST_NAMES"),
                                                                ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_empire_name_column_label = new GG::TextControl(GG::X0, GG::Y0, EMPIRE_NAME_WIDTH, TEXT_HEIGHT,
                                                                UserString("MULTIPLAYER_PLAYER_LIST_EMPIRES"),
                                                                ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_empire_colour_column_label = new GG::TextControl(GG::X0, GG::Y0, EMPIRE_NAME_WIDTH, TEXT_HEIGHT,
                                                                  UserString("MULTIPLAYER_PLAYER_LIST_COLOURS"),
                                                                  ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_species_or_original_player_label = new GG::TextControl(GG::X0, GG::Y0, EMPIRE_NAME_WIDTH, TEXT_HEIGHT,
                                                                        UserString("MULTIPLAYER_PLAYER_LIST_ORIGINAL_NAMES"),
                                                                        ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    GG::Layout* layout = new GG::Layout(x + CONTROL_MARGIN, y, ClientWidth() - 2*CONTROL_MARGIN - x, TEXT_HEIGHT, 1, 4);
    layout->SetMinimumRowHeight(0, TEXT_HEIGHT);
    layout->Add(m_players_lb_player_type_label,                 0, 0);
    layout->Add(m_players_lb_player_name_column_label,          0, 1);
    layout->Add(m_players_lb_empire_name_column_label,          0, 2);
    layout->Add(m_players_lb_empire_colour_column_label,        0, 3);
    layout->Add(m_players_lb_species_or_original_player_label,  0, 4);
    AttachChild(layout);

    y += TEXT_HEIGHT + CONTROL_MARGIN;
    x = CHAT_WIDTH + CONTROL_MARGIN;

    m_players_lb = new CUIListBox(x, y, ClientWidth() - CONTROL_MARGIN - x, m_chat_input_edit->UpperLeft().y - CONTROL_MARGIN - y);
    m_players_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);

    if (m_host)
        m_start_game_bn = new CUIButton(GG::X0, GG::Y0, GG::X(125), UserString("START_GAME_BN"));
    m_cancel_bn = new CUIButton(GG::X0, GG::Y0, GG::X(125), UserString("CANCEL"));
    m_cancel_bn->MoveTo(GG::Pt(ClientWidth() - m_cancel_bn->Width() - CONTROL_MARGIN, ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN));
    if (m_host)
        m_start_game_bn->MoveTo(GG::Pt(m_cancel_bn->UpperLeft().x - CONTROL_MARGIN - m_start_game_bn->Width(),
                                       ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN));

    m_start_conditions_text = new GG::TextControl(x, ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN,
                                                  m_cancel_bn->UpperLeft().x - x, TEXT_HEIGHT,
                                                  UserString("MULTIPLAYER_GAME_START_CONDITIONS"),
                                                  ClientUI::GetFont(),
                                                  ClientUI::TextColor(), GG::FORMAT_LEFT);

    AttachChild(m_chat_box);
    AttachChild(m_chat_input_edit);
    AttachChild(m_new_load_game_buttons);
    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_saved_games_list);
    AttachChild(m_preview_image);
    AttachChild(m_players_lb);
    AttachChild(m_start_game_bn);
    AttachChild(m_cancel_bn);
    AttachChild(m_start_conditions_text);

    // default settings (new game)
    m_new_load_game_buttons->SetCheck(0);
    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());
    m_saved_games_list->Disable();

    if (!m_host) {
        for (std::size_t i = 0; i < m_new_load_game_buttons->NumButtons(); ++i)
            m_new_load_game_buttons->DisableButton(i);
        m_galaxy_setup_panel->Disable();
        m_saved_games_list->Disable();
    }

    if (m_host) {
        GG::Connect(m_new_load_game_buttons->ButtonChangedSignal,   &MultiplayerLobbyWnd::NewLoadClicked,           this);
        GG::Connect(m_galaxy_setup_panel->SettingsChangedSignal,    &MultiplayerLobbyWnd::GalaxySetupPanelChanged,  this);
        GG::Connect(m_saved_games_list->SelChangedSignal,           &MultiplayerLobbyWnd::SaveGameChanged,          this);
        GG::Connect(m_start_game_bn->ClickedSignal,                 start_game_callback);
    }
    GG::Connect(m_galaxy_setup_panel->ImageChangedSignal,           &MultiplayerLobbyWnd::PreviewImageChanged,      this);
    GG::Connect(m_cancel_bn->ClickedSignal,                         cancel_callback);
}

bool MultiplayerLobbyWnd::LoadGameSelected() const
{ return m_new_load_game_buttons->CheckedButton() == 1; }

void MultiplayerLobbyWnd::Render()
{
    CUIWnd::Render();
    GG::Pt image_ul = g_preview_ul + ClientUpperLeft(), image_lr = image_ul + PREVIEW_SZ;
    GG::FlatRectangle(GG::Pt(image_ul.x - PREVIEW_MARGIN, image_ul.y - PREVIEW_MARGIN),
                      GG::Pt(image_lr.x + PREVIEW_MARGIN, image_lr.y + PREVIEW_MARGIN), 
                      GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}

void MultiplayerLobbyWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    if ((key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) && GG::GUI::GetGUI()->FocusWnd() == m_chat_input_edit) {
        int receiver = -1; // all players by default
        std::string text = m_chat_input_edit->Text();
        HumanClientApp::GetApp()->Networking().SendMessage(LobbyChatMessage(HumanClientApp::GetApp()->PlayerID(), receiver, text));
        m_chat_input_edit->SetText("");
        *m_chat_box += m_lobby_data.m_players[HumanClientApp::GetApp()->PlayerID()].m_player_name + ": " + text + "\n";
    } else if (m_start_game_bn && !m_start_game_bn->Disabled() && (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER)) {
        m_start_game_bn->ClickedSignal();
    } else if (key == GG::GGK_ESCAPE) {
       m_cancel_bn->ClickedSignal();
    }
}

void MultiplayerLobbyWnd::ChatMessage(int player_id, const std::string& msg)
{
    std::map<int, PlayerSetupData>::iterator it = m_lobby_data.m_players.find(player_id);
    *m_chat_box += (it != m_lobby_data.m_players.end() ? (it->second.m_player_name + ": ") : "[unknown]: ") + msg + '\n';
}

void MultiplayerLobbyWnd::LobbyUpdate(const MultiplayerLobbyData& lobby_data)
{
    m_new_load_game_buttons->SetCheck(!lobby_data.m_new_game);
    m_galaxy_setup_panel->SetFromSetupData(lobby_data);

    m_saved_games_list->Clear();
    for (unsigned int i = 0; i < lobby_data.m_save_games.size(); ++i) {
        m_saved_games_list->Insert(new CUISimpleDropDownListRow(lobby_data.m_save_games[i]));
        if (static_cast<int>(i) == lobby_data.m_save_file_index)
            m_saved_games_list->Select(lobby_data.m_save_file_index);
    }

    m_lobby_data = lobby_data;

    bool send_update_back = PopulatePlayerList();

    if (m_host && send_update_back)
        SendUpdate();
}

void MultiplayerLobbyWnd::LobbyExit(int player_id)
{
    std::string player_name = m_lobby_data.m_players[player_id].m_player_name;
    m_lobby_data.m_players.erase(player_id);
    for (GG::ListBox::iterator it = m_players_lb->begin(); it != m_players_lb->end(); ++it) {
        if (player_name == boost::polymorphic_downcast<GG::TextControl*>((**it)[0])->Text()) {
            delete m_players_lb->Erase(it);
            break;
        }
    }
    if (m_host)
        m_start_game_bn->Disable(!CanStart());
}

void MultiplayerLobbyWnd::NewLoadClicked(std::size_t idx)
{
    switch (idx) {
    case std::size_t(0):
        m_lobby_data.m_new_game = true;
        m_galaxy_setup_panel->Disable(false);
        m_saved_games_list->Disable();
        break;
    case std::size_t(1):
        m_lobby_data.m_new_game = false;
        m_galaxy_setup_panel->Disable();
        m_saved_games_list->Disable(false);
        break;
    default:
        break;
    }
    PopulatePlayerList();
    SendUpdate();
}

void MultiplayerLobbyWnd::GalaxySetupPanelChanged()
{
    m_galaxy_setup_panel->GetSetupData(m_lobby_data);
    SendUpdate();
}

void MultiplayerLobbyWnd::SaveGameChanged(GG::DropDownList::iterator it)
{
    m_lobby_data.m_save_file_index = m_saved_games_list->IteratorToIndex(it);
    m_lobby_data.m_save_game_empire_data.clear();
    PopulatePlayerList();
    SendUpdate();
}

void MultiplayerLobbyWnd::PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image)
{
    if (m_preview_image) {
        DeleteChild(m_preview_image);
        m_preview_image = 0;
    }
    m_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, new_image, GG::GRAPHIC_FITGRAPHIC);
    AttachChild(m_preview_image);
}

void MultiplayerLobbyWnd::PlayerDataChanged()
{
    m_lobby_data.m_players.clear();
    for (GG::ListBox::iterator it = m_players_lb->begin(); it != m_players_lb->end(); ++it) {
        const PlayerRow* row = boost::polymorphic_downcast<const PlayerRow*>(*it);
        m_lobby_data.m_players[row->m_player_id] = row->m_player_data;
    }
    if (m_host)
        m_start_game_bn->Disable(!CanStart());
    SendUpdate();
}

bool MultiplayerLobbyWnd::PopulatePlayerList()
{
    bool send_update_back_retval = false;

    m_players_lb->Clear();

    // repopulate list with rows built from current lobby data
    for (std::map<int, PlayerSetupData>::iterator player_setup_it = m_lobby_data.m_players.begin();
         player_setup_it != m_lobby_data.m_players.end(); ++player_setup_it)
    {
        int data_player_id = player_setup_it->first;
        PlayerSetupData& psd = player_setup_it->second;

        if (m_lobby_data.m_new_game) {
            bool immutable_row = !m_host && (data_player_id != HumanClientApp::GetApp()->PlayerID());   // host can modify any player's row.  non-hosts can only modify their own row
            NewGamePlayerRow* row = new NewGamePlayerRow(psd, data_player_id, immutable_row);
            m_players_lb->Insert(row);
            Connect(row->DataChangedSignal, &MultiplayerLobbyWnd::PlayerDataChanged, this);

        } else {
            bool immutable_row = (!m_host && (data_player_id != HumanClientApp::GetApp()->PlayerID())) || m_lobby_data.m_save_game_empire_data.empty();
            LoadGamePlayerRow* row = new LoadGamePlayerRow(psd, data_player_id, m_lobby_data.m_save_game_empire_data, immutable_row);
            m_players_lb->Insert(row);
            Connect(row->DataChangedSignal, &MultiplayerLobbyWnd::PlayerDataChanged, this);

            // if the player setup data in the row is different from the player
            // setup data in this lobby wnd's lobby data (which may have
            // happened because the LoadGamePlayerRow constructor selects an
            // empire row for the droplist in the player row) then the change
            // needs to be sent to other players
            if (row->m_player_data.m_save_game_empire_id != psd.m_save_game_empire_id) {
                psd = row->m_player_data;
                send_update_back_retval = true;
            }
        }
    }

    if (m_lobby_data.m_new_game) {
        m_players_lb_species_or_original_player_label->SetText(UserString("MULTIPLAYER_PLAYER_LIST_STARTING_SPECIES"));
    } else {
        m_players_lb_species_or_original_player_label->SetText(UserString("MULTIPLAYER_PLAYER_LIST_ORIGINAL_NAMES"));
    }

    if (m_host)
        m_start_game_bn->Disable(!CanStart());

    return send_update_back_retval;
}

void MultiplayerLobbyWnd::SendUpdate()
{
    int player_id = HumanClientApp::GetApp()->PlayerID();
    if (player_id != Networking::INVALID_PLAYER_ID)
        HumanClientApp::GetApp()->Networking().SendMessage(LobbyUpdateMessage(player_id, m_lobby_data));
}

bool MultiplayerLobbyWnd::PlayerDataAcceptable() const
{
    std::set<std::string> empire_names;
    std::set<unsigned int> empire_colors;
    int num_players_excluding_observers(0);

    for (GG::ListBox::iterator it = m_players_lb->begin(); it != m_players_lb->end(); ++it) {
        const PlayerRow& row = dynamic_cast<const PlayerRow&>(**it);
        if (row.m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            row.m_player_data.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
        {
            num_players_excluding_observers++;

            // all non-observers must have unique non-blank names and colours

            if (row.m_player_data.m_empire_name.empty())
                return false;

            empire_names.insert(row.m_player_data.m_empire_name);

            unsigned int color_as_uint =
                row.m_player_data.m_empire_color.r << 24 |
                row.m_player_data.m_empire_color.g << 16 |
                row.m_player_data.m_empire_color.b << 8 |
                row.m_player_data.m_empire_color.a;
            empire_colors.insert(color_as_uint);
        } else if (row.m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER) {
            // do nothing special for this player
        } else {
            // unrecognized client type!
            return false;
        }
    }
    // any duplicate names or colours will means that the number of active
    // players and number of colours / names won't match
    return empire_names.size() == num_players_excluding_observers &&
           empire_colors.size() == num_players_excluding_observers;
}

bool MultiplayerLobbyWnd::CanStart() const
{ return PlayerDataAcceptable(); }
