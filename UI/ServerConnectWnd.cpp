#include "ServerConnectWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "GGButton.h"
#include "GGTextControl.h"
#include "../client/human/HumanClientApp.h"
#include "SDLGGApp.h"

#include <sstream>
#include <iomanip>

namespace {
    const int SERVERS_LIST_BOX_WIDTH = 400;
    const int OK_CANCEL_BUTTON_WIDTH = 80;
    const int CONTROL_MARGIN = 5; // gap to leave between controls in the window
    const int LAN_SERVER_SEARCH_TIMEOUT = 1; // in seconds
    const int SERVER_CONNECT_WND_WIDTH = SERVERS_LIST_BOX_WIDTH + 6 * CONTROL_MARGIN;
    std::set<std::string> g_LAN_servers; // semi-persistent list of known LAN servers (persists only during runtime, but longer than the server connect window)

    bool NameOK(const std::string& name) {return !name.empty() && name.find_first_of(" \t:") == std::string::npos;}
}

ServerConnectWnd::ServerConnectWnd() : 
    CUI_Wnd(ClientUI::String("SCONNECT_WINDOW_TITLE"), (GG::App::GetApp()->AppWidth() - SERVER_CONNECT_WND_WIDTH) / 2, 
            (GG::App::GetApp()->AppHeight() - 500 + ClientUI::PTS + 10 + 3 * CONTROL_MARGIN) / 2, SERVER_CONNECT_WND_WIDTH, 
            500 + ClientUI::PTS + 10 + 3 * CONTROL_MARGIN, GG::Wnd::CLICKABLE | GG::Wnd::MODAL),
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
    GG::TextControl* temp = new GG::TextControl(LeftBorder() + CONTROL_MARGIN, TopBorder() + CONTROL_MARGIN, ClientUI::String("PLAYER_NAME_LABEL"), 
                                                ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    AttachChild(temp);
    const int PLAYER_NAME_EDIT_X = LeftBorder() + CONTROL_MARGIN + temp->Width() + CONTROL_MARGIN;
    m_player_name_edit = new CUIEdit(PLAYER_NAME_EDIT_X, TopBorder() + CONTROL_MARGIN, SERVER_CONNECT_WND_WIDTH - 2 * CONTROL_MARGIN - PLAYER_NAME_EDIT_X, 
                                     ClientUI::PTS + 10, "");

    m_host_or_join_radio_group = new GG::RadioButtonGroup(LeftBorder() + CONTROL_MARGIN, m_player_name_edit->LowerRight().y + CONTROL_MARGIN);
    m_host_or_join_radio_group->AddButton(new CUIStateButton(0, 0, SERVERS_LIST_BOX_WIDTH / 2, ClientUI::PTS + 4, ClientUI::String("HOST_GAME_BN"), GG::TF_LEFT, 
                                                             CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_host_or_join_radio_group->AddButton(new CUIStateButton(0, ClientUI::PTS + 4 + CONTROL_MARGIN, SERVERS_LIST_BOX_WIDTH / 2, ClientUI::PTS + 4, 
                                                             ClientUI::String("JOIN_GAME_BN"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));

    const int JOIN_CONTROLS_X = LeftBorder() + CONTROL_MARGIN + 10;
    m_LAN_game_label = new GG::TextControl(JOIN_CONTROLS_X, m_host_or_join_radio_group->LowerRight().y + CONTROL_MARGIN, 
                                           ClientUI::String("LAN_GAME_LABEL"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    m_servers_lb = new CUIListBox(JOIN_CONTROLS_X, m_LAN_game_label->LowerRight().y + CONTROL_MARGIN, SERVERS_LIST_BOX_WIDTH, 300);
    m_find_LAN_servers_bn = new CUIButton(JOIN_CONTROLS_X, m_servers_lb->LowerRight().y + CONTROL_MARGIN, 100, ClientUI::String("REFRESH_LIST_BN"));
    m_internet_game_label = new GG::TextControl(JOIN_CONTROLS_X, m_find_LAN_servers_bn->LowerRight().y + 2 * CONTROL_MARGIN, 
                                                ClientUI::String("INTERNET_GAME_LABEL"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    m_IP_address_edit = new CUIEdit(JOIN_CONTROLS_X, m_internet_game_label->LowerRight().y + CONTROL_MARGIN, SERVERS_LIST_BOX_WIDTH, ClientUI::PTS + 10, "");
    m_ok_bn = new CUIButton(Width() - RightBorder() - 2 * (OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN), m_IP_address_edit->LowerRight().y + 2 * CONTROL_MARGIN, 
                            OK_CANCEL_BUTTON_WIDTH, ClientUI::String("OK"));
    m_cancel_bn = new CUIButton(Width() - RightBorder() - (OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN), m_IP_address_edit->LowerRight().y + 2 * CONTROL_MARGIN, 
                                OK_CANCEL_BUTTON_WIDTH, ClientUI::String("CANCEL"));

    m_servers_lb->SetStyle(GG::LB_NOSORT | GG::LB_SINGLESEL);

    Init();
}

ServerConnectWnd::ServerConnectWnd(const GG::XMLElement& elem) : 
    CUI_Wnd(elem.Child("CUI_Wnd"))
{
    // TODO : implement if needed
}

void ServerConnectWnd::Keypress (GG::Key key, Uint32 key_mods)
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

    Connect(m_host_or_join_radio_group->ButtonChangedSignal(), &ServerConnectWnd::HostOrJoinClicked, this);
    Connect(m_servers_lb->SelChangedSignal(), &ServerConnectWnd::ServerSelected, this);
    Connect(m_find_LAN_servers_bn->ClickedSignal(), &ServerConnectWnd::RefreshServerList, this);
    Connect(m_IP_address_edit->EditedSignal(), &ServerConnectWnd::IPAddressEdited, this);
    Connect(m_player_name_edit->EditedSignal(), &ServerConnectWnd::NameEdited, this);
    Connect(m_ok_bn->ClickedSignal(), &ServerConnectWnd::OkClicked, this);
    Connect(m_cancel_bn->ClickedSignal(), &ServerConnectWnd::CancelClicked, this);

    m_host_or_join_radio_group->SetCheck(0);
    PopulateServerList();
    GG::App::GetApp()->SetFocusWnd(m_player_name_edit);
}

void ServerConnectWnd::PopulateServerList()
{
    m_servers_lb->Clear();
    for (std::set<std::string>::iterator it = g_LAN_servers.begin(); it != g_LAN_servers.end(); ++it) {
        GG::ListBox::Row* row = new GG::ListBox::Row;
        row->push_back(*it, ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
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
    if (!idx) { // host
        m_LAN_game_label->Disable();
        m_servers_lb->Disable();
        m_find_LAN_servers_bn->Disable();
        m_internet_game_label->Disable();
        m_IP_address_edit->Disable();
        if (NameOK(m_player_name_edit->WindowText()))
            m_ok_bn->Disable(false);
        else
            m_ok_bn->Disable();
    } else { // join
        m_LAN_game_label->Disable(false);
        m_servers_lb->Disable(false);
        m_find_LAN_servers_bn->Disable(false);
        m_internet_game_label->Disable(false);
        m_IP_address_edit->Disable(false);
        if ((m_servers_lb->Selections().empty() && m_IP_address_edit->WindowText() == "") || !NameOK(m_player_name_edit->WindowText()))
            m_ok_bn->Disable();
    }
}

void ServerConnectWnd::ServerSelected(const std::set<int>& selections)
{
    if (!selections.empty()) {
        *m_IP_address_edit << "";
        if (NameOK(m_player_name_edit->WindowText()))
            m_ok_bn->Disable(false);
    } else if (m_IP_address_edit->WindowText() == "") {
        m_ok_bn->Disable();
    }
}

void ServerConnectWnd::IPAddressEdited(const std::string& str)
{
    if (str != "") {
        m_servers_lb->ClearSelection();
        if (NameOK(m_player_name_edit->WindowText()))
            m_ok_bn->Disable(false);
    } else if (m_servers_lb->Selections().empty()) {
        m_ok_bn->Disable();
    }
}

void ServerConnectWnd::NameEdited(const std::string& str)
{
    if (str != "") {
        if (m_host_or_join_radio_group->CheckedButton() == 0)
            m_ok_bn->Disable(false);
        else if (!m_servers_lb->Selections().empty() || m_IP_address_edit->WindowText() != "")
            m_ok_bn->Disable(false);
    } else {
        m_ok_bn->Disable();
    }
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
    CUI_Wnd::CloseClicked();
}
