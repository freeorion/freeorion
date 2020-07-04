#ifndef _PlayerListWnd_h_
#define _PlayerListWnd_h_

#include "CUIWnd.h"
#include "../network/Message.h"
#include <GG/ListBox.h>

class PlayerListBox;

class PlayerListWnd : public CUIWnd {
public:
    PlayerListWnd(const std::string& config_name);
    void CompleteConstruction() override;

    std::set<int>   SelectedPlayerIDs() const;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

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
    void            LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void            LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) override;

    void            DoLayout();

    void            PlayerSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            PlayerDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void            PlayerRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    int             PlayerInRow(GG::ListBox::iterator it) const;
    int             EmpireInRow(GG::ListBox::iterator it) const;

    std::shared_ptr<PlayerListBox>  m_player_list;
};

#endif
