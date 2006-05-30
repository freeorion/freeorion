// -*- C++ -*-
#ifndef _TechTreeWnd_h_
#define _TechTreeWnd_h_

#include "CUIWnd.h"

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
        ALL_TECH_TYPES
    };

    enum TechStatusesShown {
        RESEARCHABLE_TECHS,
        COMPLETE_AND_RESEARCHABLE_TECHS,
        ALL_TECH_STATUSES
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
    double             Scale() const;
    const std::string& CategoryShown() const;
    TechTypesShown     GetTechTypesShown() const;
    TechStatusesShown  GetTechStatusesShown() const;
    //@}

    //! \name Mutators //@{
    void Update();
    void Clear();
    void Reset();
    void SetScale(double scale);
    void ShowCategory(const std::string& category);
    void SetTechTypesShown(TechTypesShown tech_types);
    void SetTechStatusesShown(TechStatusesShown tech_statuses);
    void UncollapseAll();
    void CenterOnTech(const Tech* tech);
    //@}

    static const int NAVIGATOR_AND_DETAIL_HEIGHT = 200;

    mutable TechBrowsedSignalType       TechBrowsedSignal;
    mutable TechClickedSignalType       TechSelectedSignal;
    mutable TechDoubleClickedSignalType AddTechToQueueSignal;

private:
    class TechDetailPanel;
    class TechNavigator;
    class LayoutPanel;

    void TechBrowsedSlot(const Tech* tech);
    void TechClickedSlot(const Tech* tech);
    void TechDoubleClickedSlot(const Tech* tech);
    void TechTypesShownSlot(int types);
    void TechStatusesShownSlot(int statuses);

    std::vector<CUIButton*> m_category_buttons;
    TechDetailPanel*        m_tech_detail_panel;
    TechNavigator*          m_tech_navigator;
    LayoutPanel*            m_layout_panel;
    GG::RadioButtonGroup*   m_tech_type_buttons;
    GG::RadioButtonGroup*   m_tech_status_buttons;
    CUIButton*              m_uncollapse_all_button;
};

#endif // _TechTreeWnd_h_
