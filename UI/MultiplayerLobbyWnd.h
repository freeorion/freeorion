// -*- C++ -*-
#ifndef _MultiplayerLobbyWnd_h_
#define _MultiplayerLobbyWnd_h_

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

#include <vector>

class CUIButton;
class CUIDropDownList;
class CUIEdit;
class CUIListBox;
class CUIMultiEdit;
class Message;
namespace GG {
class RadioButtonGroup;
class StaticGraphic;
class TextControl;
}

/** multiplayer lobby window */
class MultiplayerLobbyWnd : public CUI_Wnd
{
public:
    /** \name Structors */ //@{
    MultiplayerLobbyWnd(bool host);
    MultiplayerLobbyWnd(const GG::XMLElement& elem);
    virtual ~MultiplayerLobbyWnd();
    //@}
    
    /** \name Accessors */ //@{
    bool Result() const {return m_result;}  ///< returns true iff a new game was successfully launched from the lobby
    //@}

    /** \name Mutators */ //@{
    virtual bool Render();
    virtual void Keypress(GG::Key key, Uint32 key_mods);
    void         HandleMessage(const Message& msg);
    //@}

private:
    void Init();
    void AttachSignalChildren();
    void DetachSignalChildren();
    void GalaxySizeClicked(int idx);
    void GalaxyTypeClicked(int idx);
    void BrowseClicked();
    void PlayerSelected(const std::set<int>& selections);
    void StartGameClicked();
    void CancelClicked();
    void DisableControls();
    GG::XMLDoc LobbyUpdateDoc() const;

    bool m_result;

    bool m_host;
    std::map<std::string, int> m_player_IDs;
    std::map<int, std::string> m_player_names;

    CUIMultiEdit*         m_chat_box;
    CUIEdit*              m_chat_input_edit;
    GG::RadioButtonGroup* m_galaxy_size_buttons;
    GG::RadioButtonGroup* m_galaxy_type_buttons;
    GG::StaticGraphic*    m_galaxy_preview_image;
    CUIEdit*              m_galaxy_image_file_edit;
    CUIButton*            m_image_file_browse_bn;
    CUIListBox*           m_players_lb;
    CUIButton*            m_start_game_bn;
    CUIButton*            m_cancel_bn;

    std::vector<boost::shared_ptr<GG::Texture> > m_textures; //!< textures for galaxy previews
};

#endif // _MultiplayerLobbyWnd_h_
