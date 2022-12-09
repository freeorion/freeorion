#ifndef COMBATREPORTDLG_H
#define COMBATREPORTDLG_H

#include "../CUIWnd.h"

#include <memory>


/// Shows a report on a combat
class CombatReportWnd final : public CUIWnd {
public:
    CombatReportWnd(std::string_view config_name = "");
    void CompleteConstruction() override;
    virtual ~CombatReportWnd(); // Must have explicit destructor since Impl is incomplete here

    void CloseClicked() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void SetLog(int log_id);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};


#endif
