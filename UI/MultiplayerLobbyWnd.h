// -*- C++ -*-
#ifndef _MultiplayerLobbyWnd_h_
#define _MultiplayerLobbyWnd_h_

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif

#ifndef _GalaxySetupWnd_h_
#include "GalaxySetupWnd.h"
#endif

#include "../util/MultiplayerCommon.h"

#include <vector>

class CUIButton;
class CUIDropDownList;
class CUIEdit;
class CUIListBox;
class CUIMultiEdit;
class Message;
namespace GG {
class StaticGraphic;
class TextControl;
}

/** multiplayer lobby window */
class MultiplayerLobbyWnd : public CUIWnd
{
public:
    /** \name Structors */ //@{
    MultiplayerLobbyWnd(bool host,
                        const CUIButton::ClickedSignalType::slot_type& start_game_callback,
                        const CUIButton::ClickedSignalType::slot_type& cancel_callback);
    //@}

    /** \name Accessors */ //@{
    bool            LoadGameSelected() const;
    //@}
    
    /** \name Mutators */ //@{
    virtual void    Render();
    virtual void    KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    void            ChatMessage(int player_id, const std::string& msg);
    void            LobbyUpdate(const MultiplayerLobbyData& lobby_data);
    void            LobbyExit(int player_id);
    //@}

private:
    void            Init();
    void            NewLoadClicked(std::size_t idx);
    void            GalaxySetupPanelChanged();
    void            SaveGameChanged(GG::DropDownList::iterator it);
    void            PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image);
    void            PlayerDataChanged();
    bool            PopulatePlayerList();
    void            SendUpdate();
    bool            PlayerDataAcceptable() const;
    bool            CanStart() const;

    MultiplayerLobbyData    m_lobby_data; // a copy of the most recently received lobby update

    bool                    m_host;

    CUIMultiEdit*           m_chat_box;
    CUIEdit*                m_chat_input_edit;
    GG::RadioButtonGroup*   m_new_load_game_buttons;
    GalaxySetupPanel*       m_galaxy_setup_panel;
    CUIDropDownList*        m_saved_games_list;
    GG::StaticGraphic*      m_preview_image;
    GG::TextControl*        m_players_lb_player_name_column_label;
    GG::TextControl*        m_players_lb_empire_name_column_label;
    GG::TextControl*        m_players_lb_empire_colour_column_label;
    GG::TextControl*        m_players_lb_empire_original_name_label;
    CUIListBox*             m_players_lb;
    CUIButton*              m_start_game_bn;
    CUIButton*              m_cancel_bn;
    GG::TextControl*        m_start_conditions_text;
};

#endif // _MultiplayerLobbyWnd_h_
