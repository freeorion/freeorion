#include "PlayerListWnd.h"

#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Message.h"
#include "../util/OptionsDB.h"
#include "../util/MultiplayerCommon.h"

#include <GG/DrawUtil.h>

#include <algorithm>

namespace {
    const int           DATA_PANEL_BORDER = 1;

    boost::shared_ptr<GG::Texture> AIIcon()         { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ai.png"); }
    boost::shared_ptr<GG::Texture> HumanIcon()      { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "human.png"); }
    boost::shared_ptr<GG::Texture> ObserverIcon()   { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "observer.png"); }
    boost::shared_ptr<GG::Texture> HostIcon()       { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "host.png"); }
    boost::shared_ptr<GG::Texture> PlayingIcon()    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "playing.png"); }
    boost::shared_ptr<GG::Texture> WaitingIcon()    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "waiting.png"); }
    boost::shared_ptr<GG::Texture> CombatIcon()     { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "combat.png"); }
    boost::shared_ptr<GG::Texture> WarIcon()        { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "war.png"); }
    boost::shared_ptr<GG::Texture> PeaceIcon()      { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "peace.png"); }
    boost::shared_ptr<GG::Texture> UnknownIcon()    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "unknown.png"); }

    ////////////////////////////////////////////////
    // PlayerDataPanel
    ////////////////////////////////////////////////
    /** Represents a player.  This class is used as the sole Control in
      * each PlayerRow. */
    class PlayerDataPanel : public GG::Control {
    public:
        PlayerDataPanel(GG::X w, GG::Y h, int player_id) :
            Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
            m_player_id(player_id),
            m_player_name_text(0),
            m_empire_name_text(0),
            m_diplo_status(INVALID_DIPLOMATIC_STATUS),
            m_player_status(Message::WAITING),
            m_player_type(Networking::INVALID_CLIENT_TYPE),
            m_host(false),
            m_selected(false)
        {
            SetChildClippingMode(ClipToClient);

            m_player_name_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, h, "",
                                                     ClientUI::GetFont(), ClientUI::TextColor(),
                                                     GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_player_name_text);

            m_empire_name_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, h, "",
                                                     ClientUI::GetFont(), ClientUI::TextColor(),
                                                     GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_empire_name_text);

            DoLayout();
            Update();
        }

        /** Excludes border from the client area. */
        virtual GG::Pt  ClientUpperLeft() const
        { return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

        /** Excludes border from the client area. */
        virtual GG::Pt  ClientLowerRight() const
        { return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

        /** Renders panel background, border with color depending on the current state. */
        virtual void    Render() {
            const GG::Clr& background_colour = ClientUI::WndColor();
            const GG::Clr& unselected_colour = ClientUI::WndOuterBorderColor();
            const GG::Clr& selected_colour = ClientUI::WndInnerBorderColor();
            GG::Clr border_colour = m_selected ? selected_colour : unselected_colour;
            if (Disabled())
                border_colour = DisabledColor(border_colour);

            GG::FlatRectangle(UpperLeft(), LowerRight(), background_colour,  border_colour, DATA_PANEL_BORDER);

            glColor(GG::CLR_WHITE);

            GG::Pt ICON_SIZE = GG::Pt(GG::X(IconSize()), GG::Y(IconSize()));

            const ClientApp* app = ClientApp::GetApp();
            if (!app) {
                Logger().errorStream() << "PlayerDataPanel::Render couldn't get client app!";
                return;
            }
            if (m_player_id != app->PlayerID()) {
                // render diplomacy icon
                switch (m_diplo_status) {
                case DIPLO_WAR:                 WarIcon()->OrthoBlit(    UpperLeft() + m_diplo_status_icon_ul, UpperLeft() + m_diplo_status_icon_ul + ICON_SIZE); break;
                case DIPLO_PEACE:               PeaceIcon()->OrthoBlit(  UpperLeft() + m_diplo_status_icon_ul, UpperLeft() + m_diplo_status_icon_ul + ICON_SIZE); break;
                case INVALID_DIPLOMATIC_STATUS: UnknownIcon()->OrthoBlit(UpperLeft() + m_diplo_status_icon_ul, UpperLeft() + m_diplo_status_icon_ul + ICON_SIZE); break;
                default:    break;
                }
            }

            // render player status icon
            switch (m_player_status) {
            case Message::PLAYING_TURN:     PlayingIcon()->OrthoBlit(UpperLeft() + m_player_status_icon_ul, UpperLeft() + m_player_status_icon_ul + ICON_SIZE); break;
            case Message::RESOLVING_COMBAT: CombatIcon()->OrthoBlit( UpperLeft() + m_player_status_icon_ul, UpperLeft() + m_player_status_icon_ul + ICON_SIZE); break;
            case Message::WAITING:          WaitingIcon()->OrthoBlit(UpperLeft() + m_player_status_icon_ul, UpperLeft() + m_player_status_icon_ul + ICON_SIZE); break;
            default:    break;
            }

            // render player type icon
            switch (m_player_type) {
            case Networking::CLIENT_TYPE_HUMAN_PLAYER:  HumanIcon()->OrthoBlit(   UpperLeft() + m_player_type_icon_ul, UpperLeft() + m_player_type_icon_ul + ICON_SIZE); break;
            case Networking::CLIENT_TYPE_AI_PLAYER:     AIIcon()->OrthoBlit(      UpperLeft() + m_player_type_icon_ul, UpperLeft() + m_player_type_icon_ul + ICON_SIZE); break;
            case Networking::CLIENT_TYPE_HUMAN_OBSERVER:ObserverIcon()->OrthoBlit(UpperLeft() + m_player_type_icon_ul, UpperLeft() + m_player_type_icon_ul + ICON_SIZE); break;
            default:    break;
            }
 
            if (m_host)
                HostIcon()->OrthoBlit(UpperLeft() + m_host_icon_ul, UpperLeft() + m_host_icon_ul + ICON_SIZE);
        }

        void            Select(bool b) {
            if (m_selected != b)
                m_selected = b;
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::Control::SizeMove(ul, lr);
            //std::cout << "ShipDataPanel::SizeMove new size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (old_size != Size())
                DoLayout();
        }

        void            Update() {
            const ClientApp* app = ClientApp::GetApp();
            if (!app) {
                Logger().errorStream() << "PlayerDataPanel::Update couldn't get client app!";
                return;
            }

            const std::map<int, PlayerInfo>& players = app->Players();

            std::map<int, PlayerInfo>::const_iterator player_it = players.find(m_player_id);
            if (player_it == players.end()) {
                Logger().errorStream() << "PlayerDataPanel::Update couldn't find player with id " << m_player_id;
                return;
            }

            const PlayerInfo& player_info = player_it->second;

            // if player has an empire, get its name and colour.  (Some player types might not have empires...)
            GG::Clr empire_color = ClientUI::TextColor();
            std::string empire_name;
            const Empire* empire = Empires().Lookup(player_info.empire_id);
            if (empire) {
                empire_color = empire->Color();
                empire_name = empire->Name();
                m_diplo_status = Empires().GetDiplomaticStatus(player_info.empire_id, app->EmpireID());
            }

            m_player_name_text->SetTextColor(empire_color);
            m_player_name_text->SetText(player_info.name);

            m_empire_name_text->SetTextColor(empire_color);
            m_empire_name_text->SetText(empire_name);

            m_player_type = player_info.client_type;
            m_host = player_info.host;
        }

        void            SetStatus(Message::PlayerStatus player_status)
        { m_player_status = player_status; }

    private:
        int             IconSize() const   { return Value(Height()) - 2; }

        void            DoLayout() {
            const GG::X PLAYER_NAME_WIDTH(ClientUI::Pts() * 17/2);
            const GG::X EMPIRE_NAME_WIDTH(ClientUI::Pts() * 18/2);

            GG::X left(DATA_PANEL_BORDER);
            GG::Y top(DATA_PANEL_BORDER);
            GG::Y bottom(ClientHeight());
            GG::X PAD(3);

            m_diplo_status_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_player_name_text->SizeMove(GG::Pt(left, top), GG::Pt(left + PLAYER_NAME_WIDTH, bottom));
            left += PLAYER_NAME_WIDTH;

            m_empire_name_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_NAME_WIDTH, bottom));
            left += EMPIRE_NAME_WIDTH;

            m_player_status_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_player_type_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;

            m_host_icon_ul = GG::Pt(left, top);
            left += GG::X(IconSize()) + PAD;
        }

        int                     m_player_id;
        GG::TextControl*        m_player_name_text;
        GG::TextControl*        m_empire_name_text;

        GG::Pt                  m_diplo_status_icon_ul;
        GG::Pt                  m_player_status_icon_ul;
        GG::Pt                  m_player_type_icon_ul;
        GG::Pt                  m_host_icon_ul;

        DiplomaticStatus        m_diplo_status;
        Message::PlayerStatus   m_player_status;
        Networking::ClientType  m_player_type;
        bool                    m_host;

        bool                    m_selected;
    };


    ////////////////////////////////////////////////
    // PlayerRow
    ////////////////////////////////////////////////
    class PlayerRow : public GG::ListBox::Row {
    public:
        PlayerRow(GG::X w, GG::Y h, int player_id) :
            GG::ListBox::Row(w, h, "", GG::ALIGN_NONE, 0),
            m_player_id(player_id),
            m_panel(0)
        {
            SetName("PlayerRow");
            SetChildClippingMode(ClipToClient);
            SetDragDropDataType("PLAYER_ROW");
            m_panel = new PlayerDataPanel(w, h, m_player_id);
            push_back(m_panel);
        }

        int     PlayerID() const {
            return m_player_id;
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
        void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "PlayerRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

    private:
        int                 m_player_id;
        PlayerDataPanel*    m_panel;
    };
}

