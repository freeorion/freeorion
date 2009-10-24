// -*- C++ -*-
#ifndef _CombatSetupWnd_h_
#define _CombatSetupWnd_h_

#include "CUIControls.h"
#include "CUIWnd.h"


namespace Ogre {
    class SceneManager;
    class Entity;
    class SceneNode;
    class Vector3;
}

class Fleet;
class Ship;
class CombatWnd;

class CombatSetupWnd :
    public CUIWnd
{
public:
    CombatSetupWnd(std::vector<Fleet*> fleets,
                   CombatWnd* combat_wnd,
                   Ogre::SceneManager* scene_manager,
                   boost::function<std::pair<bool, Ogre::Vector3> (const GG::Pt& pt)>
                   intersect_mouse_with_ecliptic,
                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::DRAGABLE);

    GG::Pt ListRowSize() const;

protected:
    virtual bool EventFilter(GG::Wnd* w, const GG::WndEvent& event);

private:
    Ogre::SceneNode* PlaceableShipNode() const;
    void HandleMouseMoves(const GG::Pt& pt);

    void PlaceableShipSelected_(const GG::ListBox::SelectionSet& sels);
    void PlaceableShipSelected(Ship* ship);
    void EndCurrentShipPlacement();

    CUIListBox* m_listbox;
    Ship* m_selected_placeable_ship;
    Ogre::SceneNode* m_placeable_ship_node;
    std::map<std::string, Ogre::Entity*> m_placeable_ship_entities;
    Ogre::SceneManager* m_scene_manager;
    boost::function<std::pair<bool, Ogre::Vector3> (const GG::Pt& pt)>
    m_intersect_mouse_with_ecliptic;
};

#endif // _CombatSetupWnd_h_
