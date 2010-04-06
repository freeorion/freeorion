// -*- C++ -*-
#ifndef _CombatSetupWnd_h_
#define _CombatSetupWnd_h_

#include "CUIControls.h"
#include "CUIWnd.h"
#include "../combat/CombatOrder.h"


namespace Ogre {
    class Entity;
    class Material;
    class MaterialPtr;
    class MovableObject;
    class SceneManager;
    class SceneNode;
    class Vector3;
}

struct CombatSetupGroup;
class CombatWnd;
struct CombatData;
class Fleet;
class Ship;
class ShipDesign;
class UniverseObject;

class CombatSetupWnd :
    public CUIWnd
{
public:
    CombatSetupWnd(const std::vector<CombatSetupGroup>& setup_groups,
                   CombatWnd* combat_wnd,
                   CombatData* combat_data,
                   Ogre::SceneManager* scene_manager,
                   boost::function<std::pair<bool, Ogre::Vector3> (const GG::Pt&)>
                   intersect_mouse_with_ecliptic,
                   boost::function<const Ogre::MaterialPtr& (const ShipDesign&)>
                   get_ship_material,
                   boost::function<void (int, Ogre::SceneNode*, Ogre::Entity*, const Ogre::MaterialPtr&)>
                   add_ship_node_to_combat_wnd,
                   boost::function<Ogre::MovableObject* (const GG::Pt&)>
                   get_object_under_pt,
                   boost::function<void (int, const Ogre::Vector3&)>
                   reposition_ship_node,
                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE);
    ~CombatSetupWnd();

    GG::Pt ListRowSize() const;

protected:
    virtual bool EventFilter(GG::Wnd* w, const GG::WndEvent& event);

private:
    Ogre::SceneNode* PlaceableShipNode() const;
    void HandleMouseMoves(const GG::Pt& pt);
    void CreateCombatOrder(int ship_id, Ogre::SceneNode* node);
    void PlaceableShipSelected_(const GG::ListBox::SelectionSet& sels);
    void PlaceableShipSelected(Ship* ship);
    void UpdatePlacementIndicators(const Ship* ship);
    void CancelCurrentShipPlacement();
    void PlaceCurrentShip();
    void DoneButtonClicked();

    std::vector<CombatSetupGroup> m_setup_groups;
    std::map<std::size_t, std::vector<Ogre::SceneNode*> > m_region_nodes_by_setup_group;
    std::size_t m_current_setup_group;
    std::map<int, CombatOrder> m_placement_orders;
    bool m_setup_finished_waiting_for_server;
    bool m_dragging_placed_ship;
    GG::Pt m_button_press_on_placed_ship;
    Ogre::SceneNode* m_button_press_placed_ship_node;
    bool m_mouse_dragged;

    CUIListBox* m_listbox;
    CUIButton* m_done_button;
    Ship* m_selected_placeable_ship;
    Ogre::SceneNode* m_placeable_ship_node;
    std::map<int, Ogre::Entity*> m_ship_entities;
    std::map<int, Ogre::SceneNode*> m_ship_nodes;
    Ogre::SceneManager* m_scene_manager;
    std::map<int, UniverseObject*> m_combat_universe;

    boost::function<std::pair<bool, Ogre::Vector3> (const GG::Pt& pt)>
    m_intersect_mouse_with_ecliptic;
    boost::function<const Ogre::MaterialPtr& (const ShipDesign&)>
    m_get_ship_material;
    boost::function<void (int, Ogre::SceneNode*, Ogre::Entity*, const Ogre::MaterialPtr&)>
    m_add_ship_node_to_combat_wnd;
    boost::function<Ogre::MovableObject* (const GG::Pt&)>
    m_get_object_under_pt;
    boost::function<void (int, const Ogre::Vector3&)>
    m_reposition_ship_node;
};

#endif // _CombatSetupWnd_h_
