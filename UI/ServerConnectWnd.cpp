#include "ServerConnectWnd.h"

#include <sstream>
#include <iomanip>
#include <cctype>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/fstream.hpp>
#include <GG/Button.h>
#include <GG/Layout.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../client/human/GGHumanClientApp.h"
#include "../client/ClientNetworking.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "Sound.h"


namespace {
    constexpr GG::X WINDOW_WIDTH{400};
    constexpr GG::Y WINDOW_HEIGHT{535};

    bool NameOK(const std::string& name) {
        for (const auto& character : name) {
            if (!std::isalnum(character) && character != '_' && character != '-')
                return false;
        }
        return !name.empty();
    }

    void AddOptions(OptionsDB& db) {
        db.Add("setup.multiplayer.player.name",     UserStringNop("OPTIONS_DB_MP_PLAYER_NAME"),     std::string(""),            Validator<std::string>());
        db.Add("setup.multiplayer.host.address",    UserStringNop("OPTIONS_DB_MP_HOST_ADDRESS"),    std::string("localhost"),   Validator<std::string>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    class ClientTypeRow : public GG::DropDownList::Row {
    public:
        ClientTypeRow(Networking::ClientType type_) :
            GG::DropDownList::Row(),
            type(type_)
        {
            switch (type) {
            case Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER:
                m_label = GG::Wnd::Create<CUILabel>(UserString("PLAYER"));
                break;
            case Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR:
                m_label = GG::Wnd::Create<CUILabel>(UserString("MODERATOR"));
                break;
            case Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER:
                m_label = GG::Wnd::Create<CUILabel>(UserString("OBSERVER"));
                break;
            default:
                break;
            }
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            if (m_label) {
                push_back(m_label);
            }
        }

        const Networking::ClientType type;
    private:
        std::shared_ptr<CUILabel> m_label;
    };
}

ServerConnectWnd::ServerConnectWnd() :
    CUIWnd(UserString("SCONNECT_WINDOW_TITLE"),
           GG::INTERACTIVE | GG::MODAL)
{}

void ServerConnectWnd::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    Sound::TempUISoundDisabler sound_disabler;

    auto player_name_label = GG::Wnd::Create<CUILabel>(UserString("PLAYER_NAME_LABEL"), GG::FORMAT_LEFT);
    m_player_name_edit = GG::Wnd::Create<CUIEdit>(GetOptionsDB().Get<std::string>("setup.multiplayer.player.name"));
    m_host_or_join_radio_group = GG::Wnd::Create<GG::RadioButtonGroup>(GG::Orientation::VERTICAL);
    m_host_or_join_radio_group->AddButton(GG::Wnd::Create<CUIStateButton>(UserString("HOST_GAME_BN"), GG::FORMAT_LEFT, std::make_shared<CUIRadioRepresenter>()));
    m_host_or_join_radio_group->AddButton(GG::Wnd::Create<CUIStateButton>(UserString("JOIN_GAME_BN"), GG::FORMAT_LEFT, std::make_shared<CUIRadioRepresenter>()));
    m_client_type_list = GG::Wnd::Create<CUIDropDownList>(3);
    m_client_type_list->SetStyle(GG::LIST_NOSORT);
    m_LAN_game_label = GG::Wnd::Create<CUILabel>(UserString("LAN_GAME_LABEL"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
    m_servers_lb = GG::Wnd::Create<CUIListBox>();
    m_servers_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);
    m_find_LAN_servers_bn = Wnd::Create<CUIButton>(UserString("REFRESH_LIST_BN"));
    m_internet_game_label = GG::Wnd::Create<CUILabel>(UserString("INTERNET_GAME_LABEL"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
    m_IP_address_edit = GG::Wnd::Create<CUIEdit>(GetOptionsDB().Get<std::string>("setup.multiplayer.host.address"));
    m_ok_bn = Wnd::Create<CUIButton>(UserString("OK"));
    m_cancel_bn = Wnd::Create<CUIButton>(UserString("CANCEL"));

    static constexpr GG::X OK_CANCEL_BUTTON_WIDTH{100};
    static constexpr int CONTROL_MARGIN = 5;

    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 8, 4, CONTROL_MARGIN);
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
    layout->Add(m_host_or_join_radio_group, 1, 0, 1, 2, GG::ALIGN_VCENTER);
    layout->Add(m_client_type_list, 1, 2, 1, 2, GG::ALIGN_BOTTOM);
    layout->Add(m_LAN_game_label, 2, 0, 1, 4, GG::ALIGN_BOTTOM);
    layout->Add(m_servers_lb, 3, 0, 1, 4);
    layout->Add(m_find_LAN_servers_bn, 4, 3);
    layout->Add(m_internet_game_label, 5, 0, 1, 4);
    layout->Add(m_IP_address_edit, 6, 0, 1, 4);
    layout->Add(m_ok_bn, 7, 2);
    layout->Add(m_cancel_bn, 7, 3);

    SetLayout(layout);

    ResetDefaultPosition();

    using boost::placeholders::_1;

    m_host_or_join_radio_group->ButtonChangedSignal.connect(
        boost::bind(&ServerConnectWnd::EnableDisableControls, this));
    m_servers_lb->SelRowsChangedSignal.connect(
        boost::bind(&ServerConnectWnd::ServerSelected, this, _1));
    m_find_LAN_servers_bn->LeftClickedSignal.connect(
        boost::bind(&ServerConnectWnd::PopulateServerList, this));
    m_IP_address_edit->EditedSignal.connect(
        boost::bind(&ServerConnectWnd::IPAddressEdited, this, _1));
    m_player_name_edit->EditedSignal.connect(
        boost::bind(&ServerConnectWnd::EnableDisableControls, this));
    m_ok_bn->LeftClickedSignal.connect(
        boost::bind(&ServerConnectWnd::OkClicked, this));
    m_cancel_bn->LeftClickedSignal.connect(
        boost::bind(&ServerConnectWnd::CancelClicked, this));

    m_client_type_list->Insert(GG::Wnd::Create<ClientTypeRow>(Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER));
    m_client_type_list->Insert(GG::Wnd::Create<ClientTypeRow>(Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR));
    m_client_type_list->Insert(GG::Wnd::Create<ClientTypeRow>(Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER));

    m_client_type_list->Select(0);
    m_result.type = Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER;

    m_host_or_join_radio_group->SetCheck(0);
    PopulateServerList();
    if (m_servers_lb->NumRows()) {
        m_host_or_join_radio_group->SetCheck(1);
        m_servers_lb->SelectRow(m_servers_lb->begin());
        ServerSelected(m_servers_lb->Selections());
    }
    EnableDisableControls();
}

void ServerConnectWnd::ModalInit()
{ GG::GUI::GetGUI()->SetFocusWnd(m_player_name_edit); }

GG::Rect ServerConnectWnd::CalculatePosition() const {
    GG::Pt new_ul((GG::GUI::GetGUI()->AppWidth() - WINDOW_WIDTH) / 2,
                  (GG::GUI::GetGUI()->AppHeight() - WINDOW_HEIGHT) / 2);
    GG::Pt new_sz(WINDOW_WIDTH, WINDOW_HEIGHT);
    return GG::Rect(new_ul, new_ul + new_sz);
}

void ServerConnectWnd::KeyPress(GG::Key key, uint32_t key_code_point,
                                GG::Flags<GG::ModKey> mod_keys)
{
    if (!m_ok_bn->Disabled() && (key == GG::Key::GGK_RETURN || key == GG::Key::GGK_KP_ENTER)) {
        // Same behaviour as if "OK" was pressed
        OkClicked();
    } else if (key == GG::Key::GGK_ESCAPE) {
        // Same behaviour as if "Cancel" was pressed
        CancelClicked();
    }
}

namespace {
    auto to_string_vec(const std::vector<std::string_view>& svs) {
        std::vector<std::string> retval;
        retval.reserve(svs.size());
        std::transform(svs.begin(), svs.end(), std::back_inserter(retval),
                       [](const auto sv) { return std::string{sv}; });
        return retval;
    }
}

void ServerConnectWnd::PopulateServerList() {
    m_servers_lb->Clear();
    {
        auto server_names = GGHumanClientApp::GetApp()->Networking().DiscoverLANServerNames();
        for (const auto& server_name : server_names) {
            auto row = GG::Wnd::Create<GG::ListBox::Row>();
            row->push_back(GG::Wnd::Create<CUILabel>(std::move(server_name)));
            m_servers_lb->Insert(row);
        }
    }
    // make local copies of server name options, since code below will possibly add more options.
    // that could invalidate any views into names of existing options.
    auto known_servers_options = to_string_vec(GetOptionsDB().FindOptions("network.known-servers", true));

    for (const auto& option : known_servers_options) {
        if (boost::algorithm::ends_with(option, ".address")) {
            if (!GetOptionsDB().OptionExists(option))
                GetOptionsDB().Add<std::string>(option, "OPTIONS_DB_SERVER_COOKIE", "");
            auto server = GetOptionsDB().Get<std::string>(option);
            auto row = GG::Wnd::Create<GG::ListBox::Row>();
            row->push_back(GG::Wnd::Create<CUILabel>(std::move(server)));
            m_servers_lb->Insert(std::move(row));
        }
    }
}

void ServerConnectWnd::ServerSelected(const GG::ListBox::SelectionSet& selections) {
    if (!selections.empty())
        *m_IP_address_edit << "";
    EnableDisableControls();
}

void ServerConnectWnd::IPAddressEdited(const std::string& str) {
    if (!str.empty()) {
        m_servers_lb->DeselectAll();
        ServerSelected(m_servers_lb->Selections());
    }
    EnableDisableControls();
}

void ServerConnectWnd::OkClicked() {
    // record selected galaxy setup options as new defaults
    GetOptionsDB().Set("setup.multiplayer.player.name", m_player_name_edit->Text());
    GetOptionsDB().Set("setup.multiplayer.host.address", m_IP_address_edit->Text());
    GetOptionsDB().Commit();

    m_result.player_name = *m_player_name_edit;
    auto it = m_client_type_list->CurrentItem();
    if (it != m_client_type_list->end()) {
        if (const auto* type_row = dynamic_cast<const ClientTypeRow*>(it->get()))
            m_result.type = type_row->type;
    }
    if (m_host_or_join_radio_group->CheckedButton() == 0) {
        m_result.server_dest = "HOST GAME SELECTED";
    } else {
        m_result.server_dest = *m_IP_address_edit;
        if (m_result.server_dest.empty()) {
            const auto& sels{m_servers_lb->Selections()};
            if (!sels.empty()) {
                if (const auto* sel_row{(*sels.begin())->get()})
                    if (!sel_row->empty())
                        if (const auto* label = dynamic_cast<const GG::Label*>(sel_row->at(0)))
                             m_result.server_dest = label->Text();
            }
        }
    }
    CUIWnd::CloseClicked();
}

void ServerConnectWnd::EnableDisableControls() {
    bool host_selected = m_host_or_join_radio_group->CheckedButton() == 0;
    m_client_type_list->Disable(host_selected);
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
