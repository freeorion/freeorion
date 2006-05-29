// -*- C++ -*-
#ifndef _SidePanel_h_
#define _SidePanel_h_

#ifndef _CUIControls_h_
#include "CUIControls.h"
#endif

#ifndef _GG_DynamicGraphic_h_
#include <GG/DynamicGraphic.h>
#endif

#ifndef _GG_Wnd_h_
#include <GG/Wnd.h>
#endif

#ifndef _GG_SignalsAndSlots_h_
#include <GG/SignalsAndSlots.h>
#endif

#ifndef _GG_Texture_h_
#include <GG/Texture.h>
#endif

#ifndef _ResourceCenter_h_
#include "../universe/ResourceCenter.h"
#endif

#ifndef _Planet_h_
#include "../universe/Planet.h"
#endif

#ifndef _System_h_
#include "../universe/System.h"
#endif

#include <vector>

class CUI_CloseButton;
class CUIDropDownList;
class CUIIconButton;
class CUIScroll;
class CUITextureButton;
class FocusSelector;
class RotatingPlanetControl;
class UniverseObjectVisitor;
namespace GG {class TextControl;}

class SidePanel : public GG::Wnd
{
public:
    class PlanetPanel;

    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> PlanetSelectedSignalType; ///< emitted when a rotating planet in the side panel is clicked by the user
    //@}

    /** \name Structors */ //@{
    SidePanel(int x, int y, int w, int h);
    ~SidePanel();
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;

    int                PlanetPanels() const;
    const PlanetPanel* GetPlanetPanel(int n) const;
    int                SystemID() const;
    int                PlanetID() const; ///< returns the id of the currently-selected planet, if any
    //@}

    /** \name Mutators */ //@{
    virtual void  Render();

    void          SelectPlanet(int planet_id); ///< selects the planet with id \a planet_id within the current system, if such a planet exists

    /** sets the predicate that determines what planets can be selected in the side panel.  If none is set, planet selection is disabled. */
    void          SetValidSelectionPredicate(const boost::shared_ptr<UniverseObjectVisitor> &visitor);
    //@}

    static void   SetSystem(int system_id); ///< sets the system currently being viewed in all side panels

    static const int MAX_PLANET_DIAMETER; // size of a huge planet, in on-screen pixels
    static const int MIN_PLANET_DIAMETER; // size of a tiny planet, in on-screen pixels

    mutable PlanetSelectedSignalType PlanetSelectedSignal;

private:
    class PlanetPanelContainer;
    class SystemResourceSummary;

    void SetSystemImpl();
    void SystemSelectionChanged(int selection);
    void SystemFleetAdded  (const Fleet &);
    void SystemFleetRemoved(const Fleet &);
    void FleetsChanged();
    void PlanetsChanged();
    void PrevButtonClicked();
    void NextButtonClicked();
    void PlanetSelected(int planet_id);

    CUIDropDownList     *m_system_name;
    GG::TextControl     *m_system_name_unknown;
    GG::Button          *m_button_prev, *m_button_next;
    GG::DynamicGraphic  *m_star_graphic;
    GG::TextControl     *m_static_text_systemproduction;

    int                 m_next_pltview_fade_in;
    int                 m_next_pltview_planet_id;
    int                 m_next_pltview_fade_out;

    std::vector<GG::SubTexture> m_fleet_icons;

    PlanetPanelContainer  *m_planet_panel_container;
    SystemResourceSummary *m_system_resource_summary;

    static const System*        s_system;
    static std::set<SidePanel*> s_side_panels;
};

#endif // _SidePanel_h_
