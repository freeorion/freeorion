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

    void HullSelected(int hull_index);                                      ///< called when a hull is selected.  alters the design accordingly
    void PartSelected(int part_index, bool external, unsigned int slot);    ///< called when a part is selected.  alters the design accordingly

    mutable DesignChangedSignalType DesignChangedSignal;
    //@}

private:
    void DoLayout();

    bool ValidateCurrentDesign();                           ///< returns true if ship currently being designed is complete and can be added to the empire and universe

    void AddDesign();

    void SetDesignHull(const std::string& hull);            ///< sets m_selected_hull and creates appropriate set of part droplists
    void SetDesignPart(const std::string& part, bool external, unsigned int slot);  ///< sets part in external part \a slot if \a external is true, or internal slot if external is false

    void DesignChanged();                                   ///< called when design is changed.  disables add design button if design is invalid

    CUIDropDownList*                                    m_hulls_list;
    std::vector<boost::shared_ptr<CUIDropDownList> >    m_external_parts_lists;
    std::vector<boost::shared_ptr<CUIDropDownList> >    m_internal_parts_lists;

    CUIButton*                      m_add_design_button;
    CUIEdit*                        m_design_name_edit;
    CUIEdit*                        m_design_description_edit;

    std::string                     m_selected_hull;
    std::vector<std::string>        m_selected_external_parts;
    std::vector<std::string>        m_selected_internal_parts;
};

#endif // _DesignWnd_h_
