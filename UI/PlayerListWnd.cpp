#include "PlayerListWnd.h"

#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Message.h"
#include "../network/ClientNetworking.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Enums.h"

#include <GG/DrawUtil.h>

#include <algorithm>

namespace {
    const int           DATA_PANEL_BORDER = 1;

    std::shared_ptr<GG::Texture> AIIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ai.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> HumanIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "human.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> ObserverIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "observer.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> ModeratorIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> HostIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "host.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> PlayingIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "not_ready.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> WaitingIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ready.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> WarIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "war.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> PeaceIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "peace.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> AlliedIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "allied.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> UnknownIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "unknown.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> ShipIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "sitrep" / "fleet_arrived.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> ProductionIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "industry.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> ResearchIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "research.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> DetectionIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "detection.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> WonIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "sitrep" / "victory.png");
        return retval;
    }
    std::shared_ptr<GG::Texture> LostIcon() {
        static std::shared_ptr<GG::Texture> retval = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "sitrep" / "empire_eliminated.png");
        return retval;
    }


    ////////////////////////////////////////////////
    // PlayerDataPanel
    ////////////////////////////////////////////////
    /** Represents a player.  This class is used as the sole Control in
      * each PlayerRow. */
    class PlayerDataPanel : public GG::Control {
    public:
        PlayerDataPanel(GG::X w, GG::Y h, int player_id, int empire_id) :
            Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
            m_player_id(player_id),
            m_empire_id(empire_id),
            //m_player_name_text(nullptr),
            m_empire_name_text(nullptr),
            m_empire_ship_text(nullptr),
            m_empire_planet_text(nullptr),
            m_empire_production_text(nullptr),
            m_empire_research_text(nullptr),
            m_empire_detection_text(nullptr),
            m_diplo_status(INVALID_DIPLOMATIC_STATUS),
            m_player_status(Message::PLAYING_TURN),
            m_player_type(Networking::INVALID_CLIENT_TYPE),
            m_host(false),
            m_win_status(NEITHER),
            m_selected(false)
        {}

        void CompleteConstruction() override {
            GG::Control::CompleteConstruction();

            SetChildClippingMode(ClipToClient);

            //m_player_name_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);
            m_empire_name_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);
            m_empire_ship_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);
            m_empire_planet_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);
            m_empire_production_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);
            m_empire_research_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);
            m_empire_detection_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);

            //AttachChild(m_player_name_text);
            AttachChild(m_empire_name_text);
            AttachChild(m_empire_ship_text);
            AttachChild(m_empire_planet_text);
            AttachChild(m_empire_production_text);
            AttachChild(m_empire_research_text);
            AttachChild(m_empire_detection_text);

            DoLayout();
            Update();
        }

        /** Excludes border from the client area. */
        GG::Pt ClientUpperLeft() const override
        { return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

        /** Excludes border from the client area. */
        GG::Pt ClientLowerRight() const override
        { return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

        /** Renders panel background, border with color depending on the current state. */
        void Render() override {
            const GG::Clr& background_colour = ClientUI::WndColor();
            const GG::Clr& unselected_colour = ClientUI::WndOuterBorderColor();
            const GG::Clr& selected_colour = ClientUI::WndInnerBorderColor();
            GG::Clr border_colour = m_selected ? selected_colour : unselected_colour;
            if (Disabled())
                border_colour = DisabledColor(border_colour);

            GG::FlatRectangle(UpperLeft(), LowerRight(), background_colour, border_colour, DATA_PANEL_BORDER);

            glColor(GG::CLR_WHITE);

            GG::Pt ICON_SIZE = GG::Pt(GG::X(IconSize()), GG::Y(IconSize()));


            ShipIcon()->OrthoBlit(UpperLeft() + m_ship_icon_ul, UpperLeft() + m_ship_icon_ul+ ICON_SIZE);

            //ClientUI::PlanetIcon(favored_planet_type)->OrthoBlit(UpperLeft() + m_research_icon_ul, UpperLeft() + m_research_icon_ul + ICON_SIZE);
            ClientUI::PlanetIcon(PT_TERRAN)->OrthoBlit(UpperLeft() + m_planet_icon_ul , UpperLeft() + m_planet_icon_ul + ICON_SIZE);

            ProductionIcon()->OrthoBlit(UpperLeft() + m_production_icon_ul, UpperLeft() + m_production_icon_ul + ICON_SIZE);
            ResearchIcon()->OrthoBlit(UpperLeft() + m_research_icon_ul, UpperLeft() + m_research_icon_ul + ICON_SIZE);
            DetectionIcon()->OrthoBlit(UpperLeft() + m_detection_icon_ul, UpperLeft() + m_detection_icon_ul + ICON_SIZE);

            const ClientApp* app = ClientApp::GetApp();
            if (!app) {
                ErrorLogger() << "PlayerDataPanel::Render couldn't get client app!";
                return;
            }
            if (m_player_id != app->PlayerID()) {
                // render diplomacy icon
                switch (m_diplo_status) {
                case DIPLO_WAR:                 WarIcon()->OrthoBlit(    UpperLeft() + m_diplo_status_icon_ul, UpperLeft() + m_diplo_status_icon_ul + ICON_SIZE); break;
                case DIPLO_PEACE:               PeaceIcon()->OrthoBlit(  UpperLeft() + m_diplo_status_icon_ul, UpperLeft() + m_diplo_status_icon_ul + ICON_SIZE); break;
                case DIPLO_ALLIED:              AlliedIcon()->OrthoBlit( UpperLeft() + m_diplo_status_icon_ul, UpperLeft() + m_diplo_status_icon_ul + ICON_SIZE); break;
                case INVALID_DIPLOMATIC_STATUS: UnknownIcon()->OrthoBlit(UpperLeft() + m_diplo_status_icon_ul, UpperLeft() + m_diplo_status_icon_ul + ICON_SIZE); break;
                default:    break;
                }
            }

            // render player status icon
            switch (m_player_status) {
            case Message::PLAYING_TURN:     PlayingIcon()->OrthoBlit(UpperLeft() + m_player_status_icon_ul, UpperLeft() + m_player_status_icon_ul + ICON_SIZE); break;
            case Message::WAITING:          WaitingIcon()->OrthoBlit(UpperLeft() + m_player_status_icon_ul, UpperLeft() + m_player_status_icon_ul + ICON_SIZE); break;
            default:    break;
            }

            // render player type icon
            switch (m_player_type) {
            case Networking::CLIENT_TYPE_HUMAN_PLAYER:      HumanIcon()->OrthoBlit(    UpperLeft() + m_player_type_icon_ul, UpperLeft() + m_player_type_icon_ul + ICON_SIZE); break;
            case Networking::CLIENT_TYPE_AI_PLAYER:         AIIcon()->OrthoBlit(       UpperLeft() + m_player_type_icon_ul, UpperLeft() + m_player_type_icon_ul + ICON_SIZE); break;
            case Networking::CLIENT_TYPE_HUMAN_OBSERVER:    ObserverIcon()->OrthoBlit( UpperLeft() + m_player_type_icon_ul, UpperLeft() + m_player_type_icon_ul + ICON_SIZE); break;
            case Networking::CLIENT_TYPE_HUMAN_MODERATOR:   ModeratorIcon()->OrthoBlit(UpperLeft() + m_player_type_icon_ul, UpperLeft() + m_player_type_icon_ul + ICON_SIZE); break;
            default:    break;
            }

            if (m_host)
                HostIcon()->OrthoBlit(UpperLeft() + m_host_icon_ul, UpperLeft() + m_host_icon_ul + ICON_SIZE);


            switch (m_win_status)
            {
            case WON:     WonIcon()->OrthoBlit(UpperLeft() + m_win_status_icon_ul, UpperLeft() + m_win_status_icon_ul + ICON_SIZE); break;
            case LOST:    LostIcon()->OrthoBlit(UpperLeft() + m_win_status_icon_ul, UpperLeft() + m_win_status_icon_ul + ICON_SIZE); break;
            case NEITHER: break;
            }
        }

        void            Select(bool b)
        { m_selected = b; }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            const GG::Pt old_size = Size();
            GG::Control::SizeMove(ul, lr);
            if (old_size != Size())
                DoLayout();
        }

        void            Update() {
            const ClientApp* app = ClientApp::GetApp();
            if (!app) {
                ErrorLogger() << "PlayerDataPanel::Update couldn't get client app!";
                return;
            }

            // empire name
            std::string empire_name;
            const std::map<int, PlayerInfo>& players = app->Players();

            auto player_it = players.find(m_player_id);
            if (player_it != players.end()) {
                const PlayerInfo& player_info = player_it->second;

                m_player_type = player_info.client_type;
                m_host = player_info.host;
                empire_name = player_info.name;
            } else {
                m_player_type = Networking::INVALID_CLIENT_TYPE;
                m_host = false;
            }

            // if player has an empire, get its name and colour.  (Some player types might not have empires...)
            GG::Clr empire_color = ClientUI::TextColor();
            const Empire* empire = GetEmpire(m_empire_id);
            if (empire) {
                empire_color = empire->Color();
                // ignore player name
                empire_name = empire->Name();
                if (m_empire_id == ALL_EMPIRES || m_empire_id == app->EmpireID())
                    m_diplo_status = INVALID_DIPLOMATIC_STATUS;
                else
                    m_diplo_status = Empires().GetDiplomaticStatus(m_empire_id, app->EmpireID());
                if (empire->Won())
                    m_win_status = WON; // even if you later get eliminated, you still won
                else if (empire->Eliminated())
                    m_win_status = LOST;
                else
                    m_win_status = NEITHER;
            }

            //m_player_name_text->SetTextColor(empire_color);
            //m_player_name_text->SetText(player_info.name);

            m_empire_name_text->SetTextColor(empire_color);
            m_empire_name_text->SetText(empire_name);

            const ObjectMap& objects = Objects();
            double empires_ship_count = 0.0;
            double empires_planet_count = 0.0;
            double empires_production_points = 0.0;
            double empires_research_points = 0.0;

            const std::set<int>& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(HumanClientApp::GetApp()->EmpireID());
            const std::set<int>& this_client_stale_object_info       = GetUniverse().EmpireStaleKnowledgeObjectIDs(HumanClientApp::GetApp()->EmpireID());

            for (auto& ship : objects.FindObjects<Ship>()) {
                if (empire) {
                    if (ship->Owner() == empire->EmpireID()
                        && !this_client_known_destroyed_objects.count(ship->ID())
                        && !this_client_stale_object_info.count(ship->ID())) {
                            empires_ship_count += 1;
                    }
                }
            }

            for (auto& planet : objects.FindObjects<Planet>()) {
                if (empire) {
                    if (planet->Owner() == empire->EmpireID()) {
                        empires_planet_count      += 1;
                        empires_production_points += planet->InitialMeterValue(METER_INDUSTRY);
                        empires_research_points   += planet->InitialMeterValue(METER_RESEARCH);
                    }
                }
            }

            std::string ship_text;
            std::string planet_text;
            std::string production_text;
            std::string research_text;
            std::string detection_text;

            if (empires_ship_count == 0.0) {
                ship_text       = UserString("NOTHING_VALUE_SYMBOL");
            } else {
                ship_text       = DoubleToString(empires_ship_count, 2, false);
            }

            if (empires_planet_count == 0.0) {
                planet_text     = UserString("NOTHING_VALUE_SYMBOL");
            } else {
                planet_text     = DoubleToString(empires_planet_count, 2, false);
            }

            if (empires_production_points == 0.0) {
                production_text = UserString("NOTHING_VALUE_SYMBOL");
            } else {
                production_text = DoubleToString(empires_production_points, 2, false);
            }

            if (empires_research_points == 0.0) {
                research_text   = UserString("NOTHING_VALUE_SYMBOL");
            } else {
                research_text   = DoubleToString(empires_research_points, 2, false);
            }

            detection_text = DoubleToString(empire ? empire->GetMeter("METER_DETECTION_STRENGTH")->Current() : 0.0, 0, false);

            m_empire_ship_text->SetText(ship_text);
            m_empire_planet_text->SetText(planet_text);
            m_empire_production_text->SetText(production_text);
            m_empire_research_text->SetText(research_text);
            m_empire_detection_text->SetText(detection_text);


        }

        void            SetStatus(Message::PlayerStatus player_status)
        { m_player_status = player_status; }

    private:
        int             IconSize() const   { return Value(Height()) - 2; }

        void            DoLayout() {
            const GG::X PLAYER_NAME_WIDTH(ClientUI::Pts()       * 10);
            const GG::X EMPIRE_NAME_WIDTH(ClientUI::Pts()       * 10);
            const GG::X EMPIRE_SHIP_WIDTH(ClientUI::Pts()       * 16/5);
            const GG::X EMPIRE_PLANET_WIDTH(ClientUI::Pts()     * 16/5);
            const GG::X EMPIRE_PRODUCTION_WIDTH(ClientUI::Pts() * 16/5);
            const GG::X EMPIRE_RESEARCH_WIDTH(ClientUI::Pts()   * 16/5);
            const GG::X EMPIRE_DETECTION_WIDTH(ClientUI::Pts()  * 16/5);


            GG::X left(DATA_PANEL_BORDER);
            GG::Y top(DATA_PANEL_BORDER);
            GG::Y bottom(ClientHeight());
            GG::X PAD(3);

            m_diplo_status_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            //m_player_name_text->SizeMove(GG::Pt(left, top), GG::Pt(left + PLAYER_NAME_WIDTH, bottom));
            //left += PLAYER_NAME_WIDTH;

            m_empire_name_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_NAME_WIDTH, bottom));
            left += EMPIRE_NAME_WIDTH;

            m_ship_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_empire_ship_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_SHIP_WIDTH, bottom));
            left += EMPIRE_SHIP_WIDTH;

            m_planet_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_empire_planet_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_PLANET_WIDTH, bottom));
            left += EMPIRE_PLANET_WIDTH;

            m_production_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_empire_production_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_PRODUCTION_WIDTH, bottom));
            left += EMPIRE_PRODUCTION_WIDTH;

            m_research_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_empire_research_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_RESEARCH_WIDTH, bottom));
            left += EMPIRE_RESEARCH_WIDTH;

            m_detection_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_empire_detection_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_DETECTION_WIDTH, bottom));
            left += EMPIRE_DETECTION_WIDTH;

            m_player_status_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_player_type_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_host_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_win_status_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;
        }

        int                     m_player_id;
        int                     m_empire_id;
        //std::shared_ptr<GG::Label>            m_player_name_text;
        std::shared_ptr<GG::Label>              m_empire_name_text;
        std::shared_ptr<GG::Label>              m_empire_ship_text;
        std::shared_ptr<GG::Label>              m_empire_planet_text;
        std::shared_ptr<GG::Label>              m_empire_production_text;
        std::shared_ptr<GG::Label>              m_empire_research_text;
        std::shared_ptr<GG::Label>              m_empire_detection_text;

        GG::Pt                  m_diplo_status_icon_ul;
        GG::Pt                  m_ship_icon_ul;
        GG::Pt                  m_planet_icon_ul;
        GG::Pt                  m_production_icon_ul;
        GG::Pt                  m_research_icon_ul;
        GG::Pt                  m_detection_icon_ul;
        GG::Pt                  m_player_status_icon_ul;
        GG::Pt                  m_player_type_icon_ul;
        GG::Pt                  m_host_icon_ul;
        GG::Pt                  m_win_status_icon_ul;

        DiplomaticStatus        m_diplo_status;
        Message::PlayerStatus   m_player_status;
        Networking::ClientType  m_player_type;
        bool                    m_host;
        enum {
            WON,
            LOST,
            NEITHER
        }                       m_win_status;

        bool                    m_selected;
    };


    ////////////////////////////////////////////////
    // PlayerRow
    ////////////////////////////////////////////////
    class PlayerRow : public GG::ListBox::Row {
    public:
        PlayerRow(GG::X w, GG::Y h, int player_id, int empire_id) :
            GG::ListBox::Row(w, h, "", GG::ALIGN_NONE, 0),
            m_player_id(player_id),
            m_empire_id(empire_id),
            m_panel(nullptr)
        {
            SetName("PlayerRow");
            SetChildClippingMode(ClipToClient);
        }

        void CompleteConstruction() override {

            GG::ListBox::Row::CompleteConstruction();
            m_panel = GG::Wnd::Create<PlayerDataPanel>(Width(), Height(), m_player_id, m_empire_id);
            push_back(m_panel);
        }

        int     PlayerID() const {
            return m_player_id;
        }

        int     EmpireID() const {
            return m_empire_id;
        }

        void    Update() {
            if (m_panel)
                m_panel->Update();
        }

        void    SetStatus(Message::PlayerStatus player_status) {
            if (m_panel)
                m_panel->SetStatus(player_status);
        }

        /** This function overridden because otherwise, rows don't expand
          * larger than their initial size when resizing the list. */
        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "PlayerRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

    private:
        int                 m_player_id;
        int                 m_empire_id;
        std::shared_ptr<PlayerDataPanel>    m_panel;
    };
}

