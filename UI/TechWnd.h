// -*- C++ -*-
#ifndef _TechWnd_h_
#define _TechWnd_h_

#include "CUI_Wnd.h"

class CUIScroll;
class Tech;

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
    virtual GG::Pt ClientLowerRight() const;

    const std::string& CategoryShown() const;
    TechTypesShown     GetTechTypesShown() const;

    TechBrowsedSignalType&       TechBrowsedSignal() const       {return m_tech_browsed_sig;}
    TechClickedSignalType&       TechClickedSignal() const       {return m_tech_clicked_sig;}
    TechDoubleClickedSignalType& TechDoubleClickedSignal() const {return m_tech_double_clicked_sig;}
    //@}

    //! \name Mutators //@{
    virtual bool Render();
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys);

    void ShowCategory(const std::string& category);
    void SetTechTypesShown(TechTypesShown tech_types);
    void UncollapseAll();
    //@}

private:
    class TechPanel;
    struct CollapseSubtreeFunctor;
    typedef std::multimap<const Tech*,
                          std::pair<const Tech*,
                                    std::vector<std::vector<std::pair<double, double> > > > > DependencyArcsMap;

    void Layout(bool keep_position);
    bool TechVisible(const Tech* tech);
    void CollapseTechSubtree(const Tech* tech, bool collapse);
    void DrawArc(DependencyArcsMap::const_iterator it, GG::Clr color, bool with_arrow_head);
    void ScrolledSlot(int, int, int, int);
    void TechBrowsedSlot(const Tech* t);
    void TechClickedSlot(const Tech* t);
    void TechDoubleClickedSlot(const Tech* t);

    std::string    m_category_shown;
    TechTypesShown m_tech_types_shown;
    const Tech*    m_selected_tech;
    
    // indexed by category-view (including "ALL"), the techs whose subtrees are desired collapsed
    std::map<std::string, std::set<const Tech*> > m_collapsed_subtree_techs_per_view;

    std::map<const Tech*, TechPanel*> m_techs;
    DependencyArcsMap m_dependency_arcs;

    CUIScroll*     m_vscroll;
    CUIScroll*     m_hscroll;
    GG::Pt         m_scroll_position;

    mutable TechBrowsedSignalType       m_tech_browsed_sig;
    mutable TechClickedSignalType       m_tech_clicked_sig;
    mutable TechDoubleClickedSignalType m_tech_double_clicked_sig;

    friend struct CollapseSubtreeFunctor;
};

class TechWnd : public CUI_Wnd
{
public:
    /** \name Structors */ //@{
    TechWnd();
    //@}

private:
    
};

#endif // _TechWnd_h_
