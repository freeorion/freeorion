// -*- C++ -*-
#ifndef _SidePanel_h_
#define _SidePanel_h_

#include "CUIControls.h"
#include <GG/DynamicGraphic.h>
#include <GG/Wnd.h>
#include <GG/SignalsAndSlots.h>
#include <GG/Texture.h>
#include "../universe/ResourceCenter.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include <vector>

class CUI_CloseButton;
class CUIDropDownList;
class CUIIconButton;
class CUIScroll;
class CUITextureButton;
class RotatingPlanetControl;
class UniverseObjectVisitor;
namespace GG {
    class TextControl;
}

class SidePanel : public GG::Wnd
{
public:
    class PlanetPanel;

    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> PlanetSelectedSignalType; ///< emitted when a rotating planet in the side panel is clicked by the user
    typedef boost::signal<void (int)> SystemSelectedSignalType; ///< emitted when something in the sidepanel wants to change the selected system, including the droplist or back/forward arrows
    typedef boost::signal<void ()> ResourceCenterChangedSignalType; ///< emitted when the a planet's resourcecenter has changed
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

    static void   Refresh();    ///< causes all sidepanels to refresh / update indicators

    void          SelectPlanet(int planet_id); ///< selects the planet with id \a planet_id within the current system, if such a planet exists

    /** sets the predicate that determines what planets can be selected in the side panel.  If none is set, planet selection is disabled. */
    void          SetValidSelectionPredicate(const boost::shared_ptr<UniverseObjectVisitor> &visitor);
    //@}

    static void   SetSystem(int system_id); ///< sets the system currently being viewed in all side panels

    static const int MAX_PLANET_DIAMETER; // size of a huge planet, in on-screen pixels
    static const int MIN_PLANET_DIAMETER; // size of a tiny planet, in on-screen pixels

    mutable PlanetSelectedSignalType PlanetSelectedSignal;
    mutable SystemSelectedSignalType SystemSelectedSignal;
    mutable ResourceCenterChangedSignalType ResourceCenterChangedSignal;

private:
    class PlanetPanelContainer;
    class SystemResourceSummary;

    void RefreshImpl();
    void SetSystemImpl();
    void SystemSelectionChanged(int selection);
    void SystemFleetAdded(const Fleet& flt);
    void SystemFleetRemoved(const Fleet& flt);
    void FleetsChanged();
    void UpdateSystemResourceSummary();
    void PrevButtonClicked();
    void NextButtonClicked();
    void PlanetSelected(int planet_id);

    CUIDropDownList     *m_system_name;
    GG::Button          *m_button_prev, *m_button_next;
    GG::DynamicGraphic  *m_star_graphic;

    int                 m_next_pltview_fade_in;
    int                 m_next_pltview_planet_id;
    int                 m_next_pltview_fade_out;

    std::vector<GG::SubTexture> m_fleet_icons;

    PlanetPanelContainer  *m_planet_panel_container;
    SystemResourceSummary *m_system_resource_summary;

    static const System*        s_system;
    static std::set<SidePanel*> s_side_panels;

    std::set<boost::signals::connection> m_system_connections;
    std::map<int, boost::signals::connection> m_fleet_connections;
};

#endif // _SidePanel_h_
