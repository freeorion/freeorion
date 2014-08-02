#include "MultiplayerLobbyWnd.h"

#include "CUIControls.h"
#include "Hotkeys.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Message.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
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
    const GG::X BROWSE_BTN_WIDTH(50);

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
        boost::signals2::signal<void ()>  DataChangedSignal;
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
                push_back(UserString("NO_PLAYER"), ClientUI::GetFont(), ClientUI::TextColor());
            }
            TypeRow(Networking::ClientType type_, bool show_add_drop = false) :
                GG::DropDownList::Row(GG::X1, GG::Y1, "PlayerTypeSelectorRow"),
                type(type_)
            {
                switch (type) {
                case Networking::CLIENT_TYPE_AI_PLAYER:
                    if (show_add_drop)
                        push_back(UserString("ADD_AI_PLAYER"), ClientUI::GetFont(), ClientUI::TextColor());
                    else
                        push_back(UserString("AI_PLAYER"), ClientUI::GetFont(), ClientUI::TextColor());
                    break;
                case Networking::CLIENT_TYPE_HUMAN_OBSERVER:
                    push_back(UserString("OBSERVER"), ClientUI::GetFont(), ClientUI::TextColor());
                    break;
                case Networking::CLIENT_TYPE_HUMAN_PLAYER:
                    push_back(UserString("HUMAN_PLAYER"), ClientUI::GetFont(), ClientUI::TextColor());
                    break;
                case Networking::CLIENT_TYPE_HUMAN_MODERATOR:
                    push_back(UserString("MODERATOR"), ClientUI::GetFont(), ClientUI::TextColor());
                    break;
                default:
                    if (show_add_drop)
                        push_back(UserString("DROP_PLAYER"), ClientUI::GetFont(), ClientUI::TextColor());
                    else
                        push_back(UserString("NO_PLAYER"), ClientUI::GetFont(), ClientUI::TextColor());
                }
            }

            Networking::ClientType type;
        };

    public:
        TypeSelector() :
            CUIDropDownList(GG::Y1)
        {}

        TypeSelector(GG::X w, GG::Y h, Networking::ClientType client_type, bool disabled) :
            CUIDropDownList(h)
        {
            Resize(GG::Pt(w, std::max(GG::Y1, h - 8)));
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
                       client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                       client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
            {
                if (disabled) {
                    // For human players on other players non-host, have "AI" or "Observer" indicator (disabled)
                    Insert(new TypeRow(client_type));
                    Select(0);
                } else {
                    // For human players on host, have "Player", "Observer", and "Drop" options.  TODO: have "Ban" option.
                    Insert(new TypeRow(Networking::CLIENT_TYPE_HUMAN_PLAYER));   // "Human" display / option
                    Insert(new TypeRow(Networking::CLIENT_TYPE_HUMAN_OBSERVER)); // "Observer" display / option
                    Insert(new TypeRow(Networking::CLIENT_TYPE_HUMAN_MODERATOR));// "Moderator" display / option
                    Insert(new TypeRow(Networking::INVALID_CLIENT_TYPE, true));  // "Drop" option
                    SetDropHeight(h * 4);
                    if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                        Select(0);
                    else if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
                        Select(1);
                    else if (client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
                        Select(2);
                    else
                        Select(3);
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

        mutable boost::signals2::signal<void (Networking::ClientType)> TypeChangedSignal;
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
            push_back(player_data.m_player_name, ClientUI::GetFont(), ClientUI::TextColor());

            if (player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR) {
                // observers don't need to pick an empire or species
                push_back("", ClientUI::GetFont());
                push_back("", ClientUI::GetFont());
                push_back("", ClientUI::GetFont());
                return;
            }

            // empire name editable text
            CUIEdit* edit = new CUIEdit(m_player_data.m_empire_name);
            edit->SetColor(GG::CLR_ZERO);
            edit->SetInteriorColor(GG::CLR_ZERO);
            edit->Resize(GG::Pt(EMPIRE_NAME_WIDTH, edit->MinUsableSize().y));
            push_back(edit);
            if (disabled)
                edit->Disable();
            else
                GG::Connect(edit->FocusUpdateSignal,                &NewGamePlayerRow::EmpireNameChanged,   this);

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
            species_selector->SelectSpecies(m_player_data.m_starting_species_name);
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

    // Row for player info when loading a game
    struct LoadGamePlayerRow : PlayerRow {
        LoadGamePlayerRow(const PlayerSetupData& player_data, int player_id, const std::map<int, SaveGameEmpireData>& save_game_empire_data, bool disabled) :
            PlayerRow(player_data, player_id),
            m_empire_list(0),
            m_save_game_empire_data(save_game_empire_data)
        {
            // human / AI / observer indicator / selector
            TypeSelector* type_drop = new TypeSelector(GG::X(90), PLAYER_ROW_HEIGHT, player_data.m_client_type, disabled);
            push_back(type_drop);
            if (disabled)
                type_drop->Disable();
            else
                GG::Connect(type_drop->TypeChangedSignal, &LoadGamePlayerRow::PlayerTypeChanged, this);

            // player name text
            push_back(player_data.m_player_name, ClientUI::GetFont(), ClientUI::TextColor());

            // droplist to select empire
            m_empire_list = new CUIDropDownList(5 * PLAYER_ROW_HEIGHT);
            m_empire_list->Resize(GG::Pt(EMPIRE_NAME_WIDTH, PLAYER_ROW_HEIGHT));
            m_empire_list->SetStyle(GG::LIST_NOSORT);
            std::map<int, SaveGameEmpireData>::const_iterator save_game_empire_it = m_save_game_empire_data.end();
            for (std::map<int, SaveGameEmpireData>::const_iterator it = m_save_game_empire_data.begin();
                 it != m_save_game_empire_data.end(); ++it)
            {
                // insert row into droplist of empires for this player row
                m_empire_list->Insert(new CUISimpleDropDownListRow(it->second.m_empire_name));

                // attempt to choose a default empire to be selected in this
                // player row.  if this empire row matches this player data's
                // save gamge empire id, or if this empire row's player name
                // matches this player data's player name, select the row
                if ((it->first == m_player_data.m_save_game_empire_id) ||
                        (m_player_data.m_save_game_empire_id == ALL_EMPIRES &&
                         it->second.m_player_name == m_player_data.m_player_name)
                   )
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
                      ClientUI::GetFont(), ClientUI::TextColor());

            m_color_selector->Disable();

            if (disabled)
                m_empire_list->Disable();
            else
                Connect(m_empire_list->SelChangedSignal, &LoadGamePlayerRow::EmpireChanged, this);
        }

    private:
        void PlayerTypeChanged(Networking::ClientType type) {
            m_player_data.m_client_type = type;
            DataChangedSignal();
        }
        void EmpireChanged(GG::DropDownList::iterator selected_it) {
            assert(selected_it != m_empire_list->end());
            std::map<int, SaveGameEmpireData>::const_iterator it = m_save_game_empire_data.begin();
            std::advance(it, m_empire_list->IteratorToIndex(selected_it));
            m_player_data.m_empire_name =           it->second.m_empire_name;
            m_player_data.m_empire_color =          it->second.m_color;
            m_player_data.m_save_game_empire_id =   it->second.m_empire_id;
            m_color_selector->SelectColor(m_player_data.m_empire_color);

            // set previous player name indication
            boost::polymorphic_downcast<GG::TextControl*>(operator[](4))->SetText(it->second.m_player_name);

            DataChangedSignal();
        }

        EmpireColorSelector*                     m_color_selector;
        CUIDropDownList*                         m_empire_list;
        const std::map<int, SaveGameEmpireData>& m_save_game_empire_data;
    };

    // Row for indicating that an AI client should be added to the game
    struct EmptyPlayerRow : PlayerRow {
        EmptyPlayerRow() :
            PlayerRow()
        {
            TypeSelector* type_drop = new TypeSelector(GG::X(90), PLAYER_ROW_HEIGHT, Networking::INVALID_CLIENT_TYPE, false);
            push_back(type_drop);
            GG::Connect(type_drop->TypeChangedSignal,       &EmptyPlayerRow::PlayerTypeChanged,   this);
            // extra entries to make layout consistent
            push_back("", ClientUI::GetFont());
            push_back("", ClientUI::GetFont());
            push_back("", ClientUI::GetFont());
            push_back("", ClientUI::GetFont());
        }
    private:
        void PlayerTypeChanged(Networking::ClientType type) {
            m_player_data.m_client_type = type;
            DataChangedSignal();
        }
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

MultiPlayerLobbyWnd::MultiPlayerLobbyWnd() :
    CUIWnd(UserString("MPLOBBY_WINDOW_TITLE"),
           (GG::GUI::GetGUI()->AppWidth() - LOBBY_WND_WIDTH) / 2,
           (GG::GUI::GetGUI()->AppHeight() - LOBBY_WND_HEIGHT) / 2,
           LOBBY_WND_WIDTH, LOBBY_WND_HEIGHT,
           GG::ONTOP | GG::INTERACTIVE),
    m_chat_box(0),
    m_chat_input_edit(0),
    m_new_load_game_buttons(0),
    m_galaxy_setup_panel(0),
    m_browse_saves_btn(0),
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

    m_chat_input_edit = new CUIEdit("");
    m_chat_box = new CUIMultiEdit(GG::X0, GG::Y0, GG::X1, GG::Y1, "", GG::MULTI_LINEWRAP | GG::MULTI_READ_ONLY | GG::MULTI_TERMINAL_STYLE);
    m_chat_box->SetMaxLinesOfHistory(250);

    m_galaxy_setup_panel = new GalaxySetupPanel(GG::X0, GG::Y0, GG::X1);

    m_new_load_game_buttons = new GG::RadioButtonGroup(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::VERTICAL);
    m_new_load_game_buttons->AddButton(
        new CUIStateButton(UserString("NEW_GAME_BN"), GG::FORMAT_LEFT, GG::SBSTYLE_3D_RADIO));
    m_new_load_game_buttons->AddButton(
        new CUIStateButton(UserString("LOAD_GAME_BN"), GG::FORMAT_LEFT, GG::SBSTYLE_3D_RADIO));

    m_browse_saves_btn = new CUIButton("...");
    m_save_file_text = new GG::TextControl(GG::X0, GG::Y0, "", ClientUI::GetFont(), ClientUI::TextColor());

    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_preview_image = new GG::StaticGraphic(GG::X0, GG::Y0, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::GRAPHIC_FITGRAPHIC);

    m_players_lb_player_type_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                         UserString("MULTIPLAYER_PLAYER_LIST_TYPES"),
                                                         ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_player_name_column_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                                UserString("MULTIPLAYER_PLAYER_LIST_NAMES"),
                                                                ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_empire_name_column_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                                UserString("MULTIPLAYER_PLAYER_LIST_EMPIRES"),
                                                                ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_empire_colour_column_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                                  UserString("MULTIPLAYER_PLAYER_LIST_COLOURS"),
                                                                  ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb_species_or_original_player_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                                        UserString("MULTIPLAYER_PLAYER_LIST_ORIGINAL_NAMES"),
                                                                        ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT);

    m_players_lb = new CUIListBox();
    m_players_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);

    m_start_game_bn = new CUIButton(UserString("START_GAME_BN"));
    m_cancel_bn = new CUIButton(UserString("CANCEL"));

    m_start_conditions_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                  UserString("MULTIPLAYER_GAME_START_CONDITIONS"),
                                                  ClientUI::GetFont(),
                                                  ClientUI::TextColor(), GG::FORMAT_LEFT);

    AttachChild(m_chat_box);
    AttachChild(m_chat_input_edit);
    AttachChild(m_new_load_game_buttons);
    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_save_file_text);
    AttachChild(m_browse_saves_btn);
    AttachChild(m_preview_image);
    AttachChild(m_players_lb);
    AttachChild(m_players_lb_player_type_label);
    AttachChild(m_players_lb_player_name_column_label);
    AttachChild(m_players_lb_empire_name_column_label);
    AttachChild(m_players_lb_empire_colour_column_label);
    AttachChild(m_players_lb_species_or_original_player_label);
    AttachChild(m_start_game_bn);
    AttachChild(m_cancel_bn);
    AttachChild(m_start_conditions_text);

    DoLayout();

    // default settings (new game)
    m_new_load_game_buttons->SetCheck(0);
    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());
    m_save_file_text->Disable();
    m_browse_saves_btn->Disable();

    GG::Connect(m_chat_input_edit->GainingFocusSignal,          &MultiPlayerLobbyWnd::EnableTypingUnsafeAccels,  this);
    GG::Connect(m_chat_input_edit->LosingFocusSignal,           &MultiPlayerLobbyWnd::DisableTypingUnsafeAccels, this);
    GG::Connect(m_new_load_game_buttons->ButtonChangedSignal,   &MultiPlayerLobbyWnd::NewLoadClicked,           this);
    GG::Connect(m_galaxy_setup_panel->SettingsChangedSignal,    &MultiPlayerLobbyWnd::GalaxySetupPanelChanged,  this);
    GG::Connect(m_browse_saves_btn->LeftClickedSignal,          &MultiPlayerLobbyWnd::SaveGameBrowse,          this);
    GG::Connect(m_start_game_bn->LeftClickedSignal,             &MultiPlayerLobbyWnd::StartGameClicked,         this);
    GG::Connect(m_galaxy_setup_panel->ImageChangedSignal,       &MultiPlayerLobbyWnd::PreviewImageChanged,      this);
    GG::Connect(m_cancel_bn->LeftClickedSignal,                 &MultiPlayerLobbyWnd::CancelClicked,            this);

    Refresh();
}

void MultiPlayerLobbyWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

bool MultiPlayerLobbyWnd::LoadGameSelected() const
{ return m_new_load_game_buttons->CheckedButton() == 1; }

void MultiPlayerLobbyWnd::Render() {
    CUIWnd::Render();
    GG::Pt image_ul = g_preview_ul + ClientUpperLeft(), image_lr = image_ul + PREVIEW_SZ;
    GG::FlatRectangle(GG::Pt(image_ul.x - PREVIEW_MARGIN, image_ul.y - PREVIEW_MARGIN),
                      GG::Pt(image_lr.x + PREVIEW_MARGIN, image_lr.y + PREVIEW_MARGIN), 
                      GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}

void MultiPlayerLobbyWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if ((key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) &&
         GG::GUI::GetGUI()->FocusWnd() == m_chat_input_edit)
    {
        int receiver = Networking::INVALID_PLAYER_ID;   // all players by default
        std::string text = m_chat_input_edit->Text();   // copy, so subsequent clearing doesn't affect typed message that is used later...
        HumanClientApp::GetApp()->Networking().SendMessage(LobbyChatMessage(HumanClientApp::GetApp()->PlayerID(), receiver, text));
        m_chat_input_edit->SetText("");

        // look up this client's player name by ID
        std::string player_name;
        for (std::list<std::pair<int, PlayerSetupData> >::const_iterator it = m_lobby_data.m_players.begin();
             it != m_lobby_data.m_players.end(); ++it)
        {
            if (it->first == HumanClientApp::GetApp()->PlayerID() && it->first != Networking::INVALID_PLAYER_ID) {
                player_name = it->second.m_player_name;
                break;
            }
        }

        // put message just sent in chat box (local echo)
        *m_chat_box += player_name + ": " + text + "\n";

    } else if (m_start_game_bn && !m_start_game_bn->Disabled() &&
               (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER))
    {
        m_start_game_bn->LeftClickedSignal();
    } else if (key == GG::GGK_ESCAPE) {
        m_cancel_bn->LeftClickedSignal();
    }
}

