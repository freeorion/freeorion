#ifndef _MultiPlayerLobbyWnd_h_
#define _MultiPlayerLobbyWnd_h_

#include <vector>
#include <GG/GGFwd.h>

#include "CUIWnd.h"
#include "GalaxySetupWnd.h"
#include "../util/MultiplayerCommon.h"

class Message;
struct PlayerLabelRow;


/** multiplayer lobby window */
class MultiPlayerLobbyWnd : public CUIWnd {
public:
    /** \name Structors */ //@{
    MultiPlayerLobbyWnd();
    void CompleteConstruction() override;
    //@}

    /** \name Accessors */ //@{
    GG::Pt MinUsableSize() const override;

    bool            LoadGameSelected() const;
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Render() override;

    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void            ChatMessage(int player_id, const std::string& msg);
    void            LobbyUpdate(const MultiplayerLobbyData& lobby_data);
    void            Refresh();
    //@}

protected:
    struct PlayerLabelRow : GG::ListBox::Row {
        PlayerLabelRow(GG::X width = GG::X(600));

        void CompleteConstruction() override;

        /** Set text of control at @p column to @p str */
        void SetText(size_t column, const std::string& str);

        void Render() override;
    };

    GG::Rect CalculatePosition() const override;

private:
    void            DoLayout();
    void            NewLoadClicked(std::size_t idx);
    void            GalaxySetupPanelChanged();
    void            SaveGameBrowse();
    void PreviewImageChanged(std::shared_ptr<GG::Texture> new_image);
    void            PlayerDataChangedLocally();
    bool            PopulatePlayerList();   ///< repopulate list with rows built from current m_lobby_data.  returns true iff something in the lobby data was changed during population and an update should be sent back to the server
    void            SendUpdate();
    bool            PlayerDataAcceptable() const;
    bool            CanStart() const;
    bool            ThisClientIsHost() const;
    void            ReadyClicked();
    void            CancelClicked();

    MultiplayerLobbyData    m_lobby_data;   ///< a copy of the most recently received lobby update

    GG::MultiEdit*          m_chat_box;
    GG::Edit*               m_chat_input_edit;
    GG::RadioButtonGroup*   m_new_load_game_buttons;
    GalaxySetupPanel*       m_galaxy_setup_panel;
    GG::Label*              m_save_file_text;
    GG::Button*             m_browse_saves_btn;
    GG::StaticGraphic*      m_preview_image;
    GG::ListBox*            m_players_lb;
    PlayerLabelRow*         m_players_lb_headers;
    GG::Button*             m_ready_bn;
    GG::Button*             m_cancel_bn;
    GG::Label*              m_start_conditions_text;
};

#endif // _MultiPlayerLobbyWnd_h_
