#include "PasswordEnterWnd.h"

#include <GG/GUI.h>

#include "../util/i18n.h"
#include "../client/ClientNetworking.h"
#include "../client/human/GGHumanClientApp.h"

namespace {
    constexpr GG::X WINDOW_WIDTH{300};
    constexpr GG::Y WINDOW_HEIGHT{250};
}

PasswordEnterWnd::PasswordEnterWnd() :
    CUIWnd(UserString("AUTHENTICATION_WINDOW_TITLE"), GG::X{80}, GG::Y{130},
           WINDOW_WIDTH, WINDOW_HEIGHT,
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL)
{}

void PasswordEnterWnd::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    auto auth_desc_label = GG::Wnd::Create<CUILabel>(UserString("AUTHENTICATION_DESC"), GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK);
    auto player_name_label = GG::Wnd::Create<CUILabel>(UserString("PLAYER_NAME_LABEL"), GG::FORMAT_LEFT);
    m_player_name_edit = GG::Wnd::Create<CUIEdit>("");
    auto password_label = GG::Wnd::Create<CUILabel>(UserString("PASSWORD_LABEL"), GG::FORMAT_LEFT);
    m_password_edit = GG::Wnd::Create<CensoredCUIEdit>("");
    m_ok_bn = Wnd::Create<CUIButton>(UserString("OK"));
    m_cancel_bn = Wnd::Create<CUIButton>(UserString("CANCEL"));

    static constexpr GG::X OK_CANCEL_BUTTON_WIDTH{100};
    static constexpr int CONTROL_MARGIN = 5;

    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 4, 4, CONTROL_MARGIN);
    layout->SetMinimumColumnWidth(0, std::max(player_name_label->MinUsableSize().x,
                                              password_label->MinUsableSize().x) + CONTROL_MARGIN);
    layout->SetColumnStretch(1, 1.0);
    layout->SetMinimumColumnWidth(2, OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN);
    layout->SetMinimumColumnWidth(3, OK_CANCEL_BUTTON_WIDTH + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(0, auth_desc_label->Height() + (2 * CONTROL_MARGIN));
    layout->SetMinimumRowHeight(1, m_player_name_edit->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(2, m_password_edit->Height() + CONTROL_MARGIN);
    layout->SetMinimumRowHeight(3, m_ok_bn->MinUsableSize().y + CONTROL_MARGIN);

    layout->Add(auth_desc_label, 0, 0, 1, 4, GG::ALIGN_VCENTER);
    layout->Add(player_name_label, 1, 0, 1, 1, GG::ALIGN_VCENTER);
    layout->Add(m_player_name_edit, 1, 1, 1, 3, GG::ALIGN_VCENTER);
    layout->Add(password_label, 2, 0, 1, 1, GG::ALIGN_VCENTER);
    layout->Add(m_password_edit, 2, 1, 1, 3, GG::ALIGN_VCENTER);
    layout->Add(m_ok_bn, 3, 2);
    layout->Add(m_cancel_bn, 3, 3);

    SetLayout(layout);

    ResetDefaultPosition();

    m_ok_bn->LeftClickedSignal.connect(boost::bind(&PasswordEnterWnd::OkClicked, this));
    m_cancel_bn->LeftClickedSignal.connect(boost::bind(&PasswordEnterWnd::CancelClicked, this));
}

GG::Rect PasswordEnterWnd::CalculatePosition() const {
    GG::Pt new_ul((GG::GUI::GetGUI()->AppWidth() - WINDOW_WIDTH) / 2,
                  (GG::GUI::GetGUI()->AppHeight() - WINDOW_HEIGHT) / 2);
    GG::Pt new_sz(WINDOW_WIDTH, WINDOW_HEIGHT);
    return GG::Rect(new_ul, new_ul + new_sz);
}

void PasswordEnterWnd::ModalInit()
{ GG::GUI::GetGUI()->SetFocusWnd(m_password_edit); }

void PasswordEnterWnd::KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (key == GG::Key::GGK_ESCAPE) { // Same behaviour as if "Cancel" was pressed
        CancelClicked();
    } else if (key == GG::Key::GGK_RETURN || key == GG::Key::GGK_KP_ENTER) {
        if (GG::GUI::GetGUI()->FocusWnd() == m_player_name_edit) {
            GG::GUI::GetGUI()->SetFocusWnd(m_password_edit);
        } else if (GG::GUI::GetGUI()->FocusWnd() == m_password_edit) {
            OkClicked();
        }
    }
}

void PasswordEnterWnd::SetPlayerName(const std::string& player_name) {
    m_player_name_edit->SetText(player_name);
    m_password_edit->SetText("");
}

void PasswordEnterWnd::OkClicked() {
    GGHumanClientApp::GetApp()->Networking().SendMessage(
        AuthResponseMessage(*m_player_name_edit, m_password_edit->RawText()));
    // hide window
    GGHumanClientApp::GetApp()->Remove(shared_from_this());
}

void PasswordEnterWnd::CancelClicked()
{ GGHumanClientApp::GetApp()->CancelMultiplayerGameFromLobby(); }

