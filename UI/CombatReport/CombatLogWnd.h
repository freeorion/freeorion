#ifndef COMBATLOGWND_H
#define COMBATLOGWND_H

#include "../CUIControls.h"
#include <boost/scoped_ptr.hpp>

/// Display a log of combat events with expandable log sections with more details of a combat event
class CombatLogWnd : public GG::Wnd {
public:
    CombatLogWnd(GG::X w, GG::Y h);

    virtual ~CombatLogWnd();

    /** \name Accessors */ ///@{
    GG::Pt ClientUpperLeft() const override;

    GG::Pt ClientLowerRight() const override;

    GG::Pt MinUsableSize() const override;
    //@}

    /** \name Mutators */ //@{
    void PreRender() override;

    void SetFont(boost::shared_ptr<GG::Font> font);
    /// Set which log to show
    void SetLog(int log_id);
    //@}

    ///link clicked signals: first string is the link type, second string is the specific item clicked
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;
    mutable boost::signals2::signal<void ()> WndChangedSignal;

    /* The window may have becomem visible.*/
    void HandleMadeVisible();

    class CombatLogWndImpl;
private:
    //TODO C++11 unique_ptr
    friend class CombatLogWndImpl;
    const boost::scoped_ptr<CombatLogWndImpl> pimpl;

    /// The number of pixels to leave between the text and the frame.
    static const int MARGIN = 5;

    };

#endif // COMBATLOGWND_H
