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
struct UniverseObjectVisitor;
class MultiIconValueIndicator;

namespace GG {
    class TextControl;
}

class SidePanel : public GG::Wnd
{
public:
    class PlanetPanel;

    /** \name Structors */ //@{
    SidePanel(GG::X x, GG::Y y, GG::Y h);
    ~SidePanel();
    //@}

    /** \name Accessors */ //@{
    virtual bool        InWindow(const GG::Pt& pt) const;

    int                 PlanetPanels() const;
    const PlanetPanel*  GetPlanetPanel(int n) const;
    int                 SystemID() const;
    int                 PlanetID() const; ///< returns the id of the currently-selected planet, if any
    //@}

    /** \name Mutators */ //@{
    virtual void        Render();
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    static void         Refresh();    ///< causes all sidepanels to refresh / update indicators

    void                SelectPlanet(int planet_id); ///< selects the planet with id \a planet_id within the current system, if such a planet exists

    /** sets the predicate that determines what planets can be selected in the side panel.  If none is set, planet selection is disabled. */
    void                SetValidSelectionPredicate(const boost::shared_ptr<UniverseObjectVisitor> &visitor);
    //@}

    static void         SetSystem(int system_id);       ///< sets the system currently being viewed in all side panels

    static const int            EDGE_PAD;               ///< spacing between widgets and edges of sidepanel

    mutable boost::signal<void (int)>   PlanetSelectedSignal;           ///< emitted when a rotating planet in the side panel is clicked by the user
    mutable boost::signal<void (int)>   SystemSelectedSignal;           ///< emitted when something in the sidepanel wants to change the selected system, including the droplist or back/forward arrows
    mutable boost::signal<void ()>      ResourceCenterChangedSignal;    ///< emitted when a planet's resourcecenter has changed, including when focus is chanaged

private:
    class PlanetPanelContainer;

    void                DoLayout();

    void                RefreshImpl();
    void                SetSystemImpl();
    void                SystemSelectionChanged(GG::DropDownList::iterator it);
    void                PrevButtonClicked();
    void                NextButtonClicked();
    void                PlanetSelected(int planet_id);

    void                FleetInserted(Fleet& fleet);
    void                FleetRemoved(Fleet& fleet);
    void                FleetStateChanged();

    CUIDropDownList*            m_system_name;
    GG::Button*                 m_button_prev;
    GG::Button*                 m_button_next;
    GG::DynamicGraphic*         m_star_graphic;

    std::vector<GG::SubTexture> m_fleet_icons;

    PlanetPanelContainer*       m_planet_panel_container;
    MultiIconValueIndicator*    m_system_resource_summary;

    static const System*        s_system;
    static std::set<SidePanel*> s_side_panels;

    std::set<boost::signals::connection>                m_system_connections;
    std::map<const Fleet*, boost::signals::connection>  m_fleet_state_change_signals;
};

#endif // _SidePanel_h_
