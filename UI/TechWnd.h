// -*- C++ -*-
#ifndef _TechWnd_h_
#define _TechWnd_h_

#include "CUI_Wnd.h"

class CUIScroll;
class Tech;
namespace GG {class RadioButtonGroup;}

/** Contains the tech graph layout, some controls to control what is visible in the tech layout, the tech navigator, and the tech detail window. */
class TechTreeWnd : public GG::Wnd
{
public:
    enum TechTypesShown {
        THEORY_TECHS,
        APPLICATION_AND_THEORY_TECHS,
        ALL_TECHS
    };

    /** \name Signal Types */ //@{
    typedef boost::signal<void (const Tech*)>      TechBrowsedSignalType;       ///< emitted when a technology is single-clicked
    typedef boost::signal<void (const Tech*)>      TechClickedSignalType;       ///< emitted when the mouse rolls over a technology
    typedef boost::signal<void (const Tech*)>      TechDoubleClickedSignalType; ///< emitted when a technology is double-clicked
    //@}

    /** \name Slot Types */ //@{
    typedef TechBrowsedSignalType::slot_type       TechBrowsedSlotType;       ///< type of functor(s) invoked on a TechBrowsedSignalType
    typedef TechClickedSignalType::slot_type       TechClickedSlotType;       ///< type of functor(s) invoked on a TechClickedSignalType
    typedef TechDoubleClickedSignalType::slot_type TechDoubleClickedSlotType; ///< type of functor(s) invoked on a TechDoubleClickedSignalType
    //@}

    /** \name Structors */ //@{
    TechTreeWnd(int w, int h);
    //@}

    /** \name Accessors */ //@{
    const std::string& CategoryShown() const;
    TechTypesShown     GetTechTypesShown() const;

    TechBrowsedSignalType&       TechBrowsedSignal() const    {return m_tech_browsed_sig;}
    TechClickedSignalType&       TechSelectedSignal() const   {return m_tech_selected_sig;}
    TechDoubleClickedSignalType& AddTechToQueueSignal() const {return m_add_tech_to_queue_sig;}
    //@}

    //! \name Mutators //@{
    void Update();
    void Clear();
    void Reset();
    void ShowCategory(const std::string& category);
    void SetTechTypesShown(TechTypesShown tech_types);
    void UncollapseAll();
    void CenterOnTech(const Tech* tech);
    //@}

private:
    class TechDetailPanel;
    class TechNavigator;
    class LayoutPanel;

    void TechBrowsedSlot(const Tech* tech);
    void TechClickedSlot(const Tech* tech);
    void TechDoubleClickedSlot(const Tech* tech);
    void TechTypesShownSlot(int types);

    std::vector<CUIButton*> m_category_buttons;
    TechDetailPanel*        m_tech_detail_panel;
    TechNavigator*          m_tech_navigator;
    LayoutPanel*            m_layout_panel;
    GG::RadioButtonGroup*   m_tech_type_buttons;
    CUIButton*              m_uncollapse_all_button;

    mutable TechBrowsedSignalType       m_tech_browsed_sig;
    mutable TechClickedSignalType       m_tech_selected_sig;
    mutable TechDoubleClickedSignalType m_add_tech_to_queue_sig;
};

inline std::pair<std::string, std::string> TechWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _TechWnd_h_
