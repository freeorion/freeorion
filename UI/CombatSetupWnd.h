// -*- C++ -*-
#ifndef _CombatSetupWnd_h_
#define _CombatSetupWnd_h_

#include "CUIControls.h"
#include "CUIWnd.h"


namespace Ogre {
    class SceneManager;
    class Entity;
    class SceneNode;
}

class Fleet;
class Ship;

class CombatSetupWnd :
    public CUIWnd
{
public:
    CombatSetupWnd(std::vector<Fleet*> fleets,
                   Ogre::SceneManager* scene_manager,
                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE);

    GG::Pt ListRowSize() const;
    Ship* PlaceableShip() const;
    Ogre::SceneNode* PlaceableShipNode() const;

    void EndShipPlacement();

private:
    void PlaceableShipSelected_(const GG::ListBox::SelectionSet& sels);
    void PlaceableShipSelected(Ship* ship);

    CUIListBox* m_listbox;
    Ship* m_selected_placeable_ship;
    Ogre::SceneNode* m_placeable_ship_node;
    std::map<std::string, Ogre::Entity*> m_placeable_ship_entities;
    Ogre::SceneManager* m_scene_manager;
};

#endif // _CombatSetupWnd_h_
