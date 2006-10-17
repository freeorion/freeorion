#include "ServerConnectWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"

#include <GG/Button.h>
#include <GG/TextControl.h>
#include <GG/SDL/SDLGUI.h>

#include <sstream>
#include <iomanip>
#include <cctype>

namespace {
    const int SERVERS_LIST_BOX_WIDTH = 400;
    const int OK_CANCEL_BUTTON_WIDTH = 80;
    const int CONTROL_MARGIN = 5; // gap to leave between controls in the window
    const int LAN_SERVER_SEARCH_TIMEOUT = 1; // in seconds
    const int SERVER_CONNECT_WND_WIDTH = SERVERS_LIST_BOX_WIDTH + 6 * CONTROL_MARGIN;
    std::set<std::string> g_LAN_servers; // semi-persistent list of known LAN servers (persists only during runtime, but longer than the server connect window)

    bool NameOK(const std::string& name)
    {
        for (unsigned int i = 0; i < name.size(); ++i) {
            if (!std::isalnum(name[i]) && name[i] != '_' && name[i] != '-')
                return false;
        }
        return !name.empty();
    }

}

ServerConnectWnd::ServerConnectWnd() : 
    CUIWnd(UserString("SCONNECT_WINDOW_TITLE"), (GG::GUI::GetGUI()->AppWidth() - SERVER_CONNECT_WND_WIDTH) / 2, 
           (GG::GUI::GetGUI()->AppHeight() - 500 + ClientUI::Pts() + 10 + 3 * CONTROL_MARGIN) / 2, SERVER_CONNECT_WND_WIDTH, 
           500 + ClientUI::Pts() + 10 + 3 * CONTROL_MARGIN, GG::CLICKABLE | GG::MODAL),
    m_host_or_join_radio_group(0),
    m_LAN_game_label(0),
    m_servers_lb(0),
    m_find_LAN_servers_bn(0),
    m_internet_game_label(0),
    m_IP_address_edit(0),
    m_player_name_edit(0),
    m_ok_bn(0),
    m_cancel_bn(0)
{
    TempUISoundDisabler sound_disabler;

    int text_width = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts())->TextExtent(UserString("PLAYER_NAME_LABEL")).x;
    const int PLAYER_NAME_EDIT_X = CONTROL_MARGIN + text_width + CONTROL_MARGIN;
    m_player_name_edit = new CUIEdit(PLAYER_NAME_EDIT_X, CONTROL_MARGIN, ClientWidth() - 5 - PLAYER_NAME_EDIT_X, "");

    AttachChild(new GG::TextControl(CONTROL_MARGIN, CONTROL_MARGIN, text_width, m_player_name_edit->Height(),
                                    UserString("PLAYER_NAME_LABEL"), 
                                    GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor()));

    m_host_or_join_radio_group = new GG::RadioButtonGroup(CONTROL_MARGIN, m_player_name_edit->LowerRight().y + CONTROL_MARGIN,
                                                          SERVERS_LIST_BOX_WIDTH / 2, ClientUI::Pts() + 4 + CONTROL_MARGIN + ClientUI::Pts() + 4, GG::VERTICAL);
    m_host_or_join_radio_group->AddButton(new CUIStateButton(0, 0, SERVERS_LIST_BOX_WIDTH / 2, ClientUI::Pts() + 4, UserString("HOST_GAME_BN"), GG::TF_LEFT, 
                                                             GG::SBSTYLE_3D_RADIO));
    m_host_or_join_radio_group->AddButton(new CUIStateButton(0, ClientUI::Pts() + 4 + CONTROL_MARGIN, SERVERS_LIST_BOX_WIDTH / 2, ClientUI::Pts() + 4, 
                                                             UserString("JOIN_GAME_BN"), GG::TF_LEFT, GG::SBSTYLE_3D_RADIO));

    const int JOIN_CONTROLS_X = CONTROL_MARGIN + 10;
    m_LAN_game_label = new GG::TextControl(JOIN_CONTROLS_X, m_host_or_join_radio_group->LowerRight().y + CONTROL_MARGIN, 
                                           UserString("LAN_GAME_LABEL"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor());
    m_servers_lb = new CUIListBox(JOIN_CONTROLS_X, m_LAN_game_label->LowerRight().y + CONTROL_MARGIN, SERVERS_LIST_BOX_WIDTH, 300 - CONTROL_MARGIN);
    m_find_LAN_servers_bn = new CUIButton(JOIN_CONTROLS_X, m_servers_lb->LowerRight().y + CONTROL_MARGIN, 100, UserString("REFRESH_LIST_BN"));
    m_internet_game_label = new GG::TextControl(JOIN_CONTROLS_X, m_find_LAN_servers_bn->LowerRight().y + 2 * CONTROL_MARGIN, 
                                                UserString("INTERNET_GAME_LABEL"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor());
    m_IP_address_edit = new CUIEdit(JOIN_CONTROLS_X, m_internet_game_label->LowerRight().y + CONTROL_MARGIN, SERVERS_LIST_BOX_WIDTH, "");
    m_ok_bn = new CUIButton(ClientWidth() - 2 * (OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN), m_IP_address_edit->LowerRight().y + 2 * CONTROL_MARGIN, 
                            OK_CANCEL_BUTTON_WIDTH, UserString("OK"));
    m_cancel_bn = new CUIButton(ClientWidth() - (OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN), m_IP_address_edit->LowerRight().y + 2 * CONTROL_MARGIN, 
                                OK_CANCEL_BUTTON_WIDTH, UserString("CANCEL"));

    m_servers_lb->SetStyle(GG::LB_NOSORT | GG::LB_SINGLESEL);

    g_LAN_servers = HumanClientApp::GetApp()->NetworkCore().DiscoverLANServers(LAN_SERVER_SEARCH_TIMEOUT);
    Init();
}

void ServerConnectWnd::ModalInit()
{
    GG::GUI::GetGUI()->SetFocusWnd(m_player_name_edit);
}

void ServerConnectWnd::KeyPress(GG::Key key, Uint32 key_mods)
{
    if (!m_ok_bn->Disabled() && (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER)) { // Same behaviour as if "OK" was pressed
        OkClicked();
    } else if (key == GG::GGK_ESCAPE) { // Same behaviour as if "Cancel" was pressed
        CancelClicked();
    }
}

const std::pair<std::string, std::string>& ServerConnectWnd::Result() const
{
    return m_result;
}

void ServerConnectWnd::Init()
{
    AttachSignalChildren();

    Connect(m_host_or_join_radio_group->ButtonChangedSignal, &ServerConnectWnd::HostOrJoinClicked, this);
    Connect(m_servers_lb->SelChangedSignal, &ServerConnectWnd::ServerSelected, this);
    Connect(m_find_LAN_servers_bn->ClickedSignal, &ServerConnectWnd::RefreshServerList, this);
    Connect(m_IP_address_edit->EditedSignal, &ServerConnectWnd::IPAddressEdited, this);
    Connect(m_player_name_edit->EditedSignal, &ServerConnectWnd::NameEdited, this);
    Connect(m_ok_bn->ClickedSignal, &ServerConnectWnd::OkClicked, this);
    Connect(m_cancel_bn->ClickedSignal, &ServerConnectWnd::CancelClicked, this);

    m_host_or_join_radio_group->SetCheck(0);
    PopulateServerList();
    if (m_servers_lb->NumRows()) {
        m_host_or_join_radio_group->SetCheck(1);
        m_servers_lb->SelectRow(0);
    }
}

void ServerConnectWnd::PopulateServerList()
{
    m_servers_lb->Clear();
    for (std::set<std::string>::iterator it = g_LAN_servers.begin(); it != g_LAN_servers.end(); ++it) {
        GG::ListBox::Row* row = new GG::ListBox::Row;
        row->push_back(*it, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor());
        m_servers_lb->Insert(row);
    }
}

void ServerConnectWnd::AttachSignalChildren()
{
    AttachChild(m_host_or_join_radio_group);
    AttachChild(m_LAN_game_label);
    AttachChild(m_servers_lb);
    AttachChild(m_find_LAN_servers_bn);
    AttachChild(m_internet_game_label);
    AttachChild(m_IP_address_edit);
    AttachChild(m_player_name_edit);
    AttachChild(m_ok_bn);
    AttachChild(m_cancel_bn);
}

void ServerConnectWnd::DetachSignalChildren()
{
    DetachChild(m_host_or_join_radio_group);
    DetachChild(m_LAN_game_label);
    DetachChild(m_servers_lb);
    DetachChild(m_find_LAN_servers_bn);
    DetachChild(m_internet_game_label);
    DetachChild(m_IP_address_edit);
    DetachChild(m_player_name_edit);
    DetachChild(m_ok_bn);
    DetachChild(m_cancel_bn);
}

void ServerConnectWnd::RefreshServerList()
{
    g_LAN_servers = HumanClientApp::GetApp()->NetworkCore().DiscoverLANServers(LAN_SERVER_SEARCH_TIMEOUT);
    PopulateServerList();
}

void ServerConnectWnd::HostOrJoinClicked(int idx)
{
    EnableDisableControls();
}

void ServerConnectWnd::ServerSelected(const std::set<int>& selections)
{
    if (!selections.empty())
        *m_IP_address_edit << "";
    EnableDisableControls();
}

void ServerConnectWnd::IPAddressEdited(const std::string& str)
{
    if (str != "")
        m_servers_lb->DeselectAll();
    EnableDisableControls();
}

void ServerConnectWnd::NameEdited(const std::string& str)
{
    EnableDisableControls();
}

void ServerConnectWnd::OkClicked()
{
    m_result.first = *m_player_name_edit;
    if (m_host_or_join_radio_group->CheckedButton() == 0) {
        m_result.second = "HOST GAME SELECTED";
    } else {
        m_result.second = *m_IP_address_edit;
        if (m_result.second == "")
            m_result.second = m_servers_lb->GetRow(*m_servers_lb->Selections().begin())[0]->WindowText();
    }
    CUIWnd::CloseClicked();
}

void ServerConnectWnd::EnableDisableControls()
{
    bool host_selected = m_host_or_join_radio_group->CheckedButton() == 0;
    m_LAN_game_label->Disable(host_selected);
    m_servers_lb->Disable(host_selected);
    m_find_LAN_servers_bn->Disable(host_selected);
    m_internet_game_label->Disable(host_selected);
    m_IP_address_edit->Disable(host_selected);
    bool disable_ok_bn =
        !NameOK(m_player_name_edit->WindowText()) ||
        (!host_selected && m_servers_lb->Selections().empty() &&
         m_IP_address_edit->WindowText().empty());
    m_ok_bn->Disable(disable_ok_bn);
}
