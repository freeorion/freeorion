#include "MultiplayerLobbyWnd.h"

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Serialize.h"
#include "../network/Message.h"
#include "../network/ClientNetworking.h"
#include "../client/human/HumanClientApp.h"
#include "Hotkeys.h"
#include "Sound.h"

#include <boost/cast.hpp>
#include <boost/serialization/vector.hpp>


namespace {
    // Margin between text and row edge.
    GG::Y PlayerRowMargin()
    { return GG::Y(GG::ListBox::DEFAULT_MARGIN + std::max(CUIEdit::PIXEL_MARGIN, GG::ListBox::DEFAULT_MARGIN)); }
    GG::Y PlayerFontHeight()
    { return ClientUI::GetFont(ClientUI::Pts())->Height(); }
    GG::Y PlayerRowHeight()
    { return PlayerFontHeight() + 2 * PlayerRowMargin(); }
    GG::X PlayerReadyBrowseWidth()
    { return GG::X(ClientUI::Pts() * 11); }

    const std::shared_ptr<GG::Texture> GetReadyTexture(bool ready) {
        if (ready)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons/ready.png");
        return ClientUI::GetTexture(ClientUI::ArtDir() / "icons/not_ready.png");
    }

    const std::shared_ptr<GG::Texture> GetHostTexture() {
        return ClientUI::GetTexture(ClientUI::ArtDir() / "icons/host.png");
    }

    const GG::X EMPIRE_NAME_WIDTH(150);
    const GG::X BROWSE_BTN_WIDTH(50);

    // Shows information about a single player in the mulitplayer lobby.
    // This inclues whether the player is a human or AI player, or an observer,
    // the player's name, empire's name and colour, and the starting species.
    // Can also be used to allow some of that information to be changed by
    // players or the host.
    struct PlayerRow : GG::ListBox::Row {
        PlayerRow() :
            GG::ListBox::Row(GG::X(90), PlayerRowHeight(), ""),
            m_player_data(),
            m_player_id(Networking::INVALID_PLAYER_ID)
        {}
        PlayerRow(const PlayerSetupData& player_data, int player_id) :
            GG::ListBox::Row(GG::X(90), PlayerRowHeight(), ""),
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
        class Spacer: public GG::Control {
            public:
            Spacer() {};

            void Render() override
            {}
        };

        // fills player type selection droplist in player row
        class TypeRow : public GG::DropDownList::Row {
        public:
            TypeRow() :
                GG::DropDownList::Row(),
                type(Networking::INVALID_CLIENT_TYPE),
                m_label(GG::Wnd::Create<CUILabel>(UserString("NO_PLAYER")))
            {}

            TypeRow(GG::X w, GG::Y h, Networking::ClientType type_, bool show_add_drop = false) :
                GG::DropDownList::Row(w, h, "PlayerTypeSelectorRow"),
                type(type_)
            {
                switch (type) {
                case Networking::CLIENT_TYPE_AI_PLAYER:
                    if (show_add_drop)
                        m_label = GG::Wnd::Create<CUILabel>(UserString("ADD_AI_PLAYER"));
                    else
                        m_label = GG::Wnd::Create<CUILabel>(UserString("AI_PLAYER"));
                    break;
                case Networking::CLIENT_TYPE_HUMAN_OBSERVER:
                    m_label = GG::Wnd::Create<CUILabel>(UserString("OBSERVER"));
                    break;
                case Networking::CLIENT_TYPE_HUMAN_PLAYER:
                    m_label = GG::Wnd::Create<CUILabel>(UserString("PLAYER"));
                    break;
                case Networking::CLIENT_TYPE_HUMAN_MODERATOR:
                    m_label = GG::Wnd::Create<CUILabel>(UserString("MODERATOR"));
                    break;
                default:
                    if (show_add_drop)
                        m_label = GG::Wnd::Create<CUILabel>(UserString("DROP_PLAYER"));
                    else
                        m_label = GG::Wnd::Create<CUILabel>(UserString("NO_PLAYER"));
                }
            }

            void CompleteConstruction() override {
                GG::ListBox::Row::CompleteConstruction();
                push_back(m_label);
                push_back(GG::Wnd::Create<Spacer>());
            }

            Networking::ClientType type;
        private:
            std::shared_ptr<CUILabel> m_label;
        };

