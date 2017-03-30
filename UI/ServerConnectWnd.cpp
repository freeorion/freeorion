#include "ServerConnectWnd.h"

#include <sstream>
#include <iomanip>
#include <cctype>
#include <boost/cast.hpp>
#include <boost/filesystem/fstream.hpp>
#include <GG/Button.h>
#include <GG/Layout.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../client/human/HumanClientApp.h"
#include "../network/ClientNetworking.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "Sound.h"


namespace {
    const GG::X WINDOW_WIDTH(400);
    const GG::Y WINDOW_HEIGHT(535);

    bool NameOK(const std::string& name)
    {
        for (const std::string::value_type& character : name) {
            if (!std::isalnum(character) && character != '_' && character != '-')
                return false;
        }
        return !name.empty();
    }

    void AddOptions(OptionsDB& db) {
        db.Add("multiplayersetup.player-name",  UserStringNop("OPTIONS_DB_MP_PLAYER_NAME"),    std::string(""),            Validator<std::string>());
        db.Add("multiplayersetup.host-address", UserStringNop("OPTIONS_DB_MP_HOST_ADDRESS"),   std::string("localhost"),   Validator<std::string>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

ServerConnectWnd::ServerConnectWnd() :
    CUIWnd(UserString("SCONNECT_WINDOW_TITLE"),
           GG::INTERACTIVE | GG::MODAL),
    m_host_or_join_radio_group(nullptr),
    m_LAN_game_label(nullptr),
    m_servers_lb(nullptr),
    m_find_LAN_servers_bn(nullptr),
    m_internet_game_label(nullptr),
    m_IP_address_edit(nullptr),
    m_player_name_edit(nullptr),
    m_ok_bn(nullptr),
    m_cancel_bn(nullptr)
{
    Sound::TempUISoundDisabler sound_disabler;

    GG::Label* player_name_label = new CUILabel(UserString("PLAYER_NAME_LABEL"), GG::FORMAT_LEFT);
    m_player_name_edit = new CUIEdit(GetOptionsDB().Get<std::string>("multiplayersetup.player-name"));
    m_host_or_join_radio_group = new GG::RadioButtonGroup(GG::VERTICAL);
    m_host_or_join_radio_group->AddButton(new CUIStateButton(UserString("HOST_GAME_BN"), GG::FORMAT_LEFT, std::make_shared<CUIRadioRepresenter>()));
    m_host_or_join_radio_group->AddButton(new CUIStateButton(UserString("JOIN_GAME_BN"), GG::FORMAT_LEFT, std::make_shared<CUIRadioRepresenter>()));
    m_LAN_game_label = new CUILabel(UserString("LAN_GAME_LABEL"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
    m_servers_lb = new CUIListBox();
    m_servers_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);
    m_find_LAN_servers_bn = new CUIButton(UserString("REFRESH_LIST_BN"));
    m_internet_game_label = new CUILabel(UserString("INTERNET_GAME_LABEL"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
    m_IP_address_edit = new CUIEdit(GetOptionsDB().Get<std::string>("multiplayersetup.host-address"));
    m_ok_bn = new CUIButton(UserString("OK"));
    m_cancel_bn = new CUIButton(UserString("CANCEL"));

    const GG::X OK_CANCEL_BUTTON_WIDTH(100);
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
    layout->SetMinimumRowHeight(4, m_find_LAN_servers_bn->MinUsableSize().y + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(5, m_internet_game_label->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(6, m_IP_address_edit->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(7, m_ok_bn->MinUsableSize().y + CONTROL_MARGIN);

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

    ResetDefaultPosition();

    Init();
}

void ServerConnectWnd::ModalInit()
{ GG::GUI::GetGUI()->SetFocusWnd(m_player_name_edit); }

GG::Rect ServerConnectWnd::CalculatePosition() const {
    GG::Pt new_ul((GG::GUI::GetGUI()->AppWidth() - WINDOW_WIDTH) / 2,
                  (GG::GUI::GetGUI()->AppHeight() - WINDOW_HEIGHT) / 2);
    GG::Pt new_sz(WINDOW_WIDTH, WINDOW_HEIGHT);
    return GG::Rect(new_ul, new_ul + new_sz);
}

void ServerConnectWnd::KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (!m_ok_bn->Disabled() && (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER)) { // Same behaviour as if "OK" was pressed
        OkClicked();
    } else if (key == GG::GGK_ESCAPE) { // Same behaviour as if "Cancel" was pressed
        CancelClicked();
    }
}

const std::pair<std::string, std::string>& ServerConnectWnd::Result() const
{ return m_result; }

void ServerConnectWnd::Init() {
    Connect(m_host_or_join_radio_group->ButtonChangedSignal,    &ServerConnectWnd::HostOrJoinClicked,   this);
    Connect(m_servers_lb->SelChangedSignal,                     &ServerConnectWnd::ServerSelected,      this);
    Connect(m_find_LAN_servers_bn->LeftClickedSignal,           &ServerConnectWnd::RefreshServerList,   this);
    Connect(m_IP_address_edit->EditedSignal,                    &ServerConnectWnd::IPAddressEdited,     this);
    Connect(m_player_name_edit->EditedSignal,                   &ServerConnectWnd::NameEdited,          this);
    Connect(m_ok_bn->LeftClickedSignal,                         &ServerConnectWnd::OkClicked,           this);
    Connect(m_cancel_bn->LeftClickedSignal,                     &ServerConnectWnd::CancelClicked,       this);

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
    const auto server_names = HumanClientApp::GetApp()->Networking().DiscoverLANServerNames();
    for (const auto& server : server_names) {
        GG::ListBox::Row* row = new GG::ListBox::Row;
        row->push_back(new CUILabel(server));
        m_servers_lb->Insert(row);
    }
}

void ServerConnectWnd::RefreshServerList()
{
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
    GetOptionsDB().Commit();

    m_result.first = *m_player_name_edit;
    if (m_host_or_join_radio_group->CheckedButton() == 0) {
        m_result.second = "HOST GAME SELECTED";
    } else {
        m_result.second = *m_IP_address_edit;
        if (m_result.second == "" && !(***m_servers_lb->Selections().begin()).empty()) {
            m_result.second =
                boost::polymorphic_downcast<GG::Label*>((***m_servers_lb->Selections().begin()).at(0))->Text() ;
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