////////////////////////////////////////////////
// PlayerListBox
////////////////////////////////////////////////
class PlayerListBox : public CUIListBox {
public:
    PlayerListBox(void) :
        CUIListBox()
    {
        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();
    }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "PlayerListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "PlayerListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (auto& row : *this)
                row->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight()); }

    static GG::Y    ListRowHeight()
    { return GG::Y(ClientUI::Pts() * 3/2); }
};


/////////////////////
//  PlayerListWnd  //
/////////////////////
PlayerListWnd::PlayerListWnd(const std::string& config_name) :
    CUIWnd(UserString("PLAYERS_LIST_PANEL_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | GG::RESIZABLE | CLOSABLE | PINABLE,
           config_name),
    m_player_list(nullptr)
{}

void PlayerListWnd::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    m_player_list = GG::Wnd::Create<PlayerListBox>();
    m_player_list->SetHiliteColor(GG::CLR_ZERO);
    m_player_list->SetStyle(GG::LIST_NOSORT);
    m_player_list->SelRowsChangedSignal.connect(
        boost::bind(&PlayerListWnd::PlayerSelectionChanged, this, _1));
    m_player_list->DoubleClickedRowSignal.connect(
        boost::bind(&PlayerListWnd::PlayerDoubleClicked, this, _1, _2, _3));
    m_player_list->RightClickedRowSignal.connect(
        boost::bind(&PlayerListWnd::PlayerRightClicked, this, _1, _2, _3));
    AttachChild(m_player_list);

    Empires().DiplomaticStatusChangedSignal.connect(
        boost::bind(&PlayerListWnd::Update, this));
    Empires().DiplomaticMessageChangedSignal.connect(
        boost::bind(&PlayerListWnd::Update, this));
    DoLayout();

    Refresh();
}

