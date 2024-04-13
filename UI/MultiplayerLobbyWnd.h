#ifndef _MultiPlayerLobbyWnd_h_
#define _MultiPlayerLobbyWnd_h_

#include <vector>
#include <GG/GGFwd.h>

#include "ChatWnd.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "GalaxySetupWnd.h"
#include "../util/MultiplayerCommon.h"

class Message;
struct PlayerLabelRow;


/** multiplayer lobby window */
class MultiPlayerLobbyWnd : public CUIWnd {
public:
    MultiPlayerLobbyWnd();
    void CompleteConstruction() override;

    GG::Pt MinUsableSize() const override;

    bool            LoadGameSelected() const;

    std::string     GetChatText() const;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Render() override;

    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void            ChatMessage(int player_id, const boost::posix_time::ptime& timestamp, const std::string& msg);
    void            ChatMessage(const std::string& message_text,
                                const std::string& player_name,
                                GG::Clr text_color,
                                const boost::posix_time::ptime& timestamp);
    void            TurnPhaseUpdate(Message::TurnProgressPhase phase_id);
    void            LobbyUpdate(const MultiplayerLobbyData& lobby_data);
    void            Refresh();
    void            CleanupChat();

protected:
    struct PlayerLabelRow : GG::ListBox::Row {
        PlayerLabelRow(GG::X width = GG::X(600));

        void CompleteConstruction() override;

        /** Set text of control at @p column to @p str */
        void SetText(std::size_t column, const std::string& str);

        void Render() override;
    };

    GG::Rect CalculatePosition() const override;

private:
    void DoLayout();
    void NewLoadClicked(std::size_t idx);
    void GalaxySetupPanelChanged();
    void SaveGameBrowse();
    void PreviewImageChanged(std::shared_ptr<GG::Texture> new_image);
    void PlayerDataChangedLocally();
    bool PopulatePlayerList();   ///< repopulate list with rows built from current m_lobby_data.  returns true iff something in the lobby data was changed during population and an update should be sent back to the server
    void SendUpdate() const;
    bool PlayerDataAcceptable() const;
    bool CanStart() const;
    bool HasAuthRole(Networking::RoleType role) const;
    void ReadyClicked();
    void CancelClicked();
    void AnyCanEdit(bool checked);

    MultiplayerLobbyData    m_lobby_data;   ///< a copy of the most recently received lobby update

    std::shared_ptr<MessageWnd>             m_chat_wnd;
    std::shared_ptr<CUIStateButton>         m_any_can_edit;
    std::shared_ptr<GG::RadioButtonGroup>   m_new_load_game_buttons;
    std::shared_ptr<GalaxySetupPanel>       m_galaxy_setup_panel;
    std::shared_ptr<GG::Label>              m_save_file_text;
    std::shared_ptr<GG::Button>             m_browse_saves_btn;
    std::shared_ptr<GG::StaticGraphic>      m_preview_image;
    std::shared_ptr<GG::ListBox>            m_players_lb;
    std::shared_ptr<PlayerLabelRow>         m_players_lb_headers;
    std::shared_ptr<GG::Button>             m_ready_bn;
    std::shared_ptr<GG::Button>             m_cancel_bn;
    std::shared_ptr<GG::Label>              m_start_conditions_text;
};


#endif