void MultiPlayerLobbyWnd::ChatMessage(int player_id, const std::string& msg) {
    // look up player name by ID
    std::string player_name;
    for (std::list<std::pair<int, PlayerSetupData> >::const_iterator it = m_lobby_data.m_players.begin();
         it != m_lobby_data.m_players.end(); ++it)
    {
        if (it->first == HumanClientApp::GetApp()->PlayerID() && it->first != Networking::INVALID_PLAYER_ID) {
            player_name = it->second.m_player_name;
            break;
        }
    }

    // show message with player name in chat box
    *m_chat_box += player_name + ": " + msg + '\n';
}

namespace {
    void LogPlayerSetupData(const std::list<std::pair<int, PlayerSetupData> >& psd) {
        Logger().debugStream() << "PlayerSetupData:";
        for (std::list<std::pair<int, PlayerSetupData> >::const_iterator it = psd.begin(); it != psd.end(); ++it)
            Logger().debugStream() << boost::lexical_cast<std::string>(it->first) << " : "
                                   << it->second.m_player_name << ", "
                                   << it->second.m_client_type << ", "
                                   << it->second.m_starting_species_name;
    }
}

void MultiPlayerLobbyWnd::LobbyUpdate(const MultiplayerLobbyData& lobby_data) {
    m_new_load_game_buttons->SetCheck(!lobby_data.m_new_game);
    m_galaxy_setup_panel->SetFromSetupData(lobby_data);

    m_save_file_text->SetText(lobby_data.m_save_game);

    m_lobby_data = lobby_data;

    bool send_update_back = PopulatePlayerList();

    if (send_update_back && ThisClientIsHost())
        SendUpdate();

    LogPlayerSetupData(m_lobby_data.m_players);
}