std::set<int> PlayerListWnd::SelectedPlayerIDs() const {
    std::set<int> retval;
    for (auto it = m_player_list->begin(); it != m_player_list->end(); ++it) {
        if (!m_player_list->Selected(it))
            continue;

        int selected_player_id = PlayerInRow(it);
        if (selected_player_id != Networking::INVALID_PLAYER_ID)
            retval.insert(selected_player_id);
    }
    return retval;
}

void PlayerListWnd::HandleEmpireStatusUpdate(Message::PlayerStatus player_status, int about_empire_id) {
    for (auto& row : *m_player_list) {
        if (PlayerRow* player_row = dynamic_cast<PlayerRow*>(row.get())) {
            if (about_empire_id == ALL_EMPIRES) {
                WarnLogger() << "PlayerListWnd::HandleEmpireStatusUpdate got no empire";
            } else if (player_row->EmpireID() == about_empire_id) {
                player_row->SetStatus(player_status);
                return;
            }
        }
    }
}

void PlayerListWnd::Update() {
    for (auto& row : *m_player_list) {
        if (PlayerRow* player_row = dynamic_cast<PlayerRow*>(row.get()))
            player_row->Update();
    }
}

void PlayerListWnd::Refresh() {
    std::set<int> initially_selected_players = this->SelectedPlayerIDs();

    m_player_list->Clear();

    const ClientApp* app = ClientApp::GetApp();
    if (!app) {
        ErrorLogger() << "PlayerListWnd::Refresh couldn't get client app!";
        return;
    }

    const GG::Pt row_size = m_player_list->ListRowSize();

    // first fill empires
    for (const auto& empire : Empires()) {
        int player_id = app->EmpirePlayerID(empire.first);
        auto player_row = GG::Wnd::Create<PlayerRow>(row_size.x, row_size.y, player_id, empire.first);
        m_player_list->Insert(player_row);
        player_row->Resize(row_size);
    }

    // second fill players without empires
    const std::map<int, PlayerInfo>& players = app->Players();

    for (const auto& player : players) {
        if (player.second.empire_id == ALL_EMPIRES) {
            int player_id = player.first;
            auto player_row = GG::Wnd::Create<PlayerRow>(row_size.x, row_size.y, player_id, ALL_EMPIRES);
            m_player_list->Insert(player_row);
            player_row->Resize(row_size);
        }
    }

    this->SetSelectedPlayers(initially_selected_players);
}

