#ifndef COMBATLOGWND_H
#define COMBATLOGWND_H

#include "../CUIControls.h"

/// Display a log of combat events with expandable log sections with more details of a combat event
class CombatLogWnd : public GG::Wnd {
public:
    CombatLogWnd(GG::X w, GG::Y h);

    /** \name Accessors */ ///@{
    virtual GG::Pt ClientUpperLeft() const;
    virtual GG::Pt ClientLowerRight() const;
    virtual GG::Pt MinUsableSize() const;
    //@}

    /** \name Mutators */ //@{
    void SetFont(boost::shared_ptr<GG::Font> font);
    /// Set which log to show
    void SetLog(int log_id);
    //@}

    ///link clicked signals: first string is the link type, second string is the specific item clicked
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;

    /**These handlers just echo the signals from contained CUILinkTextMultiEdit objects to the above signals*/
    void HandleLinkClick(const std::string& link_type, const std::string& data);
    void HandleLinkDoubleClick(const std::string& link_type, const std::string& data);
    void HandleLinkRightClick(const std::string& link_type, const std::string& data);

    /** DecorateLinkText creates a CUILinkTextMultiEdit using \a text and attaches it to handlers
        \a and_flags are anded to the default flags. */
    LinkText * DecorateLinkText(std::string const & text);

private:
    /** Add a row at the end of the combat report*/
    void AddRow(GG::Wnd * wnd);

    /// The number of pixels to leave between the text and the frame.
    static const int MARGIN = 5;

    ///default flags for a text link log segment
    GG::Flags<GG::TextFormat> m_text_format_flags;
    boost::shared_ptr<GG::Font> m_font;

    };

#endif // COMBATLOGWND_H
