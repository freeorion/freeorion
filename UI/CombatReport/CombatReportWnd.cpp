#include "CombatReportWnd.h"
#include "../ClientUI.h"
#include "../CUIControls.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"

#include "CombatReportData.h"
#include "GraphicalSummary.h"
#include "CombatLogWnd.h"

#include <GG/TabWnd.h>

namespace {
    GG::X COMBAT_LOG_WIDTH(400);
    GG::Y COMBAT_LOG_HEIGHT(300);
    GG::X RESIZE_MARGIN_X(4);
    GG::Y RESIZE_MARGIN_Y(4);
}

// The implementation class for CombatReportWnd
class CombatReportWnd::CombatReportPrivate {
public:
    CombatReportPrivate (CombatReportWnd& wnd):
        m_wnd(wnd),
        m_tabs(new GG::TabWnd(GG::X0, GG::Y0, GG::X1, GG::Y1, ClientUI::GetFont(), ClientUI::CtrlColor(), ClientUI::TextColor())),
        m_graphical(new GraphicalSummaryWnd()),
        m_log(new CombatLogWnd())
    {
        m_tabs->AddWnd(m_graphical, UserString("COMBAT_SUMMARY"));
        m_tabs->AddWnd(m_log, UserString("COMBAT_LOG"));
        m_wnd.AttachChild(m_tabs);
        m_wnd.GridLayout();
    }

    void SetLog(int log_id) {
        m_graphical->SetLog(log_id);
        m_log->SetLog(log_id);
    }

    void DoLayout()
    { m_graphical->DoLayout(); }

private:
    CombatReportWnd&        m_wnd;
    GG::TabWnd*             m_tabs;
    GraphicalSummaryWnd*    m_graphical;//< Graphical summary
    CombatLogWnd*           m_log;      //< Detailed log

    void SetFocus(int id)
    { DebugLogger() << "SetFocus " << id; }

    void RectangleEnter(int data) {
        DebugLogger() << "RectangleHover " << data;
        SetFocus(data);
    }
};

CombatReportWnd::CombatReportWnd () :
    CUIWnd(UserString("COMBAT_REPORT_TITLE"), GG::X(150), GG::Y(50), COMBAT_LOG_WIDTH, COMBAT_LOG_HEIGHT,
           GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP | CLOSABLE),
    m_impl(new CombatReportPrivate(*this) )
{ }

CombatReportWnd::~CombatReportWnd()
{}

void CombatReportWnd::SetLog(int log_id)
{ m_impl->SetLog(log_id); }

GG::Pt CombatReportWnd::ClientLowerRight() const {
    GG::Pt lr = CUIWnd::ClientLowerRight();
    lr.x -= RESIZE_MARGIN_X;
    lr.y -= RESIZE_MARGIN_Y;
    return lr;
}

void CombatReportWnd::CloseClicked()
{ Hide(); }

void CombatReportWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    m_impl->DoLayout();
}