    public:
        TypeSelector(GG::X w, GG::Y h, Networking::ClientType client_type, bool disabled) :
            CUIDropDownList(6)
        {
            SetStyle(GG::LIST_NOSORT);
            ManuallyManageColProps();
            NormalizeRowsOnInsert(false);
            SetNumCols(2);
            SetColStretch(0, 0.0);
            SetColStretch(1, 1.0);
            SetColAlignment(0, GG::ALIGN_CENTER);
            GG::Y type_row_height(std::max(GG::Y1, h - 8));
            Resize(GG::Pt(w, type_row_height));
            SetColWidth(0, CUIDropDownList::DisplayedRowWidth());

            if (client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
                if (disabled) {
                    // For AI players on non-hosts, have "AI" shown (disabled)
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::CLIENT_TYPE_AI_PLAYER));      // static "AI" display
                    Select(0);
                } else {
                    // For AI players on host, have "AI" shown on droplist, with "Drop" shown as alternate selection to remove the AI
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::CLIENT_TYPE_AI_PLAYER));      // "AI" display
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::INVALID_CLIENT_TYPE, true));  // "Drop" option
                    Select(0);
                }
            } else if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
                       client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                       client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
            {
                if (disabled) {
                    // For human players on other players non-host, have "AI" or "Observer" indicator (disabled)
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, client_type));
                    Select(0);
                } else {
                    // For human players on host, have "Player", "Observer", and "Drop" options.  TODO: have "Ban" option.
                    int row_player_type = -1;
                    int row_observer_type = -1;
                    int row_moderator_type = -1;
                    int row_number = 0;
                    if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
                        ClientApp::GetApp()->Networking().HasAuthRole(Networking::ROLE_CLIENT_TYPE_PLAYER))
                    {
                        Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::CLIENT_TYPE_HUMAN_PLAYER));   // "Human" display / option
                        row_player_type = (row_number++);
                    }

                    if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                        ClientApp::GetApp()->Networking().HasAuthRole(Networking::ROLE_CLIENT_TYPE_OBSERVER))
                    {
                        Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::CLIENT_TYPE_HUMAN_OBSERVER)); // "Observer" display / option
                        row_observer_type = (row_number++);
                    }

                    if (client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR ||
                        ClientApp::GetApp()->Networking().HasAuthRole(Networking::ROLE_CLIENT_TYPE_MODERATOR))
                    {
                        Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::CLIENT_TYPE_HUMAN_MODERATOR));// "Moderator" display / option
                        row_moderator_type = (row_number++);
                    }
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::INVALID_CLIENT_TYPE, true));  // "Drop" option

                    if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                        Select(row_player_type);
                    else if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
                        Select(row_observer_type);
                    else if (client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
                        Select(row_moderator_type);
                    else
                        Select(row_number);
                }
            } else {
                if (disabled) {
                    // For empty row on non-host, should probably have no row... could also put "None" (disabled)
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::INVALID_CLIENT_TYPE));
                    Select(0);
                } else {
                    // For empty row on host, have "None" and "Add AI" options on droplist
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::INVALID_CLIENT_TYPE));        // "None" display
                    Insert(GG::Wnd::Create<TypeRow>(w, type_row_height, Networking::CLIENT_TYPE_AI_PLAYER, true));// "Add AI" option

                    Select(0);
                }
            }

            SelChangedSignal.connect(
                boost::bind(&TypeSelector::SelectionChanged, this, _1));
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            GG::Pt old_size(Size());
            CUIDropDownList::SizeMove(ul, lr);
            if (old_size != Size())
                SetColWidth(0, CUIDropDownList::DisplayedRowWidth());
        }

        void SelectionChanged(GG::DropDownList::iterator it)
        {
            if (it == this->end())
                return;

            const auto& row = *it;
            if (!row)
                return;

            const TypeRow* type_row = boost::polymorphic_downcast<const TypeRow*>(row.get());
            if (!type_row)
                return;

            TypeChangedSignal(type_row->type);
        }

        mutable boost::signals2::signal<void (Networking::ClientType)> TypeChangedSignal;
    };


    // Row for indicating / manipulating info about a player when creating a new game
    struct NewGamePlayerRow : PlayerRow {
        NewGamePlayerRow(const PlayerSetupData& player_data, int player_id, bool disabled) :
            PlayerRow(player_data, player_id),
            m_initial_disabled(disabled)
        {}

        void CompleteConstruction() override {
            PlayerRow::CompleteConstruction();

            // human / AI / observer indicator / selector
            auto type_drop = GG::Wnd::Create<TypeSelector>(GG::X(90), PlayerRowHeight(), m_player_data.m_client_type, m_initial_disabled);
            push_back(type_drop);
            if (m_initial_disabled)
                type_drop->Disable();
            else
                type_drop->TypeChangedSignal.connect(
                    boost::bind(&NewGamePlayerRow::PlayerTypeChanged, this, _1));

            // player name text
            push_back(GG::Wnd::Create<CUILabel>(m_player_data.m_player_name));

            if (m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR) {
                // observers don't need to pick an empire or species
                push_back(GG::Wnd::Create<CUILabel>(""));
                push_back(GG::Wnd::Create<CUILabel>(""));
                push_back(GG::Wnd::Create<CUILabel>(""));
                push_back(GG::Wnd::Create<GG::StaticGraphic>(GetReadyTexture(m_player_data.m_player_ready),
                                                             GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE));
                at(5)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));
                at(5)->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
                at(5)->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                    m_player_data.m_player_ready ? UserString("READY_BN") : UserString("NOT_READY_BN"),
                    "", PlayerReadyBrowseWidth()));
                if (HumanClientApp::GetApp()->Networking().PlayerIsHost(m_player_id)) {
                    push_back(GG::Wnd::Create<GG::StaticGraphic>(GetHostTexture(),
                                                                 GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE));
                } else {
                    push_back(GG::Wnd::Create<CUILabel>(""));
                }
                at(6)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));

                return;
            }

            // empire name editable text
            auto edit = GG::Wnd::Create<CUIEdit>(m_player_data.m_empire_name);
            edit->SetColor(GG::CLR_ZERO);
            edit->SetInteriorColor(GG::CLR_ZERO);
            edit->Resize(GG::Pt(EMPIRE_NAME_WIDTH, edit->MinUsableSize().y));
            push_back(edit);
            if (m_initial_disabled)
                edit->Disable();
            else
                edit->FocusUpdateSignal.connect(
                    boost::bind(&NewGamePlayerRow::EmpireNameChanged, this, _1));

            // empire colour selector
            auto color_selector = GG::Wnd::Create<EmpireColorSelector>(PlayerFontHeight() + PlayerRowMargin());
            color_selector->SelectColor(m_player_data.m_empire_color);
            push_back(color_selector);
            if (m_initial_disabled)
                color_selector->Disable();
            else
                color_selector->ColorChangedSignal.connect(
                    boost::bind(&NewGamePlayerRow::ColorChanged, this, _1));

            // species selector
            auto species_selector = GG::Wnd::Create<SpeciesSelector>(m_player_data.m_starting_species_name, EMPIRE_NAME_WIDTH, PlayerRowHeight());
            push_back(species_selector);
            if (m_initial_disabled)
                species_selector->Disable();
            else
                species_selector->SpeciesChangedSignal.connect(
                    boost::bind(&NewGamePlayerRow::SpeciesChanged, this, _1));

            // ready state
            if (m_player_data.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
                push_back(GG::Wnd::Create<CUILabel>(""));
                at(5)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));
            } else {
                push_back(GG::Wnd::Create<GG::StaticGraphic>(GetReadyTexture(m_player_data.m_player_ready),
                                                             GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE));
                at(5)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));
                at(5)->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
                at(5)->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                    m_player_data.m_player_ready ? UserString("READY_BN") : UserString("NOT_READY_BN"),
                    "", PlayerReadyBrowseWidth()));
            }

            // host
            if (HumanClientApp::GetApp()->Networking().PlayerIsHost(m_player_id)) {
                push_back(GG::Wnd::Create<GG::StaticGraphic>(GetHostTexture(),
                                                             GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE));
            } else {
                push_back(GG::Wnd::Create<CUILabel>(""));
            }
            at(6)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));

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

        bool                                     m_initial_disabled;
    };

    // Row for player info when loading a game
    struct LoadGamePlayerRow : PlayerRow {
        LoadGamePlayerRow(const PlayerSetupData& player_data, int player_id, const std::map<int, SaveGameEmpireData>& save_game_empire_data, bool disabled) :
            PlayerRow(player_data, player_id),
            m_empire_list(nullptr),
            m_save_game_empire_data(save_game_empire_data),
            m_initial_disabled(disabled)
        {}

        void CompleteConstruction() override {
            PlayerRow::CompleteConstruction();

            // human / AI / observer indicator / selector
            auto type_drop = GG::Wnd::Create<TypeSelector>(GG::X(90), PlayerRowHeight(), m_player_data.m_client_type, m_initial_disabled);
            push_back(type_drop);
            if (m_initial_disabled)
                type_drop->Disable();
            else
                type_drop->TypeChangedSignal.connect(
                    boost::bind(&LoadGamePlayerRow::PlayerTypeChanged, this, _1));

            // player name text
            push_back(GG::Wnd::Create<CUILabel>(m_player_data.m_player_name));

            // droplist to select empire
            m_empire_list = GG::Wnd::Create<CUIDropDownList>(6);
            m_empire_list->Resize(GG::Pt(EMPIRE_NAME_WIDTH, PlayerRowHeight()));
            m_empire_list->SetStyle(GG::LIST_NOSORT);
            auto save_game_empire_it = m_save_game_empire_data.end();
            for (auto it = m_save_game_empire_data.begin();
                 it != m_save_game_empire_data.end(); ++it)
            {
                // insert row into droplist of empires for this player row
                m_empire_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(it->second.m_empire_name));

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
            m_color_selector = GG::Wnd::Create<EmpireColorSelector>(PlayerFontHeight() + PlayerRowMargin());
            m_color_selector->SelectColor(m_player_data.m_empire_color);
            push_back(m_color_selector);

            // original empire player name from saved game
            push_back(GG::Wnd::Create<CUILabel>(save_game_empire_it != m_save_game_empire_data.end() ? save_game_empire_it->second.m_player_name : ""));

            m_color_selector->Disable();

            if (m_initial_disabled)
                m_empire_list->Disable();
            else
                m_empire_list->SelChangedSignal.connect(
                    boost::bind(&LoadGamePlayerRow::EmpireChanged, this, _1));

            // ready state
            if (m_player_data.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
                push_back(GG::Wnd::Create<CUILabel>(""));
                at(5)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));
            } else {
                push_back(GG::Wnd::Create<GG::StaticGraphic>(GetReadyTexture(m_player_data.m_player_ready),
                                                             GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE));
                at(5)->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
                at(5)->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                    m_player_data.m_player_ready ? UserString("READY_BN") : UserString("NOT_READY_BN"),
                    "", PlayerReadyBrowseWidth()));
                at(5)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));
            }

            // host
            if (HumanClientApp::GetApp()->Networking().PlayerIsHost(m_player_id)) {
                push_back(GG::Wnd::Create<GG::StaticGraphic>(GetHostTexture(),
                                                             GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE));
            } else {
                push_back(GG::Wnd::Create<CUILabel>(""));
            }
            at(6)->SetMinSize(GG::Pt(GG::X(ClientUI::Pts()), PlayerFontHeight()));
        }

    private:
        void PlayerTypeChanged(Networking::ClientType type) {
            m_player_data.m_client_type = type;
            DataChangedSignal();
        }
        void EmpireChanged(GG::DropDownList::iterator selected_it) {
            if (selected_it == m_empire_list->end()) {
                ErrorLogger() << "Empire changed to no empire.  Ignoring change.";
                return;
            }
            auto it = m_save_game_empire_data.begin();
            std::advance(it, m_empire_list->IteratorToIndex(selected_it));
            m_player_data.m_empire_name =           it->second.m_empire_name;
            m_player_data.m_empire_color =          it->second.m_color;
            m_player_data.m_save_game_empire_id =   it->second.m_empire_id;
            m_color_selector->SelectColor(m_player_data.m_empire_color);

            // set previous player name indication
            if (size() >= 5)
                boost::polymorphic_downcast<GG::Label*>(at(4))->SetText(it->second.m_player_name);

            DataChangedSignal();
        }

        std::shared_ptr<EmpireColorSelector>                     m_color_selector;
        std::shared_ptr<GG::DropDownList>                        m_empire_list;
        const std::map<int, SaveGameEmpireData>& m_save_game_empire_data;
        bool                                     m_initial_disabled;
    };

    // Row for indicating that an AI client should be added to the game
    struct EmptyPlayerRow : PlayerRow {
        EmptyPlayerRow() :
            PlayerRow()
        {}

        void CompleteConstruction() override {
            PlayerRow::CompleteConstruction();

            auto type_drop = GG::Wnd::Create<TypeSelector>(GG::X(90), PlayerRowHeight(), Networking::INVALID_CLIENT_TYPE, false);
            push_back(type_drop);
            type_drop->TypeChangedSignal.connect(
                boost::bind(&EmptyPlayerRow::PlayerTypeChanged, this, _1));
            // extra entries to make layout consistent
            push_back(GG::Wnd::Create<CUILabel>(""));
            push_back(GG::Wnd::Create<CUILabel>(""));
            push_back(GG::Wnd::Create<CUILabel>(""));
            push_back(GG::Wnd::Create<CUILabel>(""));
            push_back(GG::Wnd::Create<CUILabel>(""));
            push_back(GG::Wnd::Create<CUILabel>(""));
        }
    private:
        void PlayerTypeChanged(Networking::ClientType type) {
            m_player_data.m_client_type = type;
            DataChangedSignal();
        }
    };

    const GG::X     LOBBY_WND_WIDTH(960);
    const GG::Y     LOBBY_WND_HEIGHT(720);
    const int       CONTROL_MARGIN = 5; // gap to leave between controls in the window
    const GG::X     GALAXY_SETUP_PANEL_WIDTH(250);
    const GG::Y     GALAXY_SETUP_PANEL_HEIGHT(340);
    const GG::Y     SAVED_GAMES_LIST_ROW_HEIGHT(22);
    const GG::Y     SAVED_GAMES_LIST_DROP_HEIGHT(10 * SAVED_GAMES_LIST_ROW_HEIGHT);
    const GG::X     CHAT_WIDTH(350);
    GG::Pt          g_preview_ul;
    const GG::Pt    PREVIEW_SZ(GG::X(248), GG::Y(186));
    const int       PREVIEW_MARGIN = 3;

    std::vector<GG::X> PlayerRowColWidths(GG::X width = GG::X(600)) {
        std::vector<GG::X> retval;
        GG::X color_width(75);
        GG::X ready_width((ClientUI::Pts() / 2) * 5);
        GG::X prop_width = ((width - color_width - 2 * ready_width) / 4) - CONTROL_MARGIN;
        retval.push_back(prop_width); // type
        retval.push_back(prop_width); // player name
        retval.push_back(prop_width); // empire name
        retval.push_back(color_width); // color
        retval.push_back(prop_width); // species/original player
        retval.push_back(ready_width); // player ready
        retval.push_back(ready_width); // host
        return retval;
    }
}

