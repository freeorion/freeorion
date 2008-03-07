// -*- C++ -*-
#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include <GG/Wnd.h>
#include <boost/signal.hpp>
#include <boost/shared_ptr.hpp>

class CUIEdit;
class CUIButton;
class CUIDropDownList;
class EncyclopediaDetailPanel;
class ShipDesign;
namespace GG {
    class TextControl;
}

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

    void HullSelected(const std::string& hull_name);                    ///< called when a hull is selected.  alters the design accordingly
    void PartSelected(const std::string& part_name, unsigned int slot); ///< called when a part is selected.  alters the design accordingly
    //@}

    mutable DesignChangedSignalType DesignChangedSignal;

private:
    class BaseSelector;     // has tabs to pick empty hull, previously-saved design or an autodesign template to start new design
    class PartPalette;      // shows parts that can be clicked for detailed or dragged on slots in design
    class MainPanel;        // shows image of hull, slots and parts, design name and description entry boxes, confirm button

    void DoLayout();

    bool ValidateCurrentDesign();                                   ///< returns true if ship currently being designed is complete and can be added to the empire and universe

    void AddDesign();                                               ///< adds current design to those stored by this empire, allowing ships of this design to be produced

    void SetDesignHull(const std::string& hull);                    ///< sets m_selected_hull and creates appropriate set of part droplists
    void SetDesignPart(const std::string& part, unsigned int slot); ///< sets part in indicated \a slot

    void DesignChanged();                                           ///< called when design is changed.  disables add design button if design is invalid


    std::string                 m_selected_hull;
    std::vector<std::string>    m_selected_parts;


    CUIDropDownList*                                    m_hulls_list;
    std::vector<boost::shared_ptr<CUIDropDownList> >    m_parts_lists;
    std::vector<boost::shared_ptr<GG::TextControl> >    m_parts_list_labels;

    CUIButton*                  m_add_design_button;
    CUIEdit*                    m_design_name_edit;
    CUIEdit*                    m_design_description_edit;

    EncyclopediaDetailPanel*    m_detail_panel;
    BaseSelector*               m_base_selector;
    PartPalette*                m_part_palette;
    MainPanel*                  m_main_panel;
};

#endif // _DesignWnd_h_
