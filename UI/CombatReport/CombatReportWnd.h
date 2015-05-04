#ifndef COMBATREPORTDLG_H
#define COMBATREPORTDLG_H

#include "../UI/CUIWnd.h"

#include <boost/scoped_ptr.hpp>

/// Shows a report on a combat
class CombatReportWnd : public CUIWnd {
public:
    CombatReportWnd();
    // Must have explicit destructor since CombatReportPrivate is incomplete here
    virtual ~CombatReportWnd();

    /// Sets which combat to show.
    void            SetLog(int log_id);
    virtual void    CloseClicked();
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

protected:
    /// Filter LMB events over the resize tab that are sent to the GraphicalSummaryWnd instead of this wnd.
    virtual bool EventFilter(GG::Wnd* w, const GG::WndEvent& wnd_event);

private:
    class CombatReportPrivate;

    boost::scoped_ptr<CombatReportPrivate> m_impl;
};

#endif // COMBATREPORTDLG_H
