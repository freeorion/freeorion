#ifndef _GovernmentWnd_h_
#define _GovernmentWnd_h_

#include "CUIWnd.h"

/** Lets the player design ships */
class GovernmentWnd final : public CUIWnd {
public:
    explicit GovernmentWnd(std::string_view config_name = "");
    void CompleteConstruction() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void ClearPolicies();
    void RevertPolicies();
    void Reset();
    void Sanitize();
    void Refresh();

    double GetPolicyZoomFactor() const; // zoom factor for policy cards
    GG::Pt GetPolicySlotSize() const;   // policy slot size with zoom factor applied
    int    GetPolicyTextSize() const;   // policy text size with zoom factor applied

    /** Enables, or disables if \a enable is false, issuing orders via this DesignWnd. */
    void EnableOrderIssuing(bool enable = true);

    mutable boost::signals2::signal<void ()> ClosingSignal;

private:
    class PolicyPalette;    // shows policies that can be clicked for detailed or dragged on slots in government
    class MainPanel;        // shows slots and adopted policies

    void CloseClicked() override;
    void DoLayout();
    void PolicySizeButtonClicked(std::size_t idx);

    std::shared_ptr<PolicyPalette>           m_policy_palette;
    std::shared_ptr<MainPanel>               m_main_panel;
    std::shared_ptr<GG::RadioButtonGroup>    m_policy_size_buttons;
    std::shared_ptr<GG::Button>              m_revert_button;
};


#endif
