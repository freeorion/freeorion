#ifndef _PlayerListWnd_h_
#define _PlayerListWnd_h_

#include "CUIWnd.h"
#include "../network/Message.h"
#include <GG/ListBox.h>

class PlayerListBox;

class PlayerListWnd : public CUIWnd {
public:
    PlayerListWnd(std::string_view config_name);
    void CompleteConstruction() override;

    std::set<int>   SelectedPlayerIDs() const;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void            HandleDiplomaticMessageChange(int empire1_id, int empire2_id);
    void            Update();
    void            Refresh();
    void            Clear();

    void            SetSelectedPlayers(const std::set<int>& player_ids);

    mutable boost::signals2::signal<void ()>    SelectedPlayersChangedSignal;
    mutable boost::signals2::signal<void (int)> PlayerDoubleClickedSignal;
    mutable boost::signals2::signal<void ()>    ClosingSignal;

private:
    void            CloseClicked() override;
    void            LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void            LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override;

    void            DoLayout();

    void            PlayerSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            PlayerDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void            PlayerRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    int             PlayerInRow(GG::ListBox::iterator it) const;
    int             EmpireInRow(GG::ListBox::iterator it) const;

    std::shared_ptr<PlayerListBox>  m_player_list;
};

#endif