MultiPlayerLobbyWnd::MultiPlayerLobbyWnd() :
    CUIWnd(UserString("MPLOBBY_WINDOW_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::RESIZABLE),
    m_chat_wnd(nullptr),
    m_any_can_edit(nullptr),
    m_new_load_game_buttons(nullptr),
    m_galaxy_setup_panel(nullptr),
    m_browse_saves_btn(nullptr),
    m_preview_image(nullptr),
    m_players_lb(nullptr),
    m_players_lb_headers(nullptr),
    m_ready_bn(nullptr),
    m_cancel_bn(nullptr),
    m_start_conditions_text(nullptr)
{}

void MultiPlayerLobbyWnd::CompleteConstruction() {
    Sound::TempUISoundDisabler sound_disabler;

    m_chat_wnd = GG::Wnd::Create<MessageWnd>(GG::INTERACTIVE, "mplobby.chat");

    m_any_can_edit = GG::Wnd::Create<CUIStateButton>(UserString("EDITABLE_GALAXY_SETTINGS"), GG::FORMAT_LEFT, std::make_shared<CUICheckBoxRepresenter>());
    m_any_can_edit->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_any_can_edit->SetBrowseText(UserString("EDITABLE_GALAXY_SETTINGS_DESC"));

    m_galaxy_setup_panel = GG::Wnd::Create<GalaxySetupPanel>(GALAXY_SETUP_PANEL_WIDTH, GALAXY_SETUP_PANEL_HEIGHT);

    m_new_load_game_buttons = GG::Wnd::Create<GG::RadioButtonGroup>(GG::VERTICAL);
    m_new_load_game_buttons->AddButton(
        GG::Wnd::Create<CUIStateButton>(UserString("NEW_GAME_BN"), GG::FORMAT_LEFT, std::make_shared<CUIRadioRepresenter>()));
    m_new_load_game_buttons->AddButton(
        GG::Wnd::Create<CUIStateButton>(UserString("LOAD_GAME_BN"), GG::FORMAT_LEFT, std::make_shared<CUIRadioRepresenter>()));

    m_browse_saves_btn = Wnd::Create<CUIButton>("...");
    m_save_file_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_NOWRAP);

    m_preview_image = GG::Wnd::Create<GG::StaticGraphic>(std::make_shared<GG::Texture>(), GG::GRAPHIC_FITGRAPHIC);

    m_players_lb_headers = GG::Wnd::Create<PlayerLabelRow>();
    m_players_lb_headers->SetMinSize(GG::Pt(GG::X(0), PlayerRowHeight() + PlayerFontHeight()));

    m_players_lb = GG::Wnd::Create<CUIListBox>();
    m_players_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
    m_players_lb->LockColWidths();
    m_players_lb->ManuallyManageColProps();
    m_players_lb->SetNumCols(7);
    std::vector<GG::X> col_widths = PlayerRowColWidths();
    for (unsigned int i = 0; i < m_players_lb->NumCols(); ++i) {
        m_players_lb->SetColWidth(i, col_widths[i]);
        m_players_lb->SetColAlignment(i, GG::ALIGN_VCENTER);
    }
    m_players_lb->SetColHeaders(m_players_lb_headers);

    m_ready_bn = Wnd::Create<CUIButton>(UserString("READY_BN"));
    m_cancel_bn = Wnd::Create<CUIButton>(UserString("CANCEL"));

    m_start_conditions_text = GG::Wnd::Create<CUILabel>(UserString("MULTIPLAYER_GAME_START_CONDITIONS"), GG::FORMAT_LEFT);

    AttachChild(m_chat_wnd);
    AttachChild(m_any_can_edit);
    AttachChild(m_new_load_game_buttons);
    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_save_file_text);
    AttachChild(m_browse_saves_btn);
    AttachChild(m_preview_image);
    AttachChild(m_players_lb);
    AttachChild(m_ready_bn);
    AttachChild(m_cancel_bn);
    AttachChild(m_start_conditions_text);

    ResetDefaultPosition();
    SetMinSize(GG::Pt(LOBBY_WND_WIDTH, LOBBY_WND_HEIGHT));
    DoLayout();
    SaveDefaultedOptions();

    // default settings (new game)
    m_new_load_game_buttons->SetCheck(0);
    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());
    m_save_file_text->Disable();
    m_browse_saves_btn->Disable();

    m_any_can_edit->CheckedSignal.connect(
        boost::bind(&MultiPlayerLobbyWnd::AnyCanEdit, this, _1));
    m_new_load_game_buttons->ButtonChangedSignal.connect(
        boost::bind(&MultiPlayerLobbyWnd::NewLoadClicked, this, _1));
    m_galaxy_setup_panel->SettingsChangedSignal.connect(
        boost::bind(&MultiPlayerLobbyWnd::GalaxySetupPanelChanged, this));
    m_browse_saves_btn->LeftClickedSignal.connect(
        boost::bind(&MultiPlayerLobbyWnd::SaveGameBrowse, this));
    m_ready_bn->LeftClickedSignal.connect(
        boost::bind(&MultiPlayerLobbyWnd::ReadyClicked, this));
    m_galaxy_setup_panel->ImageChangedSignal.connect(
        boost::bind(&MultiPlayerLobbyWnd::PreviewImageChanged, this, _1));
    m_cancel_bn->LeftClickedSignal.connect(
        boost::bind(&MultiPlayerLobbyWnd::CancelClicked, this));

    CUIWnd::CompleteConstruction();

    Refresh();
}