void PlayerListWnd::SetSelectedPlayers(const std::set<int>& player_ids) {
    const auto initial_selections = m_player_list->Selections();

    m_player_list->DeselectAll();

    // early exit if nothing to select
    if (player_ids.empty()) {
        PlayerSelectionChanged(m_player_list->Selections());
        return;
    }

    // loop through players, selecting any indicated
    for (auto it = m_player_list->begin(); it != m_player_list->end(); ++it) {
        PlayerRow* row = dynamic_cast<PlayerRow*>(it->get());
        if (!row) {
            ErrorLogger() << "PlayerRow::SetSelectedPlayers couldn't cast a listbow row to PlayerRow?";
            continue;
        }

        // if this row's player should be selected, so so
        if (player_ids.count(row->PlayerID())) {
            m_player_list->SelectRow(it);
            m_player_list->BringRowIntoView(it);  // may cause earlier rows brought into view to be brought out of view... oh well
        }
    }

    if (initial_selections != m_player_list->Selections())
        PlayerSelectionChanged(m_player_list->Selections());
}

void PlayerListWnd::Clear() {
    m_player_list->Clear();
}

void PlayerListWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void PlayerListWnd::DoLayout()
{
    if (m_player_list)
        m_player_list->SizeMove(GG::Pt(), GG::Pt(ClientWidth(), ClientHeight() - GG::Y(INNER_BORDER_ANGLE_OFFSET)));
}

