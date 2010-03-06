// -*- C++ -*-
#ifndef _CombatSetupWnd_h_
#define _CombatSetupWnd_h_

#include "CUIControls.h"
#include "CUIWnd.h"


namespace Ogre {
    class Entity;
    class Material;
    class MaterialPtr;
    class SceneManager;
    class SceneNode;
    class Vector3;
}

class Fleet;
class Ship;
class ShipDesign;
class CombatWnd;

class CombatSetupWnd :
    public CUIWnd
{
public:
    CombatSetupWnd(std::vector<Fleet*> fleets,
                   CombatWnd* combat_wnd,
                   Ogre::SceneManager* scene_manager,
                   boost::function<std::pair<bool, Ogre::Vector3> (const GG::Pt&)>
                   intersect_mouse_with_ecliptic,
                   boost::function<const Ogre::MaterialPtr& (const ShipDesign&)>
                   get_ship_material,
                   boost::function<void (int, Ogre::SceneNode*, Ogre::Entity*, const Ogre::MaterialPtr&)>
                   add_ship_node_to_combat_wnd,
                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE);
    ~CombatSetupWnd();

    GG::Pt ListRowSize() const;

protected:
    virtual bool EventFilter(GG::Wnd* w, const GG::WndEvent& event);

private:
    Ogre::SceneNode* PlaceableShipNode() const;
    void HandleMouseMoves(const GG::Pt& pt);

    void PlaceableShipSelected_(const GG::ListBox::SelectionSet& sels);
    void PlaceableShipSelected(Ship* ship);
    void CancelCurrentShipPlacement();
    void PlaceCurrentShip();

    CUIListBox* m_listbox;
    Ship* m_selected_placeable_ship;
    Ogre::SceneNode* m_placeable_ship_node;
    std::map<int, Ogre::Entity*> m_ship_entities;
    std::map<int, Ogre::SceneNode*> m_ship_nodes;
    Ogre::SceneManager* m_scene_manager;

    boost::function<std::pair<bool, Ogre::Vector3> (const GG::Pt& pt)>
    m_intersect_mouse_with_ecliptic;
    boost::function<const Ogre::MaterialPtr& (const ShipDesign&)>
    m_get_ship_material;
    boost::function<void (int, Ogre::SceneNode*, Ogre::Entity*, const Ogre::MaterialPtr&)>
    m_add_ship_node_to_combat_wnd;
};

#endif // _CombatSetupWnd_h_