MultiPlayerLobbyWnd::PlayerLabelRow::PlayerLabelRow(GG::X width /* = GG::X(580)*/) :
    GG::ListBox::Row(width, PlayerRowHeight(), "")
{}

void MultiPlayerLobbyWnd::PlayerLabelRow::CompleteConstruction() {

    GG::ListBox::Row::CompleteConstruction();

    push_back(GG::Wnd::Create<CUILabel>(UserString("MULTIPLAYER_PLAYER_LIST_TYPES"), GG::FORMAT_BOTTOM));
    push_back(GG::Wnd::Create<CUILabel>(UserString("MULTIPLAYER_PLAYER_LIST_NAMES"), GG::FORMAT_BOTTOM));
    push_back(GG::Wnd::Create<CUILabel>(UserString("MULTIPLAYER_PLAYER_LIST_EMPIRES"), GG::FORMAT_BOTTOM));
    push_back(GG::Wnd::Create<CUILabel>(UserString("MULTIPLAYER_PLAYER_LIST_COLOURS"), GG::FORMAT_BOTTOM | GG::FORMAT_WORDBREAK));
    push_back(GG::Wnd::Create<CUILabel>(UserString("MULTIPLAYER_PLAYER_LIST_ORIGINAL_NAMES"), GG::FORMAT_BOTTOM | GG::FORMAT_WORDBREAK));
    push_back(GG::Wnd::Create<GG::StaticGraphic>(GetReadyTexture(true), GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE));
    push_back(GG::Wnd::Create<GG::StaticGraphic>(GetHostTexture(), GG::GRAPHIC_CENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE));
    // restrict height of ready state icon
    at(5)->SetMaxSize(GG::Pt(GG::X(400), PlayerFontHeight()));
    at(6)->SetMaxSize(GG::Pt(GG::X(400), PlayerFontHeight()));
    std::vector<GG::X> col_widths = PlayerRowColWidths(Width());
    unsigned int i = 0;
    for (auto& control : m_cells) {
        control->SetChildClippingMode(ClipToWindow);
        if (GG::TextControl* tc = dynamic_cast<GG::TextControl*>(control.get()))
            tc->SetFont(ClientUI::GetBoldFont());
        control->Resize(GG::Pt(col_widths[i], PlayerRowHeight() + PlayerFontHeight()));
        ++i;
    }
    SetColWidths(col_widths);
    RequirePreRender();
}

