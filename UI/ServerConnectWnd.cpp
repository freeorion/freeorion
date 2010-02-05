#include "ServerConnectWnd.h"

#include "ClientUI.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <boost/filesystem/fstream.hpp>

#include <GG/Button.h>
#include <GG/Layout.h>
#include <GG/TextControl.h>

#include <boost/cast.hpp>

#include <sstream>
#include <iomanip>
#include <cctype>

namespace {
    const GG::X WINDOW_WIDTH(400);
    const GG::Y WINDOW_HEIGHT(535);

    bool NameOK(const std::string& name)
    {
        for (unsigned int i = 0; i < name.size(); ++i) {
            if (!std::isalnum(name[i]) && name[i] != '_' && name[i] != '-')
                return false;
        }
        return !name.empty();
    }

    void AddOptions(OptionsDB& db) {
        db.Add("multiplayersetup.player-name",  "OPTIONS_DB_MP_PLAYER_NAME",    std::string(""),            Validator<std::string>());
        db.Add("multiplayersetup.host-address", "OPTIONS_DB_MP_HOST_ADDRESS",   std::string("localhost"),   Validator<std::string>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

ServerConnectWnd::ServerConnectWnd() : 
    CUIWnd(UserString("SCONNECT_WINDOW_TITLE"),
           (GG::GUI::GetGUI()->AppWidth() - WINDOW_WIDTH) / 2, (GG::GUI::GetGUI()->AppHeight() - WINDOW_HEIGHT) / 2,
           WINDOW_WIDTH, WINDOW_HEIGHT, GG::INTERACTIVE | GG::MODAL),
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
    Sound::TempUISoundDisabler sound_disabler;

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    GG::TextControl* player_name_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, UserString("PLAYER_NAME_LABEL"), font, ClientUI::TextColor(), GG::FORMAT_LEFT);
    std::string player_name = GetOptionsDB().Get<std::string>("multiplayersetup.player-name");
    if (player_name.empty())
        player_name = UserString("DEFAULT_PLAYER_NAME");
    m_player_name_edit = new CUIEdit(GG::X0, GG::Y0, GG::X1, player_name);
    m_host_or_join_radio_group = new GG::RadioButtonGroup(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::VERTICAL);
    m_host_or_join_radio_group->AddButton(new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, UserString("HOST_GAME_BN"), GG::FORMAT_LEFT, GG::SBSTYLE_3D_RADIO));
    m_host_or_join_radio_group->AddButton(new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, UserString("JOIN_GAME_BN"), GG::FORMAT_LEFT, GG::SBSTYLE_3D_RADIO));
    m_LAN_game_label = new GG::TextControl(GG::X0, GG::Y0, UserString("LAN_GAME_LABEL"), font, ClientUI::TextColor(), GG::FORMAT_LEFT);
    m_servers_lb = new CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1);
    m_servers_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);
    m_find_LAN_servers_bn = new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("REFRESH_LIST_BN"));
    m_internet_game_label = new GG::TextControl(GG::X0, GG::Y0, UserString("INTERNET_GAME_LABEL"), font, ClientUI::TextColor(), GG::FORMAT_LEFT);
    m_IP_address_edit = new CUIEdit(GG::X0, GG::Y0, GG::X1, GetOptionsDB().Get<std::string>("multiplayersetup.host-address"));
    m_ok_bn = new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("OK"));
    m_cancel_bn = new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("CANCEL"));

    const GG::X OK_CANCEL_BUTTON_WIDTH(80);
    const int CONTROL_MARGIN = 5;

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 8, 4, CONTROL_MARGIN);
    layout->SetMinimumColumnWidth(0, player_name_label->MinUsableSize().x + CONTROL_MARGIN);
    layout->SetColumnStretch(1, 1.0);
    layout->SetMinimumColumnWidth(2, OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN);
    layout->SetMinimumColumnWidth(3, OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(0, m_player_name_edit->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(1, m_host_or_join_radio_group->MinUsableSize().y);
    layout->SetMinimumRowHeight(2, m_LAN_game_label->Height() + (2 * CONTROL_MARGIN));
    layout->SetRowStretch(3, 1.0);
    layout->SetMinimumRowHeight(4, m_find_LAN_servers_bn->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(5, m_internet_game_label->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(6, m_IP_address_edit->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(7, m_ok_bn->Height() + CONTROL_MARGIN);

    layout->Add(player_name_label, 0, 0, 1, 1, GG::ALIGN_VCENTER);
    layout->Add(m_player_name_edit, 0, 1, 1, 3, GG::ALIGN_VCENTER);
    layout->Add(m_host_or_join_radio_group, 1, 0, 1, 4);
    layout->Add(m_LAN_game_label, 2, 0, 1, 4, GG::ALIGN_BOTTOM);
    layout->Add(m_servers_lb, 3, 0, 1, 4);
    layout->Add(m_find_LAN_servers_bn, 4, 3);
    layout->Add(m_internet_game_label, 5, 0, 1, 4);
    layout->Add(m_IP_address_edit, 6, 0, 1, 4);
    layout->Add(m_ok_bn, 7, 2);
    layout->Add(m_cancel_bn, 7, 3);

    SetLayout(layout);

    m_LAN_servers = HumanClientApp::GetApp()->Networking().DiscoverLANServers();
    Init();
}

void ServerConnectWnd::ModalInit()
{
    GG::GUI::GetGUI()->SetFocusWnd(m_player_name_edit);
}

void ServerConnectWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
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
    Connect(m_host_or_join_radio_group->ButtonChangedSignal,    &ServerConnectWnd::HostOrJoinClicked,   this);
    Connect(m_servers_lb->SelChangedSignal,                     &ServerConnectWnd::ServerSelected,      this);
    Connect(m_find_LAN_servers_bn->ClickedSignal,               &ServerConnectWnd::RefreshServerList,   this);
    Connect(m_IP_address_edit->EditedSignal,                    &ServerConnectWnd::IPAddressEdited,     this);
    Connect(m_player_name_edit->EditedSignal,                   &ServerConnectWnd::NameEdited,          this);
    Connect(m_ok_bn->ClickedSignal,                             &ServerConnectWnd::OkClicked,           this);
    Connect(m_cancel_bn->ClickedSignal,                         &ServerConnectWnd::CancelClicked,       this);

    m_host_or_join_radio_group->SetCheck(0);
    PopulateServerList();
    if (m_servers_lb->NumRows()) {
        m_host_or_join_radio_group->SetCheck(1);
        m_servers_lb->SelectRow(m_servers_lb->begin());
        ServerSelected(m_servers_lb->Selections());
    }
    HostOrJoinClicked(m_host_or_join_radio_group->CheckedButton());
}

void ServerConnectWnd::PopulateServerList()
{
    m_servers_lb->Clear();
    for (ClientNetworking::ServerList::iterator it = m_LAN_servers.begin(); it != m_LAN_servers.end(); ++it) {
        GG::ListBox::Row* row = new GG::ListBox::Row;
        row->push_back(it->second, ClientUI::GetFont(), ClientUI::TextColor());
        m_servers_lb->Insert(row);
    }
}

void ServerConnectWnd::RefreshServerList()
{
    m_LAN_servers = HumanClientApp::GetApp()->Networking().DiscoverLANServers();
    PopulateServerList();
}

void ServerConnectWnd::HostOrJoinClicked(std::size_t idx)
{
    EnableDisableControls();
}

void ServerConnectWnd::ServerSelected(const GG::ListBox::SelectionSet& selections)
{
    if (!selections.empty())
        *m_IP_address_edit << "";
    EnableDisableControls();
}

void ServerConnectWnd::IPAddressEdited(const std::string& str)
{
    if (str != "") {
        m_servers_lb->DeselectAll();
        ServerSelected(m_servers_lb->Selections());
    }
    EnableDisableControls();
}

void ServerConnectWnd::NameEdited(const std::string& str)
{
    EnableDisableControls();
}

void ServerConnectWnd::OkClicked()
{
    // record selected galaxy setup options as new defaults
    GetOptionsDB().Set("multiplayersetup.player-name",  m_player_name_edit->Text());
    GetOptionsDB().Set("multiplayersetup.host-address", m_IP_address_edit->Text());

    // Save the changes:
    {
        boost::filesystem::ofstream ofs(GetConfigPath());
        if (ofs) {
            GetOptionsDB().GetXML().WriteDoc(ofs);
        } else {
            std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
            std::cerr << GetConfigPath().file_string() << std::endl;
            Logger().errorStream() << UserString("UNABLE_TO_WRITE_CONFIG_XML");
            Logger().errorStream() << GetConfigPath().file_string();
        }
    }

    m_result.first = *m_player_name_edit;
    if (m_host_or_join_radio_group->CheckedButton() == 0) {
        m_result.second = "HOST GAME SELECTED";
    } else {
        m_result.second = *m_IP_address_edit;
        if (m_result.second == "") {
            m_result.second =
                boost::polymorphic_downcast<GG::TextControl*>(
                    (***m_servers_lb->Selections().begin())[0])->Text();
        }
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
        !NameOK(m_player_name_edit->Text()) ||
        (!host_selected && m_servers_lb->Selections().empty() &&
         m_IP_address_edit->Text().empty());
    m_ok_bn->Disable(disable_ok_bn);
}