////////////////////////////////////////////////
// PlayerListBox
////////////////////////////////////////////////
class PlayerListBox : public CUIListBox {
public:
    PlayerListBox(GG::X x, GG::Y y, GG::X w, GG::Y h) :
        CUIListBox(x, y, w, h)
    {
        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();
    }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "PlayerListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "PlayerListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (GG::ListBox::iterator it = begin(); it != end(); ++it)
                (*it)->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const {
        return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight());
    }

    static GG::Y    ListRowHeight() {
        return GG::Y(ClientUI::Pts() * 3/2);
    }
};


/////////////////////
//  PlayerListWnd  //
/////////////////////
PlayerListWnd::PlayerListWnd(GG::X x, GG::Y y, GG::X w, GG::Y h) :
    CUIWnd(UserString("PLAYERS_LIST_PANEL_TITLE"), x, y, w, h, GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | GG::RESIZABLE | CLOSABLE),
    m_player_list(0)
{
    m_player_list = new PlayerListBox(GG::X0, GG::Y0, ClientWidth(), ClientHeight());
    m_player_list->SetHiliteColor(GG::CLR_ZERO);
    m_player_list->SetStyle(GG::LIST_QUICKSEL | GG::LIST_NOSORT);
    GG::Connect(m_player_list->SelChangedSignal,    &PlayerListWnd::PlayerSelectionChanged,  this);
    GG::Connect(m_player_list->DoubleClickedSignal, &PlayerListWnd::PlayerDoubleClicked,     this);
    AttachChild(m_player_list);

    DoLayout();

    Refresh();

    // TODO: connect selection signals of list
    //    mutable boost::signal<void ()>  SelectedPlayersChangedSignal;
}