void MultiPlayerLobbyWnd::PlayerLabelRow::SetText(size_t column, const std::string& str) {
    if (m_cells.size() < column) {
        ErrorLogger() << "Invalid column " << column << " for row with " << m_cells.size() << " columns";
        return;
    }
    if (GG::TextControl* tc = dynamic_cast<GG::TextControl*>(m_cells.at(column).get()))
        tc->SetText(str);
    else
        ErrorLogger() << "Unable to set text " << str << " for column " << column;
}

void MultiPlayerLobbyWnd::PlayerLabelRow::Render() {
    const GG::Clr& BG_CLR = ClientUI::WndOuterBorderColor();
    const GG::Clr& BORDER_CLR = ClientUI::WndInnerBorderColor();
    GG::Pt ul(UpperLeft().x + CONTROL_MARGIN, UpperLeft().y + CONTROL_MARGIN);
    GG::Pt lr(LowerRight().x - CONTROL_MARGIN, LowerRight().y - CONTROL_MARGIN);

    // background fill and border for each cell
    for (auto& cell : m_cells)
        GG::FlatRectangle(GG::Pt(cell->UpperLeft().x, ul.y), GG::Pt(cell->LowerRight().x, lr.y), BG_CLR, BORDER_CLR, 1);
}

void MultiPlayerLobbyWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