void MultiPlayerLobbyWnd::Refresh() {
    if (ThisClientIsHost()) {
        for (std::size_t i = 0; i < m_new_load_game_buttons->NumButtons(); ++i)
            m_new_load_game_buttons->DisableButton(i, false);
        m_galaxy_setup_panel->Disable(false);
        m_save_file_text->Disable(false);
        m_browse_saves_btn->Disable(false);
    } else {
        for (std::size_t i = 0; i < m_new_load_game_buttons->NumButtons(); ++i)
            m_new_load_game_buttons->DisableButton(i);
        m_galaxy_setup_panel->Disable();
        m_save_file_text->Disable();
        m_browse_saves_btn->Disable();
    }

    m_start_game_bn->Disable(!ThisClientIsHost() || !CanStart());
}

void MultiPlayerLobbyWnd::DoLayout() {
    GG::X x(CONTROL_MARGIN);

    GG::Pt chat_input_ul(x, ClientHeight() - (ClientUI::Pts() + 10) - 2 * CONTROL_MARGIN);
    GG::Pt chat_input_lr = chat_input_ul + GG::Pt(CHAT_WIDTH - x, m_chat_input_edit->MinUsableSize().y);
    m_chat_input_edit->SizeMove(chat_input_ul, chat_input_lr);

    GG::Pt chat_box_ul(x, GG::Y(CONTROL_MARGIN));
    GG::Pt chat_box_lr(CHAT_WIDTH, m_chat_input_edit->RelativeUpperLeft().y - CONTROL_MARGIN);
    m_chat_box->SizeMove(chat_box_ul, chat_box_lr);

    const GG::Y RADIO_BN_HT(ClientUI::Pts() + 4);

    GG::Pt galaxy_setup_panel_ul(CHAT_WIDTH + 2*CONTROL_MARGIN, RADIO_BN_HT);
    GG::Pt galaxy_setup_panel_lr = galaxy_setup_panel_ul + GG::Pt(GALAXY_SETUP_PANEL_WIDTH, GG::Y(340));
    m_galaxy_setup_panel->SizeMove(galaxy_setup_panel_ul, galaxy_setup_panel_lr);

    GG::Pt game_buttons_ul(CHAT_WIDTH + CONTROL_MARGIN, GG::Y(CONTROL_MARGIN));
    GG::Pt game_buttons_lr = game_buttons_ul + m_galaxy_setup_panel->RelativeLowerRight();
    m_new_load_game_buttons->SizeMove(game_buttons_ul, game_buttons_lr);

    GG::Pt browse_saves_btn_ul(CHAT_WIDTH + 2 * CONTROL_MARGIN, m_new_load_game_buttons->RelativeLowerRight().y + CONTROL_MARGIN);
    GG::Pt browse_saves_btn_lr = browse_saves_btn_ul + GG::Pt(BROWSE_BTN_WIDTH, m_browse_saves_btn->MinUsableSize().y);
    m_browse_saves_btn->SizeMove(browse_saves_btn_ul, browse_saves_btn_lr);

    GG::Pt save_file_text_ul(m_browse_saves_btn->RelativeLowerRight().x + CONTROL_MARGIN, m_browse_saves_btn->RelativeUpperLeft().y + 2);
    GG::Pt save_file_text_lr = save_file_text_ul + m_save_file_text->MinUsableSize();
    m_save_file_text->SizeMove(save_file_text_ul, save_file_text_lr);

    g_preview_ul = GG::Pt(ClientWidth() - PREVIEW_SZ.x - CONTROL_MARGIN - PREVIEW_MARGIN, GG::Y(CONTROL_MARGIN + PREVIEW_MARGIN));
    m_preview_image->SizeMove(g_preview_ul, PREVIEW_SZ);

    x = CHAT_WIDTH + CONTROL_MARGIN;
    GG::Y y = std::max(m_save_file_text->RelativeLowerRight().y, m_preview_image->RelativeLowerRight().y) + 5*CONTROL_MARGIN;
    const GG::Y TEXT_HEIGHT = GG::Y(ClientUI::Pts() * 3/2);


    GG::Pt players_lb_labels_ul(x + CONTROL_MARGIN, y);
    GG::Pt players_lb_labels_lr= players_lb_labels_ul + GG::Pt(EMPIRE_NAME_WIDTH, TEXT_HEIGHT);
    GG::Pt players_lb_labels_advance(EMPIRE_NAME_WIDTH - 45, GG::Y0);

    m_players_lb_player_type_label->SizeMove(players_lb_labels_ul, players_lb_labels_lr);
    players_lb_labels_ul += players_lb_labels_advance;
    players_lb_labels_lr += players_lb_labels_advance;
    m_players_lb_player_name_column_label->SizeMove(players_lb_labels_ul, players_lb_labels_lr);
    players_lb_labels_ul += players_lb_labels_advance;
    players_lb_labels_lr += players_lb_labels_advance;
    m_players_lb_empire_name_column_label->SizeMove(players_lb_labels_ul, players_lb_labels_lr);
    players_lb_labels_ul += players_lb_labels_advance;
    players_lb_labels_lr += players_lb_labels_advance;
    m_players_lb_empire_colour_column_label->SizeMove(players_lb_labels_ul, players_lb_labels_lr);
    players_lb_labels_ul += players_lb_labels_advance;
    players_lb_labels_lr += players_lb_labels_advance;
    m_players_lb_species_or_original_player_label->SizeMove(players_lb_labels_ul, players_lb_labels_lr);

    y += TEXT_HEIGHT + CONTROL_MARGIN;
    x = CHAT_WIDTH + CONTROL_MARGIN;

    GG::Pt players_lb_ul(x, y);
    GG::Pt players_lb_lr = players_lb_ul + GG::Pt(ClientWidth() - CONTROL_MARGIN - x, m_chat_input_edit->RelativeUpperLeft().y - CONTROL_MARGIN - y);
    m_players_lb->SizeMove(players_lb_ul, players_lb_lr);

    m_start_game_bn->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(GG::X(125), m_start_game_bn->MinUsableSize().y));
    m_cancel_bn->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(GG::X(125), m_start_game_bn->MinUsableSize().y));
    m_cancel_bn->MoveTo(GG::Pt(ClientWidth() - m_cancel_bn->Width() - CONTROL_MARGIN, ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN));
    m_start_game_bn->MoveTo(GG::Pt(m_cancel_bn->RelativeUpperLeft().x - CONTROL_MARGIN - m_start_game_bn->Width(),
                                   ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN));

    GG::Pt start_conditions_text_ul(x, ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN);
    GG::Pt start_conditions_text_lr = start_conditions_text_ul + GG::Pt(m_cancel_bn->RelativeUpperLeft().x - x, TEXT_HEIGHT);
    m_start_conditions_text->SizeMove(start_conditions_text_ul, start_conditions_text_lr);
}