std::set<int> PlayerListWnd::SelectedPlayerIDs() const {
    std::set<int> retval;
    for (GG::ListBox::iterator it = m_player_list->begin(); it != m_player_list->end(); ++it) {
        if (!m_player_list->Selected(it))
            continue;

        int selected_player_id = PlayerInRow(it);
        if (selected_player_id != Networking::INVALID_PLAYER_ID)
            retval.insert(selected_player_id);
    }
    return retval;
}

void PlayerListWnd::HandlePlayerStatusUpdate(Message::PlayerStatus player_status, int about_player_id) {
    for (CUIListBox::iterator row_it = m_player_list->begin(); row_it != m_player_list->end(); ++row_it) {
        CUIListBox::Row* row = *row_it;
        if (PlayerRow* player_row = dynamic_cast<PlayerRow*>(row)) {
            if (about_player_id == Networking::INVALID_PLAYER_ID) {
                player_row->SetStatus(player_status);
            } else if (player_row->PlayerID() == about_player_id) {
                player_row->SetStatus(player_status);
                return;
            }
        }
    }
}

void PlayerListWnd::Update() {
    for (CUIListBox::iterator row_it = m_player_list->begin(); row_it != m_player_list->end(); ++row_it) {
        CUIListBox::Row* row = *row_it;
        if (PlayerRow* player_row = dynamic_cast<PlayerRow*>(row))
            player_row->Update();
    }
}

