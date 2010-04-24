// -*- C++ -*-
#ifndef _CombatWndFwd_h_
#define _CombatWndFwd_h_

#include <OgreMaterial.h>


namespace Ogre {
    class Entity;
    class Quaternion;
    class SceneManager;
    class SceneNode;
    class Vector3;
}

namespace OpenSteer {
    class Vec3;
}

class Ship;
class ShipDesign;

bool IsVisible(const Ogre::SceneNode& node);
Ogre::SceneNode* CreateShipSceneNode(Ogre::SceneManager* scene_manager, const Ship& ship);
Ogre::Entity* CreateShipEntity(Ogre::SceneManager* scene_manager, const Ship& ship,
                               const Ogre::MaterialPtr& material);
Ogre::Vector3 ToOgre(const OpenSteer::Vec3& vec);
OpenSteer::Vec3 ToOpenSteer(const Ogre::Vector3& vec);
std::string ShipMaterialName(const ShipDesign& ship_design, int empire_id);
Ogre::Quaternion StarwardOrientationForPosition(const Ogre::Vector3& position);

extern const int PAGED_GEOMETRY_IMPOSTOR_QUEUE;
extern const int SELECTION_HILITING_OBJECT_RENDER_QUEUE;
extern const int STAR_BACK_QUEUE;
extern const int STAR_CORE_QUEUE;
extern const int ALPHA_OBJECTS_QUEUE;
extern const int SELECTION_HILITING_OUTLINED_RENDER_QUEUE;
extern const int SELECTION_HILITING_FILLED_1_RENDER_QUEUE;
extern const int SELECTION_HILITING_FILLED_2_RENDER_QUEUE;
extern const std::set<int> STENCIL_OP_RENDER_QUEUES;

extern const Ogre::uint32 UNSELECTABLE_OBJECT_MASK;

#endif