void MultiPlayerLobbyWnd::DisableTypingUnsafeAccels()
{ HotkeyManager::GetManager()->EnableTypingUnsafeHotkeys(); }

void MultiPlayerLobbyWnd::EnableTypingUnsafeAccels()
{ HotkeyManager::GetManager()->DisableTypingUnsafeHotkeys(); }

void MultiPlayerLobbyWnd::NewLoadClicked(std::size_t idx) {
    switch (idx) {
    case std::size_t(0):
        m_lobby_data.m_new_game = true;
        m_galaxy_setup_panel->Disable(false);
        m_save_file_text->Disable();
        m_browse_saves_btn->Disable();
        break;
    case std::size_t(1):
        m_lobby_data.m_new_game = false;
        m_galaxy_setup_panel->Disable();
        m_save_file_text->Disable(false);
        m_browse_saves_btn->Disable(false);
        break;
    default:
        break;
    }
    PopulatePlayerList();
    SendUpdate();
}

void MultiPlayerLobbyWnd::GalaxySetupPanelChanged() {
    m_galaxy_setup_panel->GetSetupData(m_lobby_data);
    SendUpdate();
}

void MultiPlayerLobbyWnd::SaveGameBrowse() {
    m_lobby_data.m_save_game = HumanClientApp::GetApp()->SelectLoadFile();
    m_lobby_data.m_save_game_empire_data.clear();
    PopulatePlayerList();
    SendUpdate();
}

