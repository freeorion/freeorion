#include "PasswordEnterWnd.h"

#include <GG/GUI.h>

#include "../util/i18n.h"

namespace {
    const GG::X WINDOW_WIDTH(400);
    const GG::Y WINDOW_HEIGHT(535);
}

PasswordEnterWnd::PasswordEnterWnd()
    :CUIWnd(UserString("AUTHENTICATION_WINDOW_TITLE"), GG::X(80), GG::Y(130), WINDOW_WIDTH, WINDOW_HEIGHT,
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL),
    m_player_name_edit(nullptr),
    m_password_edit(nullptr),
    m_ok_bn(nullptr),
    m_cancel_bn(nullptr)
{}

void PasswordEnterWnd::CompleteConstruction() {
    auto auth_desc_label = GG::Wnd::Create<CUILabel>(UserString("AUTHENTICATION_DESC"), GG::FORMAT_LEFT);
    auto player_name_label = GG::Wnd::Create<CUILabel>(UserString("PLAYER_NAME_LABEL"), GG::FORMAT_LEFT);
    m_player_name_edit = GG::Wnd::Create<CUIEdit>("");
    auto password_label = GG::Wnd::Create<CUILabel>(UserString("PASSWORD_LABEL"), GG::FORMAT_LEFT);
    m_password_edit = GG::Wnd::Create<CUIEdit>("");
    m_ok_bn = Wnd::Create<CUIButton>(UserString("OK"));
    m_cancel_bn = Wnd::Create<CUIButton>(UserString("CANCEL"));

    const GG::X OK_CANCEL_BUTTON_WIDTH(100);
    const int CONTROL_MARGIN = 5;

    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 8, 4, CONTROL_MARGIN);
    layout->SetMinimumColumnWidth(0, auth_desc_label->MinUsableSize().x + CONTROL_MARGIN);
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
}

GG::Rect PasswordEnterWnd::CalculatePosition() const {
    GG::Pt new_ul((GG::GUI::GetGUI()->AppWidth() - WINDOW_WIDTH) / 2,
                  (GG::GUI::GetGUI()->AppHeight() - WINDOW_HEIGHT) / 2);
    GG::Pt new_sz(WINDOW_WIDTH, WINDOW_HEIGHT);
    return GG::Rect(new_ul, new_ul + new_sz);
}

void PasswordEnterWnd::ModalInit()
{ GG::GUI::GetGUI()->SetFocusWnd(m_password_edit); }

void PasswordEnterWnd::KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
}