GG::Rect MultiPlayerLobbyWnd::CalculatePosition() const {
    GG::Pt new_ul((GG::GUI::GetGUI()->AppWidth() - LOBBY_WND_WIDTH) / 2,
                  (GG::GUI::GetGUI()->AppHeight() - LOBBY_WND_HEIGHT) / 2);
    GG::Pt new_sz(LOBBY_WND_WIDTH, LOBBY_WND_HEIGHT);
    return GG::Rect(new_ul, new_ul + new_sz);
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

void MultiPlayerLobbyWnd::KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (m_ready_bn && (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER)) {
        m_ready_bn->LeftClickedSignal();
    } else if (key == GG::GGK_ESCAPE) {
        m_cancel_bn->LeftClickedSignal();
    }
}

void MultiPlayerLobbyWnd::ChatMessage(int player_id, const boost::posix_time::ptime& timestamp, const std::string& msg) {
    // look up player name by ID
    std::string player_name{UserString("PLAYER") + " " + std::to_string(player_id)};
    GG::Clr text_color{ClientUI::GetClientUI()->TextColor()};
    if (player_id != Networking::INVALID_PLAYER_ID) {
        for (auto& entry : m_lobby_data.m_players) {
            if (entry.first != player_id || entry.first == Networking::INVALID_PLAYER_ID)
                continue;
            player_name = entry.second.m_player_name;
            text_color = entry.second.m_empire_color;
        }
    } else {
        // It's a server message. Don't set player name.
        player_name = "";
    }

    m_chat_wnd->HandlePlayerChatMessage(msg, player_name, text_color, timestamp, HumanClientApp::GetApp()->PlayerID());
}

void MultiPlayerLobbyWnd::ChatMessage(const std::string& message_text,
                                      const std::string& player_name,
                                      GG::Clr text_color,
                                      const boost::posix_time::ptime& timestamp)
{
    m_chat_wnd->HandlePlayerChatMessage(message_text, player_name, text_color, timestamp,
                                        HumanClientApp::GetApp()->PlayerID());
}

void MultiPlayerLobbyWnd::TurnPhaseUpdate(Message::TurnProgressPhase phase_id)
{ m_chat_wnd->HandleTurnPhaseUpdate(phase_id, true); }

namespace {
    void LogPlayerSetupData(const std::list<std::pair<int, PlayerSetupData>>& psd) {
        DebugLogger() << "PlayerSetupData:";
        for (const std::pair<int, PlayerSetupData>& entry : psd)
            DebugLogger() << std::to_string(entry.first) << " : "
                                   << entry.second.m_player_name << ", "
                                   << entry.second.m_client_type << ", "
                                   << entry.second.m_starting_species_name
                                   << (entry.second.m_player_ready ? ", Ready" : "");
    }
}

void MultiPlayerLobbyWnd::LobbyUpdate(const MultiplayerLobbyData& lobby_data) {
    m_new_load_game_buttons->SetCheck(!lobby_data.m_new_game);
    m_galaxy_setup_panel->SetFromSetupData(lobby_data);

    m_any_can_edit->SetCheck(lobby_data.m_any_can_edit);

    m_save_file_text->SetText(lobby_data.m_save_game);

    m_lobby_data = lobby_data;

    bool send_update_back = PopulatePlayerList();

    if (send_update_back && HasAuthRole(Networking::ROLE_GALAXY_SETUP))
        SendUpdate();

    LogPlayerSetupData(m_lobby_data.m_players);
}

