// -*- C++ -*-
#ifndef _CombatWnd_h_
#define _CombatWnd_h_

#include "../combat/CombatEventListener.h"

#include <OgreFrameListener.h>
#include <OgreManualObject.h>
#include <OgreMath.h>
#include <OgreVector3.h>

#include <GG/Wnd.h>

#include <boost/tuple/tuple.hpp>


namespace Ogre {
    class Camera;
    class MovableObject;
    class RaySceneQuery;
    class SceneManager;
    class SceneNode;
    class Viewport;
    class PlaneBoundedVolumeListSceneQuery;
}

namespace GG {
    class Texture;
}

class CombatData;
class CombatSetupWnd;
class FPSIndicator;
class System;
class Ship;
class ShipDesign;
class UniverseObject;

class bt32BitAxisSweep3;
class btBvhTriangleMeshShape;
class btCollisionDispatcher;
class btCollisionWorld;
class btCollisionShape;
class btCollisionObject;
class btDefaultCollisionConfiguration;
class btTriangleMesh;

namespace Forests {
    class PagedGeometry;
    class TreeLoader3D;
}

class CombatWnd :
    public GG::Wnd,
    public Ogre::FrameListener,
    public CombatEventListener
{
public:
    CombatWnd (Ogre::SceneManager* scene_manager,
               Ogre::Camera* camera,
               Ogre::Viewport* viewport);
    virtual ~CombatWnd();

    void InitCombat(CombatData& combat_data);
    void HandlePlayerChatMessage(const std::string& msg);

    virtual void Render();

    virtual void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void MButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void RButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    virtual void ShipPlaced(const CombatShipPtr &ship);
    virtual void ShipFired(const CombatShipPtr &ship,
                           const CombatObjectPtr &target,
                           const std::string& part_name);
    virtual void ShipDestroyed(const CombatShipPtr &ship);
    virtual void ShipEnteredStarlane(const CombatShipPtr &ship);
    virtual void FighterLaunched(const CombatFighterPtr &fighter);
    virtual void FighterFired(const CombatFighterPtr &fighter,
                              const CombatObjectPtr &target);
    virtual void FighterDestroyed(const CombatFighterPtr &fighter);
    virtual void FighterDocked(const CombatFighterPtr &fighter);
    virtual void MissileLaunched(const MissilePtr &missile);
    virtual void MissileExploded(const MissilePtr &missile);
    virtual void MissileRemoved(const MissilePtr &missile);

private:
    struct SelectedObject
    {
        struct SelectedObjectImpl;

        SelectedObject();
        explicit SelectedObject(Ogre::MovableObject* object);
        bool operator<(const SelectedObject& rhs) const;

        boost::shared_ptr<SelectedObjectImpl> m_impl;
    };

    class StencilOpQueueListener;

    std::pair<bool, Ogre::Vector3> IntersectMouseWithEcliptic(const GG::Pt& pt) const;

    virtual bool frameStarted(const Ogre::FrameEvent& event);
    virtual bool frameEnded(const Ogre::FrameEvent& event);

    void RenderLensFlare();

    void LookAt(Ogre::SceneNode* look_at_node);
    void LookAt(const Ogre::Vector3& look_at_point);
    void Zoom(int move, GG::Flags<GG::ModKey> mod_keys);
    void HandleRotation(const GG::Pt& delta);
    void UpdateCameraPosition();
    void UpdateStarFromCameraPosition();
    void UpdateSkyBox();
    void EndSelectionDrag();
    void SelectObjectsInVolume(bool toggle_selected_items);
    Ogre::MovableObject* GetObjectUnderPt(const GG::Pt& pt);
    void DeselectAll();
    const Ogre::MaterialPtr& GetShipMaterial(const ShipDesign& ship_design);
    void AddShipNode(int ship_id, Ogre::SceneNode* node, Ogre::Entity* entity,
                     const Ogre::MaterialPtr& material);
    void AddCombatShip(const CombatShipPtr& combat_ship);
    void RemoveCombatShip(const CombatShipPtr& combat_ship);

    // Keyboard accelerator handlers, etc.  See MapWnd for implementation
    // notes.
    bool OpenChatWindow();
    bool EndTurn();
    bool ShowMenu();
    bool KeyboardZoomIn();
    bool KeyboardZoomOut();
    bool ZoomToPrevIdleUnit();
    bool ZoomToNextIdleUnit();
    bool ZoomToPrevUnit();
    bool ZoomToNextUnit();
    void ConnectKeyboardAcceleratorSignals();
    void SetAccelerators();
    void RemoveAccelerators();
    void DisableAlphaNumAccels();
    void EnableAlphaNumAccels();
    void ChatMessageSentSlot();

    Ogre::SceneManager* m_scene_manager;
    Ogre::Camera* m_camera;
    Ogre::SceneNode* m_camera_node;
    Ogre::Viewport* m_viewport;
    Ogre::PlaneBoundedVolumeListSceneQuery* m_volume_scene_query;

    Ogre::Animation* m_camera_animation;
    Ogre::AnimationState* m_camera_animation_state;

    CombatData* m_combat_data;

    Ogre::Real m_distance_to_look_at_point;
    Ogre::Radian m_pitch;
    Ogre::Radian m_roll;
    GG::Pt m_last_pos;
    GG::Pt m_selection_drag_start;
    GG::Pt m_selection_drag_stop;
    bool m_mouse_dragged;
    Ogre::SceneNode* m_look_at_scene_node;
    GG::Rect m_selection_rect;
    Ogre::Vector3 m_look_at_point;
    std::map<Ogre::MovableObject*, SelectedObject> m_current_selections;
    Ogre::Billboard* m_star_back_billboard;
    Ogre::Real m_star_brightness_factor;
    boost::shared_ptr<GG::Texture> m_big_flare;
    boost::shared_ptr<GG::Texture> m_small_flare;

    // The scene nodes representing planets in the system and the materials
    // created to show them, indexed by orbit number.
    std::map<
        int,
        std::pair<Ogre::SceneNode*, std::vector<Ogre::MaterialPtr> >
    > m_planet_assets;

    // The scene nodes representing starlane entrance points in the system.
    std::set<Ogre::SceneNode*> m_starlane_entrance_point_nodes;

    // The scene nodes representing ships in the system and their associated
    // collision meshes, indexed by ship object id.
    struct ShipData
    {
        ShipData();
        ShipData(Ogre::SceneNode* node,
                 Ogre::MaterialPtr material,
                 btTriangleMesh* bt_mesh,
                 btBvhTriangleMeshShape* bt_shape,
                 btCollisionObject* bt_object);

        Ogre::SceneNode* m_node;
        Ogre::MaterialPtr m_material;
        btTriangleMesh* m_bt_mesh;
        btBvhTriangleMeshShape* m_bt_shape;
        btCollisionObject* m_bt_object;
    };
    std::map<int, ShipData> m_ship_assets;
    std::map<std::string, Ogre::MaterialPtr> m_ship_materials;

    std::vector<Ogre::TexturePtr> m_city_lights_textures;

    // The collision detection system
    btDefaultCollisionConfiguration* m_collision_configuration;
    btCollisionDispatcher* m_collision_dispatcher;
    bt32BitAxisSweep3* m_collision_broadphase;
    btCollisionWorld* m_collision_world;
    std::set<btCollisionShape*> m_collision_shapes;
    std::set<btCollisionObject*> m_collision_objects;

    Forests::PagedGeometry* m_paged_geometry;
    Forests::TreeLoader3D* m_paged_geometry_loader;

    Ogre::Real m_initial_left_horizontal_flare_scroll;
    Ogre::Real m_initial_right_horizontal_flare_scroll;
    Ogre::Real m_left_horizontal_flare_scroll_offset;
    Ogre::Real m_right_horizontal_flare_scroll_offset;

    StencilOpQueueListener* m_stencil_op_frame_listener;

    FPSIndicator* m_fps_text;

    CombatSetupWnd* m_combat_setup_wnd;

    bool m_menu_showing;
    std::set<boost::signals::connection> m_keyboard_accelerator_signals;
    std::set<GG::Key> m_disabled_accels_list;

    bool m_exit; // TODO: Remove this; it is only here for prototyping.
};

bool IsVisible(const Ogre::SceneNode& node);
Ogre::SceneNode* CreateShipSceneNode(Ogre::SceneManager* scene_manager, const Ship& ship);
Ogre::Entity* CreateShipEntity(Ogre::SceneManager* scene_manager, const Ship& ship,
                               const Ogre::MaterialPtr& material);

#endif