void PlayerListWnd::Refresh() {
    std::set<int> initially_selected_players = this->SelectedPlayerIDs();

    m_player_list->Clear();

    const ClientApp* app = ClientApp::GetApp();
    if (!app) {
        Logger().errorStream() << "PlayerListWnd::Refresh couldn't get client app!";
        return;
    }
    const std::map<int, PlayerInfo>& players = app->Players();

    const GG::Pt row_size = m_player_list->ListRowSize();

    for (std::map<int, PlayerInfo>::const_iterator player_it = players.begin(); player_it != players.end(); ++player_it) {
        int player_id = player_it->first;
        PlayerRow* player_row = new PlayerRow(row_size.x, row_size.y, player_id);
        m_player_list->Insert(player_row);
        player_row->Resize(row_size);
    }

    this->SetSelectedPlayers(initially_selected_players);
}

void PlayerListWnd::SetSelectedPlayers(const std::set<int>& player_ids) {
    const GG::ListBox::SelectionSet initial_selections = m_player_list->Selections();

    m_player_list->DeselectAll();

    // early exit if nothing to select
    if (player_ids.empty()) {
        PlayerSelectionChanged(m_player_list->Selections());
        return;
    }

    // loop through players, selecting any indicated
    for (GG::ListBox::iterator it = m_player_list->begin(); it != m_player_list->end(); ++it) {
        PlayerRow* row = dynamic_cast<PlayerRow*>(*it);
        if (!row) {
            Logger().errorStream() << "PlayerRow::SetSelectedPlayers couldn't cast a listbow row to PlayerRow?";
            continue;
        }

        // if this row's player should be selected, so so
        if (player_ids.find(row->PlayerID()) != player_ids.end()) {
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

void PlayerListWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey>& mod_keys) {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt final_move(std::max(-ul.x, std::min(move.x, GG::GUI::GetGUI()->AppWidth() - 1 - lr.x)),
                      std::max(-ul.y, std::min(move.y, GG::GUI::GetGUI()->AppHeight() - 1 - lr.y)));
    GG::Wnd::LDrag(pt + final_move - move, final_move, mod_keys);
}

void PlayerListWnd::DoLayout() {
    m_player_list->SizeMove(GG::Pt(), GG::Pt(ClientWidth(), ClientHeight() - GG::Y(INNER_BORDER_ANGLE_OFFSET)));
}

void PlayerListWnd::PlayerSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    // mark as selected all PlayerDataPanel that are in \a rows and mark as not
    // selected all PlayerDataPanel that aren't in \a rows
    for (GG::ListBox::iterator it = m_player_list->begin(); it != m_player_list->end(); ++it) {
        bool select_this_row = (rows.find(it) != rows.end());

        GG::ListBox::Row* row = *it;
        if (!row) {
            Logger().errorStream() << "PlayerListWnd::PlayerSelectionChanged couldn't get row";
            continue;
        }
        if (row->empty()) {
            Logger().errorStream() << "PlayerListWnd::PlayerSelectionChanged got empty row";
            continue;
        }
        GG::Control* control = (*row)[0];
        if (!control) {
            Logger().errorStream() << "PlayerListWnd::PlayerSelectionChanged couldn't get control from row";
            continue;
        }
        PlayerDataPanel* data_panel = dynamic_cast<PlayerDataPanel*>(control);
        if (!data_panel) {
            Logger().errorStream() << "PlayerListWnd::PlayerSelectionChanged couldn't get PlayerDataPanel from control";
            continue;
        }
        data_panel->Select(select_this_row);
    }

    SelectedPlayersChangedSignal();
}

void PlayerListWnd::PlayerDoubleClicked(GG::ListBox::iterator it) {
    int player_id = PlayerInRow(it);
    if (player_id != Networking::INVALID_PLAYER_ID)
        PlayerDoubleClickedSignal(player_id);
}

int PlayerListWnd::PlayerInRow(GG::ListBox::iterator it) const {
    if (it == m_player_list->end())
        return Networking::INVALID_PLAYER_ID;

    if (PlayerRow* player_row = dynamic_cast<PlayerRow*>(*it))
        return player_row->PlayerID();

    return Networking::INVALID_PLAYER_ID;
}