void PlayerListWnd::CloseClicked()
{ ClosingSignal(); }

void PlayerListWnd::PlayerSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    // mark as selected all PlayerDataPanel that are in \a rows and mark as not
    // selected all PlayerDataPanel that aren't in \a rows
    for (auto it = m_player_list->begin(); it != m_player_list->end(); ++it) {
        auto& row = *it;
        if (!row) {
            ErrorLogger() << "PlayerListWnd::PlayerSelectionChanged couldn't get row";
            continue;
        }
        if (row->empty()) {
            ErrorLogger() << "PlayerListWnd::PlayerSelectionChanged got empty row";
            continue;
        }
        GG::Control* control = !row->empty() ? row->at(0) : nullptr;
        if (!control) {
            ErrorLogger() << "PlayerListWnd::PlayerSelectionChanged couldn't get control from row";
            continue;
        }
        PlayerDataPanel* data_panel = dynamic_cast<PlayerDataPanel*>(control);
        if (!data_panel) {
            ErrorLogger() << "PlayerListWnd::PlayerSelectionChanged couldn't get PlayerDataPanel from control";
            continue;
        }
        data_panel->Select(rows.count(it));
    }

    SelectedPlayersChangedSignal();
}

void PlayerListWnd::PlayerDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    int player_id = PlayerInRow(it);
    if (player_id != Networking::INVALID_PLAYER_ID)
        PlayerDoubleClickedSignal(player_id);
}

