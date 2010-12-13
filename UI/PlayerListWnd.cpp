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
            m_status_text(0),
            m_player_type_text(0),
            m_host_text(0),
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

            m_status_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, h, "",
                                                ClientUI::GetFont(), ClientUI::TextColor(),
                                                GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_status_text);

            m_player_type_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, h, "",
                                                     ClientUI::GetFont(), ClientUI::TextColor(),
                                                     GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_player_type_text);

            m_host_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, h, "",
                                              ClientUI::GetFont(), ClientUI::TextColor(),
                                              GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_host_text);

            DoLayout();
            Update();
        }

        /** Excludes border from the client area. */
        virtual GG::Pt  ClientUpperLeft() const {
            return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));
        }

        /** Excludes border from the client area. */
        virtual GG::Pt  ClientLowerRight() const {
            return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); 
        }

        /** Renders black panel background, border with color depending on the current state and a background for the ship's name text. */
        virtual void    Render() {
            // main background position and colour
            const GG::Clr& background_colour = ClientUI::WndColor();
            const GG::Pt ul = UpperLeft(), lr = LowerRight(), cul = ClientUpperLeft();

            // title background colour and position
            const GG::Clr& unselected_colour = ClientUI::WndOuterBorderColor();
            const GG::Clr& selected_colour = ClientUI::WndInnerBorderColor();
            GG::Clr border_colour = unselected_colour;//m_selected ? selected_colour : unselected_colour;
            if (Disabled())
                border_colour = DisabledColor(border_colour);

            // render
            GG::FlatRectangle(ul,       lr,         background_colour,  border_colour, DATA_PANEL_BORDER);  // background and border
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
            }

            m_player_name_text->SetTextColor(empire_color);
            m_player_name_text->SetText(player_info.name);

            m_empire_name_text->SetTextColor(empire_color);
            m_empire_name_text->SetText(empire_name);

            // status not stored locally, so m_status_text unchanged

            std::string player_type_str;
            switch (player_info.client_type) {
            case Networking::CLIENT_TYPE_HUMAN_PLAYER:      player_type_str = UserString("HUMAN_PLAYER");   break;
            case Networking::CLIENT_TYPE_AI_PLAYER:         player_type_str = UserString("AI_PLAYER");      break;
            case Networking::CLIENT_TYPE_HUMAN_OBSERVER:    player_type_str = UserString("OBSERVER");       break;
            default:    // leave empty
                Logger().errorStream() << "PlayerDataPanel::Update got unknown player type";
            }
            m_player_type_text->SetText(player_type_str);

            std::string host_str;
            if (player_info.host)
                host_str = UserString("HOST");
            m_host_text->SetText(host_str);
        }

        void            SetStatus(Message::PlayerStatus player_status) {
            m_status_text->SetText(UserString(PlayerStatusStr(player_status)));
        }

    private:

        void            DoLayout() {
            const GG::X PLAYER_NAME_WIDTH(ClientUI::Pts() * 15/2);
            const GG::X EMPIRE_NAME_WIDTH(ClientUI::Pts() * 17/2);
            const GG::X STATUS_WIDTH(ClientUI::Pts() * 5);
            const GG::X PLAYER_TYPE_WIDTH(ClientUI::Pts() * 4);
            const GG::X HOST_WIDTH(ClientUI::Pts() * 3);

            GG::X left(DATA_PANEL_BORDER);
            GG::Y top(DATA_PANEL_BORDER);
            GG::Y bottom(ClientHeight());

            m_player_name_text->SizeMove(GG::Pt(left, top), GG::Pt(left + PLAYER_NAME_WIDTH, bottom));
            left += PLAYER_NAME_WIDTH;

            m_empire_name_text->SizeMove(GG::Pt(left, top), GG::Pt(left + EMPIRE_NAME_WIDTH, bottom));
            left += EMPIRE_NAME_WIDTH;

            m_status_text->SizeMove(GG::Pt(left, top), GG::Pt(left + STATUS_WIDTH, bottom));
            left += STATUS_WIDTH;

            m_player_type_text->SizeMove(GG::Pt(left, top), GG::Pt(left + PLAYER_TYPE_WIDTH, bottom));
            left += PLAYER_TYPE_WIDTH;

            m_host_text->SizeMove(GG::Pt(left, top), GG::Pt(left + HOST_WIDTH, bottom));
            left += HOST_WIDTH;
        }

        int                 m_player_id;
        GG::TextControl*    m_player_name_text;
        GG::TextControl*    m_empire_name_text;
        GG::TextControl*    m_status_text;
        GG::TextControl*    m_player_type_text;
        GG::TextControl*    m_host_text;
        bool                m_selected;
    };


    ////////////////////////////////////////////////
    // PlayerRow
    ////////////////////////////////////////////////
    class PlayerRow : public GG::ListBox::Row {
    public:
        PlayerRow(GG::X w, GG::Y h, int player_id) :
            GG::ListBox::Row(w, h, ""),
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
    AttachChild(m_player_list);

    DoLayout();

    Refresh();

    // TODO: connect selection signals of list
    //    mutable boost::signal<void ()>  SelectedPlayersChangedSignal;
}

std::set<int> PlayerListWnd::SelectedPlayerIDs() const {
    return std::set<int>();
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
