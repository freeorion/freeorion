// -*- C++ -*-
#ifndef _SitRepPanel_h_
#define _SitRepPanel_h_

#include "CUIWnd.h"

namespace GG {
    class Button;
    class ListBox;
}


class SitRepPanel : public CUIWnd {
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ClosingSignalType;   ///< emitted when the window is manually closed by user by clicking on the sitrep panel itself
    //@}

    /** \name Structors */ //@{
    SitRepPanel(GG::X x, GG::Y y, GG::X w, GG::Y h); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    std::set<std::string>   HiddenSitRepTemplates() const { return m_hidden_sitrep_templates; }
    //@}

    /** \name Mutators */ //@{
    virtual void    KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void            Update(); ///< loads all the relevant SitReps into the window

    virtual void    OnClose();
    void            ShowSitRepsForTurn(int turn);
    void            SetHiddenSitRepTemplates(const std::set<std::string>& templates);
    //@}

    mutable ClosingSignalType ClosingSignal;

private:
    virtual void    CloseClicked();
    void            PrevClicked();
    void            NextClicked();
    void            LastClicked();
    void            FilterClicked();

    void            DoLayout();

    GG::ListBox*            m_sitreps_lb;
    GG::Button*             m_prev_turn_button;
    GG::Button*             m_next_turn_button;
    GG::Button*             m_last_turn_button;
    GG::Button*             m_filter_button;

    int                     m_showing_turn;
    std::set<std::string>   m_hidden_sitrep_templates;
};

#endif // _SitRepPanel_h_
