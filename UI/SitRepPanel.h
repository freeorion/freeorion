#ifndef _SitRepPanel_h_
#define _SitRepPanel_h_

#include <GG/GGFwd.h>
#include <GG/ListBox.h>
#include "CUIWnd.h"

class SitRepEntry;


class SitRepPanel : public CUIWnd {
public:
    /** \name Structors */ //@{
    SitRepPanel(const std::string& config_name = "");
    void CompleteConstruction() override;
    //@}

    /** \name Accessors */ //@{
    std::set<std::string>   HiddenSitRepTemplates() const { return m_hidden_sitrep_templates; }
    int                     NumVisibleSitrepsThisTurn() const;
    //@}

    /** \name Mutators */ //@{
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Update(); ///< loads all the relevant SitReps into the window

    void ShowSitRepsForTurn(int turn);
    void SetHiddenSitRepTemplates(const std::set<std::string>& templates);
    //@}

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

    bool IsSitRepInvalid(SitRepEntry& sitrep) const;

    /** Search \p forward from \p turn for the next turn with valid sitreps.
      * Remove turns from \p turns that have all invalid or otherwise filtered
      * out sitreps. */
    int GetNextNonEmptySitrepsTurn(std::map<int, std::list<SitRepEntry>>& turns,
                                   int turn, bool forward) const;

    std::shared_ptr<GG::ListBox>    m_sitreps_lb = nullptr;
    std::shared_ptr<GG::Button>     m_prev_turn_button = nullptr;
    std::shared_ptr<GG::Button>     m_next_turn_button = nullptr;
    std::shared_ptr<GG::Button>     m_last_turn_button = nullptr;
    std::shared_ptr<GG::Button>     m_filter_button = nullptr;
    int                             m_showing_turn;
    std::set<std::string>           m_hidden_sitrep_templates;
};

#endif // _SitRepPanel_h_
