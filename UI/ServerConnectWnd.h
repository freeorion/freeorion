#ifndef _ServerConnectWnd_h_
#define _ServerConnectWnd_h_

#include <string>
#include <GG/GGFwd.h>
#include <GG/ListBox.h>

#include "CUIWnd.h"


/** server connections window */
class ServerConnectWnd : public CUIWnd
{
public:
    /** \name Structors */ //@{
    ServerConnectWnd();
    //@}

    //! \name Mutators
    //!@{
    void ModalInit() override;

    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    //!@}

    /** \name Accessors */ //@{
    /** returns a the player's name (.first) and the location of the server (.second -- IP address or name), or "" if none was selected */
    const std::pair<std::string, std::string>& Result() const;
    //@}

protected:
    GG::Rect CalculatePosition() const override;

private:
    void Init();
    void PopulateServerList();
    void ServerSelected(const GG::ListBox::SelectionSet& selections);
    void IPAddressEdited(const std::string& str);
    void OkClicked();
    void CancelClicked() {CUIWnd::CloseClicked();}
    void EnableDisableControls();

    std::pair<std::string, std::string> m_result;

    GG::RadioButtonGroup*               m_host_or_join_radio_group;
    GG::Label*                          m_LAN_game_label;
    GG::ListBox*                        m_servers_lb;
    GG::Button*                         m_find_LAN_servers_bn;
    GG::Label*                          m_internet_game_label;
    GG::Edit*                           m_IP_address_edit;
    GG::Edit*                           m_player_name_edit;
    GG::Button*                         m_ok_bn;
    GG::Button*                         m_cancel_bn;
};

#endif // _ServerConnectWnd_h_
