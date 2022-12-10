#ifndef COMBATLOGWND_H
#define COMBATLOGWND_H

#include "../CUIControls.h"

#include <memory>


/// Display a log of combat events with expandable log sections with more details of a combat event
class CombatLogWnd final : public GG::Wnd {
public:
    CombatLogWnd(GG::X w, GG::Y h);

    ~CombatLogWnd();

    GG::Pt ClientUpperLeft() const noexcept override;
    GG::Pt ClientLowerRight() const noexcept override;
    GG::Pt MinUsableSize() const override;

    void PreRender() override;

    void SetFont(std::shared_ptr<GG::Font> font);
    /// Set which log to show
    void SetLog(int log_id);

    ///link clicked signals: first string is the link type, second string is the specific item clicked
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;
    mutable boost::signals2::signal<void ()> WndChangedSignal;

    /* The window may have becomem visible.*/
    void HandleMadeVisible();

    class Impl;

private:
    friend class Impl;
    std::unique_ptr<Impl> const m_impl;
};


#endif
