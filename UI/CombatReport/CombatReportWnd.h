#ifndef COMBATREPORTDLG_H
#define COMBATREPORTDLG_H

#include "../CUIWnd.h"

#include <boost/scoped_ptr.hpp>

/// Shows a report on a combat
class CombatReportWnd : public CUIWnd {
public:
    CombatReportWnd(const std::string& config_name = "");
    // Must have explicit destructor since CombatReportPrivate is incomplete here
    virtual ~CombatReportWnd();

    void CloseClicked() override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    /// Sets which combat to show.
    void            SetLog(int log_id);

private:
    class Impl;

    boost::scoped_ptr<Impl> m_impl;
};

#endif // COMBATREPORTDLG_H
