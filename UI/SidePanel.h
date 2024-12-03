#ifndef _SidePanel_h_
#define _SidePanel_h_

#include <vector>
#include <GG/DynamicGraphic.h>
#include <GG/Texture.h>
#include "CUIControls.h"
#include "CUIWnd.h"
#include "../universe/Planet.h"
#include "../universe/System.h"


class Ship;

class CUIDropDownList;
class MultiIconValueIndicator;

class SidePanel : public CUIWnd {
public:
    class PlanetPanel;

    SidePanel(std::string_view config_name);
    void CompleteConstruction() override;
    ~SidePanel();

    bool InWindow(GG::Pt pt) const override;

    GG::Pt ClientUpperLeft() const noexcept override;

    /** Returns the id of the system shown in the SidePanels, or
      * INVALID_OBJECT_ID if no system is shown */
    static int SystemID() noexcept { return s_system_id; }

    /** Returns the id of the currently-selected planet, or
      * INVALID_OBJECT_ID if no planet is selected */
    int SelectedPlanetID() const noexcept { return (m_selection_enabled ? s_planet_id : INVALID_OBJECT_ID); }


    /** Returns whether this SidePanel contains an object with the indicated
      * \a object_id that can be selected within the SidePanel. */
    bool PlanetSelectable(int planet_id, const ObjectMap& objects) const;

    void PreRender() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    /** Updates contents of sidepanel, but doesn't recreate contents or check
      * that object pointers are still valid.  All SidePanels' are updated */
    static void Update();

    /** Make refresh of sidepanel and contents happen during
        pre-render. PreRender() Will recreate all contents from the stored
        system id. All SidePanels are refreshed. */
    static void Refresh();

    /** Selects the planet with id \a planet_id within the current system, if
      * such a planet exists.  All SidePanels' selected planets are set, if
      * those panels have planet selection enabled. */
    static void SelectPlanet(int planet_id, const ObjectMap& objects);

    /** Sets the system currently being viewed in all side panels */
    static void SetSystem(int system_id);

    /** Enables, or disables if \a enable is false, selection of planet panels
      * within this SidePanel.  Panels that can be selected are those which
      * meet selection conditions that need to be figured out and perhaps
      * made more configurable, but for now are just those planets owned by
      * this client's empire. */
    void EnableSelection(bool enable = true) noexcept { m_selection_enabled = enable; }

    /** Enables, or disables if \a enable is false, issuing orders via the SidePanel. */
    void EnableOrderIssuing(bool enable = true);

    /** emitted when a rotating planet in the side panel is clicked by the
      * user */
    static boost::signals2::signal<void (int)>    PlanetSelectedSignal;

    /** emitted when something in the sidepanel wants to change the selected
      * system, including the droplist or back/forward arrows */
    static boost::signals2::signal<void (int)>    SystemSelectedSignal;

    /** emitted when a planet's resourcecenter has changed, including when
      * focus is changed */
    static boost::signals2::signal<void ()>       ResourceCenterChangedSignal;

    /** emitted when a planet is left double clicked*/
    static boost::signals2::signal<void (int)>    PlanetDoubleClickedSignal;

    /** emitted when a planet is right clicked */
    static boost::signals2::signal<void (int)>    PlanetRightClickedSignal;

    /** emitted when a building is right clicked */
    static boost::signals2::signal<void (int)>    BuildingRightClickedSignal;

protected:
    void InitBuffers() override;

private:
    class PlanetPanelContainer;

    void DoLayout();

    void UpdateImpl(ScriptingContext& context, int empire_id); ///< updates contents quickly.  to be used when meters or other objects' data changes

    /** Fully refreshes sidepanel and contents, recreating all contents from
      * stored system id.  All SidePanels are refreshed. */
    void RefreshInPreRender(ScriptingContext& context);

    void RefreshImpl(ScriptingContext& context); ///< fully refreshes contents.  to be used when objects are created, destroyed or added to system

    /**  Insert all known systems into the SystemName drop down list.*/
    void RefreshSystemNames();
    /**  Refresh the system name list when it closes, in case the known systems changed while it was open. */
    void SystemNameDropListOpenedSlot(bool is_open);
    /**  Handle the user selecting a system in the droplist while the list is closed, using keys.
         It may emit SystemSelectedSignal. */
    void SystemSelectionChangedSlot(GG::DropDownList::iterator it);

    void PrevButtonClicked();            ///< responds to user clicking next system button
    void NextButtonClicked();            ///< responts to user clicking previous system button
    /** Respond to the user clicking a planet by selecting it if selection is enabled.*/
    void PlanetClickedSlot(int planet_id, const ObjectMap& objects) const;

    /** Responds to insertion fleets into system during a turn.  may update colonize buttons. */
    static void FleetsInserted(std::vector<int> fleets, const ObjectMap& objects);

    /** Responds to removal fleets from system during a turn.  may update colonize buttons. */
    static void FleetsRemoved(std::vector<int> fleets);

    class SystemNameDropDownList;
    std::shared_ptr<SystemNameDropDownList>     m_system_name;
    std::shared_ptr<GG::TextControl>            m_star_type_text;
    std::shared_ptr<GG::Button>                 m_button_prev;
    std::shared_ptr<GG::Button>                 m_button_next;
    std::shared_ptr<GG::DynamicGraphic>         m_star_graphic;

    std::vector<GG::SubTexture> m_fleet_icons;

    std::shared_ptr<PlanetPanelContainer>       m_planet_panel_container;
    std::shared_ptr<MultiIconValueIndicator>    m_system_resource_summary;

    std::vector<boost::signals2::scoped_connection> m_connections;

    bool        m_selection_enabled = false;

    static bool s_needs_update;
    static bool s_needs_refresh;

    static int  s_system_id;

    /** The id of the currently-selected planet, or INVALID_OBJECT_ID if no planet is selected. */
    static int  s_planet_id;

    static std::set<std::weak_ptr<SidePanel>, std::owner_less<std::weak_ptr<SidePanel>>> s_side_panels;

    static std::set<boost::signals2::scoped_connection>      s_system_connections;
    static std::map<int, boost::signals2::scoped_connection> s_fleet_state_change_signals;
};

#endif