void MultiPlayerLobbyWnd::PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image) {
    if (m_preview_image) {
        DeleteChild(m_preview_image);
        m_preview_image = 0;
    }
    m_preview_image = new GG::StaticGraphic(g_preview_ul.x, g_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, new_image, GG::GRAPHIC_FITGRAPHIC);
    AttachChild(m_preview_image);
}

void MultiPlayerLobbyWnd::PlayerDataChangedLocally() {
    m_lobby_data.m_players.clear();
    for (GG::ListBox::iterator it = m_players_lb->begin(); it != m_players_lb->end(); ++it) {
        const PlayerRow* row = dynamic_cast<const PlayerRow*>(*it);
        if (const EmptyPlayerRow* empty_row = dynamic_cast<const EmptyPlayerRow*>(row)) {
            // empty rows that have been changed to Add AI need to be sent so the server knows to add an AI player.
            if (empty_row->m_player_data.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
                m_lobby_data.m_players.push_back(std::make_pair(Networking::INVALID_PLAYER_ID, row->m_player_data));

            // empty rows that are still showing no player don't need to be sent to the server.

        } else {
            // all other row types pass along data directly
            m_lobby_data.m_players.push_back(std::make_pair(row->m_player_id, row->m_player_data));
        }
    }

    m_start_game_bn->Disable(!ThisClientIsHost() || !CanStart());
    SendUpdate();
}

bool MultiPlayerLobbyWnd::PopulatePlayerList() {
    bool send_update_back_retval = false;

    // store list position to restore after update
    int initial_list_scroll_pos = std::distance(m_players_lb->begin(), m_players_lb->FirstRowShown());

    m_players_lb->Clear();

    // repopulate list with rows built from current lobby data
    for (std::list<std::pair<int, PlayerSetupData> >::iterator player_setup_it = m_lobby_data.m_players.begin();
         player_setup_it != m_lobby_data.m_players.end(); ++player_setup_it)
    {
        int data_player_id = player_setup_it->first;
        PlayerSetupData& psd = player_setup_it->second;

        if (m_lobby_data.m_new_game) {
            bool immutable_row = !ThisClientIsHost() && (data_player_id != HumanClientApp::GetApp()->PlayerID());   // host can modify any player's row.  non-hosts can only modify their own row.  As of SVN 4026 this is not enforced on the server, but should be.
            NewGamePlayerRow* row = new NewGamePlayerRow(psd, data_player_id, immutable_row);
            m_players_lb->Insert(row);
            Connect(row->DataChangedSignal, &MultiPlayerLobbyWnd::PlayerDataChangedLocally, this);

        } else {
            bool immutable_row = (!ThisClientIsHost() && (data_player_id != HumanClientApp::GetApp()->PlayerID())) || m_lobby_data.m_save_game_empire_data.empty();
            LoadGamePlayerRow* row = new LoadGamePlayerRow(psd, data_player_id, m_lobby_data.m_save_game_empire_data, immutable_row);
            m_players_lb->Insert(row);
            Connect(row->DataChangedSignal, &MultiPlayerLobbyWnd::PlayerDataChangedLocally, this);

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

    // on host, add extra empty row, which the hose can use to select
    // "Add AI" to add an AI to the game.  This row's details are treated
    // specially when sending a lobby update to the server.
    if (ThisClientIsHost()) {
        EmptyPlayerRow* row = new EmptyPlayerRow();
        m_players_lb->Insert(row);
        Connect(row->DataChangedSignal, &MultiPlayerLobbyWnd::PlayerDataChangedLocally, this);
    }

    if (m_lobby_data.m_new_game) {
        m_players_lb_species_or_original_player_label->SetText(UserString("MULTIPLAYER_PLAYER_LIST_STARTING_SPECIES"));
    } else {
        m_players_lb_species_or_original_player_label->SetText(UserString("MULTIPLAYER_PLAYER_LIST_ORIGINAL_NAMES"));
    }

    // restore list scroll position
    GG::ListBox::iterator first_row_it = m_players_lb->FirstRowShown();
    int first_row_to_show = std::max<int>(0, std::min<int>(initial_list_scroll_pos,
                                                           m_players_lb->NumRows() - 1));
    std::advance(first_row_it, first_row_to_show);
    m_players_lb->SetFirstRowShown(first_row_it);


    Refresh();

    return send_update_back_retval;
}

void MultiPlayerLobbyWnd::SendUpdate() {
    int player_id = HumanClientApp::GetApp()->PlayerID();
    if (player_id != Networking::INVALID_PLAYER_ID)
        HumanClientApp::GetApp()->Networking().SendMessage(LobbyUpdateMessage(player_id, m_lobby_data));
}

bool MultiPlayerLobbyWnd::PlayerDataAcceptable() const {
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
        } else if (row.m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                   row.m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        {
            // do nothing special for this player
        } else {
            if (dynamic_cast<const EmptyPlayerRow*>(*it)) {
                // ignore empty player row
            } else {
                Logger().errorStream() << "MultiPlayerLobbyWnd::PlayerDataAcceptable found not empty player row with unrecognized client type?!";
                return false;
            }
        }
    }
    // any duplicate names or colours will means that the number of active
    // players and number of colours / names won't match
    return num_players_excluding_observers > 0 &&
           static_cast<int>(empire_names.size()) == num_players_excluding_observers &&
           static_cast<int>(empire_colors.size()) == num_players_excluding_observers;
}

bool MultiPlayerLobbyWnd::CanStart() const
{ return PlayerDataAcceptable(); }

bool MultiPlayerLobbyWnd::ThisClientIsHost() const
{ return HumanClientApp::GetApp()->Networking().PlayerIsHost(HumanClientApp::GetApp()->Networking().PlayerID()); }

void MultiPlayerLobbyWnd::StartGameClicked() {
    if (CanStart() && ThisClientIsHost()) {
        m_start_game_bn->Disable();
        HumanClientApp::GetApp()->StartMultiPlayerGameFromLobby();
    }
}

void MultiPlayerLobbyWnd::CancelClicked()
{ HumanClientApp::GetApp()->CancelMultiplayerGameFromLobby(); }
