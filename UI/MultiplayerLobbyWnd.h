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
    MultiplayerLobbyWnd(bool host);
    virtual ~MultiplayerLobbyWnd();
    //@}
    
    /** \name Accessors */ //@{
    bool Result() const {return m_result;}  ///< returns true iff a new game was successfully launched from the lobby
    bool LoadSelected() const; ///< returns true iff a load game is selected (as opposed to a new one)
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void KeyPress(GG::Key key, Uint32 key_mods);

    void         HandleMessage(const Message& msg);
    void         Cancel() {CancelClicked();}
    //@}

private:
    void Init();
    void NewLoadClicked(int idx);
    void GalaxySetupPanelChanged();
    void SaveGameChanged(int idx);
    void PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image);
    void PlayerDataChanged();
    void StartGameClicked();
    void CancelClicked();
    bool PopulatePlayerList();
    void SendUpdate();
    bool PlayerDataAcceptable() const;
    bool CanStart() const;

    bool m_result;

    MultiplayerLobbyData       m_lobby_data; // a copy of the most recently received lobby update
    bool                       m_handling_lobby_update;

    bool                       m_host;
    std::map<std::string, int> m_player_IDs;
    std::map<int, std::string> m_player_names;

    CUIMultiEdit*              m_chat_box;
    CUIEdit*                   m_chat_input_edit;
    GG::RadioButtonGroup*      m_new_load_game_buttons;
    GalaxySetupPanel*          m_galaxy_setup_panel;
    CUIDropDownList*           m_saved_games_list;
    GG::StaticGraphic*         m_preview_image;
    CUIListBox*                m_players_lb;
    CUIButton*                 m_start_game_bn;
    CUIButton*                 m_cancel_bn;
    GG::TextControl*           m_start_conditions_text;
};

#endif // _MultiplayerLobbyWnd_h_
