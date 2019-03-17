#ifndef _GovernmentWnd_h_
#define _GovernmentWnd_h_

#include "CUIWnd.h"

/** Lets the player design ships */
class GovernmentWnd : public CUIWnd {
public:
    //! \name Structors //!@{
    GovernmentWnd(const std::string& config_name = "");
    void CompleteConstruction() override;
    //!@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Reset();
    void Sanitize();
    void Refresh();

    /** Enables, or disables if \a enable is false, issuing orders via this DesignWnd. */
    void EnableOrderIssuing(bool enable = true);
    //@}

    mutable boost::signals2::signal<void ()> ClosingSignal;

private:
    class PolicyPalette;    // shows policies that can be clicked for detailed or dragged on slots in government
    class MainPanel;        // shows slots and adopted policies

    void CloseClicked() override;
    void DoLayout();

    std::shared_ptr<PolicyPalette>  m_policy_palette = nullptr;
    std::shared_ptr<MainPanel>      m_main_panel = nullptr;
};

#endif // _GovernmentWnd_h_
