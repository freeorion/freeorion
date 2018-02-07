#ifndef _ServerConnectWnd_h_
#define _ServerConnectWnd_h_

#include <string>
#include <GG/GGFwd.h>
#include <GG/ListBox.h>

#include "CUIWnd.h"
#include "../network/Networking.h"

/** server connections window */
class ServerConnectWnd : public CUIWnd
{
public:
    /** Connection parameters */
    struct Result {
        std::string player_name;
        std::string server_dest;
        Networking::ClientType type;
    };

    /** \name Structors */ //@{
    ServerConnectWnd();
    void CompleteConstruction() override;
    //@}

    //! \name Mutators
    //!@{
    void ModalInit() override;

    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    //!@}

    /** \name Accessors */ //@{
    /** returns a the player's name (.player_name); the location of the server (.server_dest -- IP address or name), or "" if none was selected and client type (.type) */
    const Result& GetResult() const;
    //@}

protected:
    GG::Rect CalculatePosition() const override;

private:
    void PopulateServerList();
    void ServerSelected(const GG::ListBox::SelectionSet& selections);
    void IPAddressEdited(const std::string& str);
    void OkClicked();
    void CancelClicked() {CUIWnd::CloseClicked();}
    void EnableDisableControls();

    Result m_result;

    std::shared_ptr<GG::RadioButtonGroup>               m_host_or_join_radio_group;
    std::shared_ptr<GG::DropDownList>                   m_client_type_list;
    std::shared_ptr<GG::Label>                          m_LAN_game_label;
    std::shared_ptr<GG::ListBox>                        m_servers_lb;
    std::shared_ptr<GG::Button>                         m_find_LAN_servers_bn;
    std::shared_ptr<GG::Label>                          m_internet_game_label;
    std::shared_ptr<GG::Edit>                           m_IP_address_edit;
    std::shared_ptr<GG::Edit>                           m_player_name_edit;
    std::shared_ptr<GG::Button>                         m_ok_bn;
    std::shared_ptr<GG::Button>                         m_cancel_bn;
};

#endif // _ServerConnectWnd_h_
