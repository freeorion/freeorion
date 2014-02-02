// -*- C++ -*-
#ifndef _ModeratorActionsWnd_h_
#define _ModeratorActionsWnd_h_

#include "CUIWnd.h"
#include "CUIControls.h"
#include "../universe/Enums.h"

#include <boost/signals2/signal.hpp>

#include <vector>

namespace GG {
    class Button;
}


class ModeratorActionsWnd : public CUIWnd {
public:
    //! \name Structors //!@{
    ModeratorActionsWnd(GG::X w, GG::Y h);
    //!@}

    /** \name Accessors */ //@{
    ModeratorActionSetting  SelectedAction() const;
    PlanetType              SelectedPlanetType() const;
    PlanetSize              SelectedPlanetSize() const;
    StarType                SelectedStarType() const;
    int                     SelectedEmpire() const;
    //!@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Refresh();
    void            EnableActions(bool enable = true);
    //!@}

    mutable boost::signals2::signal<void ()>           ClosingSignal;
    mutable boost::signals2::signal<void ()>           NoActionSelectedSignal;
    mutable boost::signals2::signal<void (StarType)>   CreateSystemActionSelectedSignal;
    mutable boost::signals2::signal<void (PlanetType)> CreatePlanetActionSelectedSignal;
    mutable boost::signals2::signal<void ()>           DeleteObjectActionSelectedSignal;
    mutable boost::signals2::signal<void (int)>        SetOwnerActionSelectedSignal;
    mutable boost::signals2::signal<void ()>           AddStarlaneActionSelectedSignal;

private:
    void            DoLayout();
    virtual void    CloseClicked();

    void            NoActionClicked();
    void            CreateSystemClicked();
    void            StarTypeSelected(GG::DropDownList::iterator it);
    void            CreatePlanetClicked();
    void            PlanetTypeSelected(GG::DropDownList::iterator it);
    void            PlanetSizeSelected(GG::DropDownList::iterator it);
    void            DeleteObjectClicked();
    void            SetOwnerClicked();
    void            EmpireSelected(GG::DropDownList::iterator it);
    void            AddStarlane();
    void            RemoveStarlane();

    StarType        StarTypeFromIndex(std::size_t i) const;
    PlanetType      PlanetTypeFromIndex(std::size_t i) const;
    PlanetSize      PlanetSizeFromIndex(std::size_t i) const;
    int             EmpireIDFromIndex(std::size_t i) const;

    bool                    m_actions_enabled;
    ModeratorActionSetting  m_selected_action;
    GG::Button*             m_no_action_button;
    GG::Button*             m_create_system_button;
    CUIDropDownList*        m_star_type_drop;
    GG::Button*             m_create_planet_button;
    CUIDropDownList*        m_planet_type_drop;
    CUIDropDownList*        m_planet_size_drop;
    GG::Button*             m_delete_object_button;
    GG::Button*             m_set_owner_button;
    CUIDropDownList*        m_empire_drop;
    GG::Button*             m_add_starlane_button;
    GG::Button*             m_remove_starlane_button;
};

#endif // _ModeratorActionsWnd_h_
