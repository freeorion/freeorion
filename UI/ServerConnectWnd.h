// -*- C++ -*-
#ifndef _ServerConnectWnd_h_
#define _ServerConnectWnd_h_

#include "CUIWnd.h"
#include "../network/ClientNetworking.h"

#include <string>


class CUIButton;
class CUIListBox;
class CUIEdit;
namespace GG {
    class RadioButtonGroup;
    class TextControl;
}

/** server connections window */
class ServerConnectWnd : public CUIWnd
{
public:
    /** \name Structors */ //@{
    ServerConnectWnd();
    //@}

    //! \name Mutators
    //!@{
    virtual void ModalInit ();
    virtual void KeyPress (GG::Key key, GG::Flags<GG::ModKey> mod_keys);
    //!@}

    /** \name Accessors */ //@{
    /** returns a the player's name (.first) and the location of the server (.second -- IP address or name), or "" if none was selected */
    const std::pair<std::string, std::string>& Result() const;
    //@}


private:
    void Init();
    void PopulateServerList();
    void RefreshServerList();
    void HostOrJoinClicked(int idx);
    void ServerSelected(const std::set<int>& selections);
    void IPAddressEdited(const std::string& str);
    void NameEdited(const std::string& str);
    void OkClicked();
    void CancelClicked() {CUIWnd::CloseClicked();}
    void EnableDisableControls();

    std::pair<std::string, std::string> m_result;

    GG::RadioButtonGroup*               m_host_or_join_radio_group;
    GG::TextControl*                    m_LAN_game_label;
    CUIListBox*                         m_servers_lb;
    CUIButton*                          m_find_LAN_servers_bn;
    GG::TextControl*                    m_internet_game_label;
    CUIEdit*                            m_IP_address_edit;
    CUIEdit*                            m_player_name_edit;
    CUIButton*                          m_ok_bn;
    CUIButton*                          m_cancel_bn;

    ClientNetworking::ServerList        m_LAN_servers;
};

#endif // _ServerConnectWnd_h_
