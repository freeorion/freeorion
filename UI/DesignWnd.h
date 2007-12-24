// -*- C++ -*-
#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include "CUIWnd.h"

class CUIEdit;
class CUIButton;
class CUIDropDownList;
class ShipDesign;

/** Lets the player design ships */
class DesignWnd : public GG::Wnd {
public:
    //! \name Signal Types //!@{
    typedef boost::signal<void ()>  DesignChangedSignalType;    //!< emitted when the design is changed
    //@}

    /** \name Structors */ //@{
    DesignWnd(int w, int h);
    //@}

    /** \name Mutators */ //@{
    void Reset();
    void Sanitize();

    void Render();

    void HullSelected(int index);                           ///< called when a hull is selected in hull droplist
    void PartSelected(int index, int slot);                 ///< called when a part is selected in one of the part droplists

    mutable DesignChangedSignalType DesignChangedSignal;
    //@}

private:
    void DoLayout();

    bool ValidateCurrentDesign();                           ///< returns true if ship currently being designed is complete and can be added to the empire and universe

    void AddDesign();

    void SetDesignHull(const std::string& hull);            ///< sets m_selected_hull and creates appropriate set of part droplists
    void SetDesignPart(const std::string& part, int slot);  ///< sets m_selected_parts entry as indicated by \a slot

    void DesignChanged();                                   ///< called when design is changed.  disables add design button if design is invalid

    CUIDropDownList*                m_hulls_list;
    std::vector<CUIDropDownList*>   m_parts_lists;

    CUIButton*                      m_add_design_button;
    CUIEdit*                        m_design_name_edit;
    CUIEdit*                        m_design_description_edit;

    std::string                     m_selected_hull;
    std::vector<std::string>        m_selected_parts;
};

#endif // _DesignWnd_h_
