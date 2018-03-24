#ifndef _GovernmentWnd_h_
#define _GovernmentWnd_h_

#include <GG/Wnd.h>

/** Lets the player design ships */
class GovernmentWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    GovernmentWnd(GG::X w, GG::Y h);
    //@}
    void CompleteConstruction() override;

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void Render() override;

    void Reset();
    void Sanitize();
    void Refresh();

    /** Enables, or disables if \a enable is false, issuing orders via this DesignWnd. */
    void EnableOrderIssuing(bool enable = true);
    //@}

private:
    class PolicyPalette;    // shows policies that can be clicked for detailed or dragged on slots in government
    class MainPanel;        // shows slots and adopted policies

    void InitializeWindows();

    std::shared_ptr<PolicyPalette>  m_policy_palette;
    std::shared_ptr<MainPanel>      m_main_panel;
};

#endif // _GovernmentWnd_h_
