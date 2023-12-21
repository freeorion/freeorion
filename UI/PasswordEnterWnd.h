#ifndef _PasswordEnterDialog_h_
#define _PasswordEnterDialog_h_

#include <GG/Layout.h>

#include "CUIWnd.h"
#include "CUIControls.h"

class PasswordEnterWnd : public CUIWnd {
public:
    PasswordEnterWnd();
    void CompleteConstruction() override;

    void ModalInit() override;

    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void SetPlayerName(const std::string& player_name);

protected:
    GG::Rect CalculatePosition() const override;

private:
    void OkClicked();
    void CancelClicked();

    std::shared_ptr<CUIEdit>            m_player_name_edit;
    std::shared_ptr<CensoredCUIEdit>    m_password_edit;
    std::shared_ptr<CUIButton>          m_ok_bn;
    std::shared_ptr<CUIButton>          m_cancel_bn;
};


#endif