void MultiPlayerLobbyWnd::Refresh() {
    m_any_can_edit->Disable(!HasAuthRole(Networking::ROLE_HOST));

    if (HasAuthRole(Networking::ROLE_GALAXY_SETUP)) {
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
}

void MultiPlayerLobbyWnd::CleanupChat()
{ m_chat_wnd->Clear(); }

GG::Pt MultiPlayerLobbyWnd::MinUsableSize() const
{ return GG::Pt(LOBBY_WND_WIDTH, LOBBY_WND_HEIGHT); }

void MultiPlayerLobbyWnd::DoLayout() {
    GG::X x(CONTROL_MARGIN);

    GG::Pt chat_box_ul(x, GG::Y(CONTROL_MARGIN));
    GG::Pt chat_box_lr(CHAT_WIDTH, ClientHeight() - CONTROL_MARGIN);
    m_chat_wnd->SizeMove(chat_box_ul, chat_box_lr);

    const GG::Y RADIO_BN_HT(ClientUI::Pts() + 4);

    GG::Pt galaxy_setup_panel_ul(CHAT_WIDTH + 2*CONTROL_MARGIN, RADIO_BN_HT);
    GG::Pt galaxy_setup_panel_lr = galaxy_setup_panel_ul + GG::Pt(GALAXY_SETUP_PANEL_WIDTH, GALAXY_SETUP_PANEL_HEIGHT);
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
    m_preview_image->SizeMove(g_preview_ul, g_preview_ul + PREVIEW_SZ);

    GG::Pt any_can_edit_ul(m_preview_image->RelativeUpperLeft().x + CONTROL_MARGIN,
                           m_preview_image->RelativeLowerRight().y + CONTROL_MARGIN);
    GG::Pt any_can_edit_lr = any_can_edit_ul + GG::Pt(GALAXY_SETUP_PANEL_WIDTH, RADIO_BN_HT);
    m_any_can_edit->SizeMove(any_can_edit_ul, any_can_edit_lr);

    const GG::Y TEXT_HEIGHT = GG::Y(ClientUI::Pts() * 3/2);

    m_ready_bn->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(GG::X(125), m_ready_bn->MinUsableSize().y));
    m_cancel_bn->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(GG::X(125), m_ready_bn->MinUsableSize().y));
    m_cancel_bn->MoveTo(GG::Pt(ClientWidth() - m_cancel_bn->Width() - CONTROL_MARGIN,
                               ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN));
    m_ready_bn->MoveTo(GG::Pt(m_cancel_bn->RelativeUpperLeft().x - CONTROL_MARGIN - m_ready_bn->Width(),
                              ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN));

    x = CHAT_WIDTH + CONTROL_MARGIN;
    GG::Y y(std::max(m_save_file_text->RelativeLowerRight().y, m_browse_saves_btn->RelativeLowerRight().y));
    GG::Pt players_lb_ul(x, std::max(y, m_any_can_edit->RelativeLowerRight().y) + CONTROL_MARGIN);
    GG::Pt players_lb_lr(ClientWidth() - CONTROL_MARGIN, m_cancel_bn->RelativeUpperLeft().y - CONTROL_MARGIN);
    m_players_lb->SizeMove(players_lb_ul, players_lb_lr);
    std::vector<GG::X> players_lb_col_widths = PlayerRowColWidths(players_lb_lr.x - players_lb_ul.x);
    m_players_lb_headers->SetColWidths(players_lb_col_widths);
    for (unsigned int i = 0; i < players_lb_col_widths.size(); ++i)
        m_players_lb->SetColWidth(i, players_lb_col_widths[i]);
    m_players_lb->RequirePreRender();

    GG::Pt start_conditions_text_ul(x, ClientHeight() - m_cancel_bn->Height() - CONTROL_MARGIN);
    GG::Pt start_conditions_text_lr = start_conditions_text_ul + GG::Pt(m_cancel_bn->RelativeUpperLeft().x - x, TEXT_HEIGHT);
    m_start_conditions_text->SizeMove(start_conditions_text_ul, start_conditions_text_lr);
}

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

void MultiPlayerLobbyWnd::PreviewImageChanged(std::shared_ptr<GG::Texture> new_image) {
    if (m_preview_image) {
        DetachChild(m_preview_image);
        m_preview_image = nullptr;
    }
    m_preview_image = GG::Wnd::Create<GG::StaticGraphic>(new_image, GG::GRAPHIC_FITGRAPHIC);
    AttachChild(m_preview_image);

    DoLayout();
}

