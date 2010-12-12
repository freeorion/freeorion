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
    std::vector<GG::X>  ColWidths(GG::X total_width) {
        const GG::X PLAYER_NAME_WIDTH(ClientUI::Pts() * 24);
        const GG::X EMPIRE_NAME_WIDTH(ClientUI::Pts() * 36);
        const GG::X STATUS_WIDTH(ClientUI::Pts() * 8);
        const GG::X PLAYER_TYPE_WIDTH(ClientUI::Pts() * 8);
        //const GG::X HOST_WIDTH(ClientUI::Pts() * 6);  // not currently used, but if another column is needed..

        const GG::X LAST_WIDTH = std::max(GG::X1, total_width - PLAYER_NAME_WIDTH - EMPIRE_NAME_WIDTH - STATUS_WIDTH - PLAYER_TYPE_WIDTH);

        std::vector<GG::X> retval;
        retval.push_back(PLAYER_NAME_WIDTH);
        retval.push_back(EMPIRE_NAME_WIDTH);
        retval.push_back(STATUS_WIDTH);
        retval.push_back(PLAYER_TYPE_WIDTH);
        retval.push_back(LAST_WIDTH);
        return retval;
    }

    class PlayerRow : public CUIListBox::Row {
    public:
        PlayerRow(GG::X w, GG::Y h, int player_id) :
            CUIListBox::Row(w, h, "PlayerRow"),
            m_player_id(player_id),
            m_player_name_text(0),
            m_empire_name_text(0),
            m_status_text(0),
            m_player_type_text(0),
            m_host_text(0)
        {
            SetDragDropDataType("PLAYER_ROW");
            SetChildClippingMode(ClipToClient);

            std::vector<GG::X> col_widths = ColWidths(w);

            m_player_name_text = new GG::TextControl(GG::X0, GG::Y0, col_widths[0], h, "",
                                                     ClientUI::GetFont(), ClientUI::TextColor(),
                                                     GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            push_back(m_player_name_text);

            m_empire_name_text = new GG::TextControl(GG::X0, GG::Y0, col_widths[1], h, "",
                                                     ClientUI::GetFont(), ClientUI::TextColor(),
                                                     GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            push_back(m_empire_name_text);

            m_status_text = new GG::TextControl(GG::X0, GG::Y0, col_widths[2], h, "",
                                                ClientUI::GetFont(), ClientUI::TextColor(),
                                                GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            push_back(m_status_text);

            m_player_type_text = new GG::TextControl(GG::X0, GG::Y0, col_widths[3], h, "",
                                                     ClientUI::GetFont(), ClientUI::TextColor(),
                                                     GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            push_back(m_player_type_text);

            m_host_text = new GG::TextControl(GG::X0, GG::Y0, col_widths[4], h, "",
                                              ClientUI::GetFont(), ClientUI::TextColor(),
                                              GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            push_back(m_host_text);

            Update();
        }

        int     PlayerID() const {
            return m_player_id;
        }

        void    Update() {
            const ClientApp* app = ClientApp::GetApp();
            if (!app) {
                Logger().errorStream() << "PlayerRow::Update couldn't get client app!";
                return;
            }

            const std::map<int, PlayerInfo>& players = app->Players();

            std::map<int, PlayerInfo>::const_iterator player_it = players.find(m_player_id);
            if (player_it == players.end()) {
                Logger().errorStream() << "PlayerRow::Update couldn't find player with id " << m_player_id;
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
                Logger().errorStream() << "PlayerRow::Update got unknown player type";
            }
            m_player_type_text->SetText(player_type_str);

            std::string host_str;
            if (player_info.host)
                host_str = UserString("HOST");
            m_host_text->SetText(host_str);
        }

        void    SetStatus(Message::PlayerStatus player_status) {
            if (m_status_text)
                m_status_text->SetText(UserString(PlayerStatusStr(player_status)));
        }

        //virtual void    Render() {
        //    CUIListBox::Row::Render();
        //    // draw yellow box
        //    GG::Rect bounds(UpperLeft(), LowerRight());
        //    FlatRectangle(bounds.ul, bounds.lr, GG::CLR_ZERO, GG::CLR_YELLOW, 1);
        //}

    private:
        int                 m_player_id;
        GG::TextControl*    m_player_name_text;
        GG::TextControl*    m_empire_name_text;
        GG::TextControl*    m_status_text;
        GG::TextControl*    m_player_type_text;
        GG::TextControl*    m_host_text;
    };
}

/////////////////////
//  PlayerListWnd  //
/////////////////////
PlayerListWnd::PlayerListWnd(GG::X x, GG::Y y, GG::X w, GG::Y h) :
    CUIWnd(UserString("PLAYERS_LIST_PANEL_TITLE"), x, y, w, h, GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | GG::RESIZABLE | CLOSABLE),
    m_player_list(0)
{
    m_player_list = new CUIListBox(GG::X0, GG::Y0, ClientWidth(), ClientHeight());
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

    for (std::map<int, PlayerInfo>::const_iterator player_it = players.begin(); player_it != players.end(); ++player_it) {
        int player_id = player_it->first;
        PlayerRow* player_row = new PlayerRow(Width() - ClientUI::ScrollWidth() - 6, GG::Y(ClientUI::Pts() * 3/2), player_id);
        m_player_list->Insert(player_row);
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
