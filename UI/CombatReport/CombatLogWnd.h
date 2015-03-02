#ifndef COMBATLOGWND_H
#define COMBATLOGWND_H

#include "../CUIControls.h"

/// Displays a textual log of combat events
class CombatLogWnd : public CUILinkTextMultiEdit {
public:
    CombatLogWnd();

    /// Set which log to show
    void SetLog(int log_id);
};

#endif // COMBATLOGWND_H
