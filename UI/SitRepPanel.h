#ifndef _SitRepPanel_h_
#define _SitRepPanel_h_

#include <GG/GGFwd.h>
#include <GG/ListBox.h>
#include "CUIWnd.h"

class SitRepEntry;


class SitRepPanel : public CUIWnd {
public:
    SitRepPanel(std::string_view config_name = "");
    void CompleteConstruction() override;

    std::set<std::string>   HiddenSitRepTemplates() const { return m_hidden_sitrep_templates; }
    int                     NumVisibleSitrepsThisTurn() const;

    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Update(); ///< loads all the relevant SitReps into the window

    void ShowSitRepsForTurn(int turn);
    void SetHiddenSitRepTemplates(const std::set<std::string>& templates);

    mutable boost::signals2::signal<void ()> ClosingSignal;

private:
    void CloseClicked() override;

    void PrevClicked();
    void NextClicked();
    void LastClicked();
    void FilterClicked();
    void IgnoreSitRep(GG::ListBox::iterator it, const GG::Pt& pt,
                      const GG::Flags<GG::ModKey>& mod);
    void DismissalMenu(GG::ListBox::iterator it, const GG::Pt& pt,
                       const GG::Flags<GG::ModKey>& mod);
    void DoLayout();

    /** Return true iff the \a sitrep is not hidden, validates and is not snoozed. */
    bool IsSitRepInvalid(const SitRepEntry& sitrep) const;

    /** Search forward (if \a forward is true) or backward from \a turn for the next
      * turn with one or more valid sitreps (not including \a turn itself). */
    int GetNextNonEmptySitrepsTurn(const std::map<int, std::vector<SitRepEntry>>& turns,
                                   int turn, bool forward) const;

    std::shared_ptr<GG::ListBox>    m_sitreps_lb;
    std::shared_ptr<GG::Button>     m_prev_turn_button;
    std::shared_ptr<GG::Button>     m_next_turn_button;
    std::shared_ptr<GG::Button>     m_last_turn_button;
    std::shared_ptr<GG::Button>     m_filter_button;
    int                             m_showing_turn = 0;
    std::set<std::string>           m_hidden_sitrep_templates;
};


#endif