void MultiPlayerLobbyWnd::PlayerDataChangedLocally() {
    m_lobby_data.m_players.clear();
    for (auto& row : *m_players_lb) {
        const PlayerRow* player_row = dynamic_cast<const PlayerRow*>(row.get());
        if (const EmptyPlayerRow* empty_row = dynamic_cast<const EmptyPlayerRow*>(player_row)) {
            // empty rows that have been changed to Add AI need to be sent so the server knows to add an AI player.
            if (empty_row->m_player_data.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
                m_lobby_data.m_players.push_back({Networking::INVALID_PLAYER_ID, player_row->m_player_data});

            // empty rows that are still showing no player don't need to be sent to the server.

        } else {
            // all other row types pass along data directly
            m_lobby_data.m_players.push_back({player_row->m_player_id, player_row->m_player_data});
        }
    }

    SendUpdate();
}

bool MultiPlayerLobbyWnd::PopulatePlayerList() {
    bool send_update_back_retval = false;
    std::vector<GG::X> col_widths = PlayerRowColWidths();

    // store list position to restore after update
    int initial_list_scroll_pos = std::distance(m_players_lb->begin(), m_players_lb->FirstRowShown());

    m_players_lb->Clear();
    m_players_lb_headers->SetColWidths(col_widths);
    for (unsigned int i = 0; i < col_widths.size(); ++i)
        m_players_lb->SetColWidth(i, col_widths[i]);

    bool is_client_ready = false;
    bool is_other_ready = true;

    // repopulate list with rows built from current lobby data
    for (std::pair<int, PlayerSetupData>& entry : m_lobby_data.m_players) {
        int data_player_id = entry.first;
        PlayerSetupData& psd = entry.second;

        if (m_lobby_data.m_new_game) {
            bool immutable_row = !HasAuthRole(Networking::ROLE_HOST) && (data_player_id != HumanClientApp::GetApp()->PlayerID()) && !(psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER && HasAuthRole(Networking::ROLE_GALAXY_SETUP));   // host can modify any player's row.  non-hosts can only modify their own row.  As of SVN 4026 this is not enforced on the server, but should be.
            auto row = GG::Wnd::Create<NewGamePlayerRow>(psd, data_player_id, immutable_row);
            m_players_lb->Insert(row);
            row->DataChangedSignal.connect(
                boost::bind(&MultiPlayerLobbyWnd::PlayerDataChangedLocally, this));

        } else {
            bool immutable_row = (!HasAuthRole(Networking::ROLE_HOST) && (data_player_id != HumanClientApp::GetApp()->PlayerID()) && !(psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER && HasAuthRole(Networking::ROLE_GALAXY_SETUP))) || m_lobby_data.m_save_game_empire_data.empty();
            auto row = GG::Wnd::Create<LoadGamePlayerRow>(psd, data_player_id, m_lobby_data.m_save_game_empire_data, immutable_row);
            m_players_lb->Insert(row);
            row->DataChangedSignal.connect(
                boost::bind(&MultiPlayerLobbyWnd::PlayerDataChangedLocally, this));

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

        // checks for ready button
        if (data_player_id == HumanClientApp::GetApp()->PlayerID())
            is_client_ready = psd.m_player_ready;
        else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
            psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        { is_other_ready = is_other_ready && psd.m_player_ready; }
    }

    // on host, add extra empty row, which the host can use to select
    // "Add AI" to add an AI to the game.  This row's details are treated
    // specially when sending a lobby update to the server.
    if (HasAuthRole(Networking::ROLE_GALAXY_SETUP)) {
        auto row = GG::Wnd::Create<EmptyPlayerRow>();
        m_players_lb->Insert(row);
        row->DataChangedSignal.connect(
            boost::bind(&MultiPlayerLobbyWnd::PlayerDataChangedLocally, this));
    }

    if (m_lobby_data.m_new_game) {
        m_players_lb_headers->SetText(4, UserString("MULTIPLAYER_PLAYER_LIST_STARTING_SPECIES"));
    } else {
        m_players_lb_headers->SetText(4, UserString("MULTIPLAYER_PLAYER_LIST_ORIGINAL_NAMES"));
    }

    // restore list scroll position
    GG::ListBox::iterator first_row_it = m_players_lb->FirstRowShown();
    int first_row_to_show = std::max<int>(0, std::min<int>(initial_list_scroll_pos,
                                                           m_players_lb->NumRows() - 1));
    std::advance(first_row_it, first_row_to_show);
    m_players_lb->SetFirstRowShown(first_row_it);

    // set ready button text according of situaton
    m_ready_bn->Disable(false);
    if (is_client_ready)
        m_ready_bn->SetText(UserString("NOT_READY_BN"));
    else if (is_other_ready)
        m_ready_bn->SetText(UserString("START_GAME_BN"));
    else
        m_ready_bn->SetText(UserString("READY_BN"));

    // set ready button state
    if (m_lobby_data.m_start_locked) {
        m_ready_bn->Disable(true);
        m_start_conditions_text->SetText(UserString(m_lobby_data.m_start_lock_cause));
    } else {
        m_ready_bn->Disable(false);
        m_start_conditions_text->SetText(UserString("MULTIPLAYER_GAME_START_CONDITIONS"));
    }

    Refresh();

    // This PreRender forces an 'extra' prerender to make the nested
    // dropdown lists to prerender after their rows have
    // prerendered.  Ideally, Layout would deal with the recursion on its
    // own.
    GG::GUI::PreRenderWindow(m_players_lb);

    return send_update_back_retval;
}

void MultiPlayerLobbyWnd::SendUpdate() {
    if (HumanClientApp::GetApp()->PlayerID() != Networking::INVALID_PLAYER_ID)
        HumanClientApp::GetApp()->Networking().SendMessage(LobbyUpdateMessage(m_lobby_data));
}

bool MultiPlayerLobbyWnd::PlayerDataAcceptable() const {
    std::set<std::string> empire_names;
    std::set<unsigned int> empire_colors;
    int num_players_excluding_observers(0);

    for (auto& row : *m_players_lb) {
        const PlayerRow& prow = dynamic_cast<const PlayerRow&>(*row);
        if (prow.m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            prow.m_player_data.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
        {
            num_players_excluding_observers++;

            // all non-observers must have unique non-blank names and colours

            if (prow.m_player_data.m_empire_name.empty())
                return false;

            empire_names.insert(prow.m_player_data.m_empire_name);

            unsigned int color_as_uint =
                prow.m_player_data.m_empire_color.r << 24 |
                prow.m_player_data.m_empire_color.g << 16 |
                prow.m_player_data.m_empire_color.b << 8 |
                prow.m_player_data.m_empire_color.a;
            empire_colors.insert(color_as_uint);
        } else if (prow.m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                   prow.m_player_data.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        {
            // do nothing special for this player
        } else {
            if (dynamic_cast<const EmptyPlayerRow*>(row.get())) {
                // ignore empty player row
            } else {
                ErrorLogger() << "MultiPlayerLobbyWnd::PlayerDataAcceptable found not empty player row with unrecognized client type?!";
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

bool MultiPlayerLobbyWnd::HasAuthRole(Networking::RoleType role) const
{ return ClientApp::GetApp()->Networking().HasAuthRole(role); }

void MultiPlayerLobbyWnd::ReadyClicked() {
    for (std::pair<int, PlayerSetupData>& entry : m_lobby_data.m_players) {
        if (entry.first == HumanClientApp::GetApp()->PlayerID()) {
            entry.second.m_player_ready = (! entry.second.m_player_ready);
        }
    }

    PopulatePlayerList();
    m_ready_bn->Disable(true);
    SendUpdate();
}

void MultiPlayerLobbyWnd::CancelClicked()
{ HumanClientApp::GetApp()->CancelMultiplayerGameFromLobby(); }

void MultiPlayerLobbyWnd::AnyCanEdit(bool checked) {
    if (HasAuthRole(Networking::ROLE_HOST)) {
        m_lobby_data.m_any_can_edit = m_any_can_edit->Checked();
        SendUpdate();
    }
}
