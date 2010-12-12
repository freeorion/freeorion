// -*- C++ -*-
#ifndef _PlayerListWnd_h_
#define _PlayerListWnd_h_

#include "CUIWnd.h"
#include "../network/Message.h"

class CUIListBox;

class PlayerListWnd : public CUIWnd
{
public:
    //! \name Structors //@{
    PlayerListWnd(GG::X x, GG::Y y, GG::X w, GG::Y h);
    //@}

    //! \name Mutators //@{
    std::set<int>   SelectedPlayerIDs() const;
    //@}

    //! \name Mutators //@{
    void            HandlePlayerStatusUpdate(Message::PlayerStatus player_status, int about_player_id);
    void            Update();
    void            Refresh();
    void            Clear();

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey>& mod_keys);
    //@}

    mutable boost::signal<void ()>  SelectedPlayersChangedSignal;

private:
    void            DoLayout();

    CUIListBox* m_player_list;
};

#endif
