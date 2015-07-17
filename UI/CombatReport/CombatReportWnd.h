#ifndef COMBATREPORTDLG_H
#define COMBATREPORTDLG_H

#include "../CUIWnd.h"

#include <boost/scoped_ptr.hpp>

/// Shows a report on a combat
class CombatReportWnd : public CUIWnd {
public:
    CombatReportWnd(GG::X default_x, GG::Y default_y,
                    GG::X default_w, GG::Y default_h,
                    const std::string& config_name = "");
    // Must have explicit destructor since CombatReportPrivate is incomplete here
    virtual ~CombatReportWnd();

    /// Sets which combat to show.
    void            SetLog(int log_id);
    virtual void    CloseClicked();
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

private:
    class CombatReportPrivate;

    boost::scoped_ptr<CombatReportPrivate> m_impl;
};

#endif // COMBATREPORTDLG_H