namespace {
    std::function<void()> MakeSendDiplomaticAction(
        const int client_empire_id, const int clicked_empire_id,
        const std::function<DiplomaticMessage(int, int)>& message)
    {
        auto premade_message = DiplomacyMessage(message(client_empire_id, clicked_empire_id));
        auto &networking = HumanClientApp::GetApp()->Networking();
        return std::bind(&ClientNetworking::SendMessage, &networking, premade_message);
    }

}

void PlayerListWnd::PlayerRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    // check that a valid player was clicked and that it wasn't this client's own player
    int clicked_empire_id = EmpireInRow(it);
    if (clicked_empire_id == ALL_EMPIRES)
        return;
    const ClientApp* app = ClientApp::GetApp();
    if (!app) {
        ErrorLogger() << "PlayerListWnd::PlayerRightClicked couldn't get client app!";
        return;
    }
    int client_player_id = app->PlayerID();
    if (client_player_id == Networking::INVALID_PLAYER_ID)
        return;
    int client_empire_id = app->EmpireID();

    if (!GetEmpire(clicked_empire_id)) {
        ErrorLogger() << "PlayerListWnd::PlayerRightClicked tried to look up empire id "
                      << clicked_empire_id
                      << " but couldn't find such an empire";
        return;
    }

    // Actions
    auto war_declaration_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, WarDeclarationDiplomaticMessage);
    auto peace_proposal_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, PeaceProposalDiplomaticMessage);
    auto peace_accept_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, AcceptPeaceDiplomaticMessage);
    auto allies_proposal_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, AlliesProposalDiplomaticMessage);
    auto allies_accept_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, AcceptAlliesDiplomaticMessage);
    auto end_alliance_declaration_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, EndAllianceDiplomaticMessage);
    auto proposal_cancel_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, CancelDiplomaticMessage);
    auto proposal_reject_action = MakeSendDiplomaticAction(
        client_empire_id, clicked_empire_id, RejectProposalDiplomaticMessage);
    auto pedia_lookup_action = [clicked_empire_id]() { ClientUI::GetClientUI()->ZoomToEmpire(clicked_empire_id); };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    if (app->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER &&
        client_empire_id != ALL_EMPIRES &&
        clicked_empire_id != ALL_EMPIRES)
    {
        // get diplomatic status between client and clicked empires
        DiplomaticStatus diplo_status = Empires().GetDiplomaticStatus(clicked_empire_id, client_empire_id);
        if (diplo_status == INVALID_DIPLOMATIC_STATUS && clicked_empire_id != client_empire_id) {
            ErrorLogger() << "PlayerListWnd::PlayerRightClicked found invalid diplomatic status between client and clicked empires.";
            return;
        }
        const DiplomaticMessage& existing_message_from_clicked_empire_to_this_player =
            Empires().GetDiplomaticMessage(clicked_empire_id, client_empire_id);
        const DiplomaticMessage& existing_message_from_this_player_to_clicked_empire =
            Empires().GetDiplomaticMessage(client_empire_id, clicked_empire_id);

        // decide what diplomatic commands to show in popup menu
        bool show_peace_propose = false;
        bool show_peace_cancel = false;
        bool show_peace_reject = false;
        bool show_peace_accept = false;
        bool show_allies_end = false;
        bool show_allies_propose = false;
        bool show_allies_cancel = false;
        bool show_allies_reject = false;
        bool show_allies_accept = false;
        bool show_declare_war = false;

        if (existing_message_from_this_player_to_clicked_empire.GetType() == DiplomaticMessage::PEACE_PROPOSAL)
            show_peace_cancel = true;
        else if (existing_message_from_this_player_to_clicked_empire.GetType() == DiplomaticMessage::ALLIES_PROPOSAL)
            show_allies_cancel = true;

        if (existing_message_from_clicked_empire_to_this_player.GetType() == DiplomaticMessage::PEACE_PROPOSAL) {
            show_peace_accept = true;
            show_peace_reject = true;
        } else if (existing_message_from_clicked_empire_to_this_player.GetType() == DiplomaticMessage::ALLIES_PROPOSAL) {
            show_allies_accept = true;
            show_allies_reject = true;
        }

        if (diplo_status == DIPLO_WAR) {
            if (!show_peace_accept)
                show_peace_propose = true;

        } else if (diplo_status == DIPLO_PEACE) {
            if (!show_allies_accept)
                show_allies_propose = true;
            show_declare_war = true;

        } else if (diplo_status == DIPLO_ALLIED) {
            show_allies_end = true;
        }


        if (show_peace_propose)
            popup->AddMenuItem(GG::MenuItem(UserString("PEACE_PROPOSAL"),           false, false, peace_proposal_action));
        if (show_peace_cancel)
            popup->AddMenuItem(GG::MenuItem(UserString("PEACE_PROPOSAL_CANCEL"),    false, false, proposal_cancel_action));
        if (show_peace_accept)
            popup->AddMenuItem(GG::MenuItem(UserString("PEACE_ACCEPT"),             false, false, peace_accept_action));
        if (show_peace_reject)
            popup->AddMenuItem(GG::MenuItem(UserString("PEACE_REJECT"),             false, false, proposal_reject_action));
        if (show_allies_end)
            popup->AddMenuItem(GG::MenuItem(UserString("END_ALLIANCE_DECLARATION"), false, false, end_alliance_declaration_action));
        if (show_allies_propose)
            popup->AddMenuItem(GG::MenuItem(UserString("ALLIES_PROPOSAL"),          false, false, allies_proposal_action));
        if (show_allies_accept)
            popup->AddMenuItem(GG::MenuItem(UserString("ALLIES_ACCEPT"),            false, false, allies_accept_action));
        if (show_allies_cancel)
            popup->AddMenuItem(GG::MenuItem(UserString("ALLIES_PROPOSAL_CANCEL"),   false, false, proposal_cancel_action));
        if (show_allies_reject)
            popup->AddMenuItem(GG::MenuItem(UserString("ALLIES_REJECT"),            false, false, proposal_reject_action));
        if (show_declare_war)
            popup->AddMenuItem(GG::MenuItem(UserString("WAR_DECLARATION"),          false, false, war_declaration_action));
    }

    popup->AddMenuItem(GG::MenuItem(str(FlexibleFormat(UserString("ENC_LOOKUP")) % GetEmpire(clicked_empire_id)->Name()), false, false, pedia_lookup_action));

    popup->Run();
}

int PlayerListWnd::PlayerInRow(GG::ListBox::iterator it) const {
    if (it == m_player_list->end())
        return Networking::INVALID_PLAYER_ID;

    if (PlayerRow* player_row = dynamic_cast<PlayerRow*>(it->get()))
        return player_row->PlayerID();

    return Networking::INVALID_PLAYER_ID;
}

int PlayerListWnd::EmpireInRow(GG::ListBox::iterator it) const {
    if (it == m_player_list->end())
        return ALL_EMPIRES;

    if (PlayerRow* player_row = dynamic_cast<PlayerRow*>(it->get()))
        return player_row->EmpireID();

    return ALL_EMPIRES;
}
