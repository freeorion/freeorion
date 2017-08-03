#ifndef COMBATREPORTDLG_H
#define COMBATREPORTDLG_H

#include "../CUIWnd.h"

#include <memory>


/// Shows a report on a combat
class CombatReportWnd : public CUIWnd {
public:
    CombatReportWnd(const std::string& config_name = "");
    void CompleteConstruction() override;
    // Must have explicit destructor since Impl is incomplete here
    virtual ~CombatReportWnd();

    void CloseClicked() override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    /// Sets which combat to show.
    void            SetLog(int log_id);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif // COMBATREPORTDLG_H
