#include "CombatWnd.h"

#include "ClientUI.h"
#include "../util/Version.h"

#include <OgreCamera.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreMeshManager.h>
#include <OgreRoot.h>
#include <OgreRenderTarget.h>
#include <OgreSceneManager.h>
#include <OgreSceneQuery.h>
#include <OgreSubEntity.h>

#include <GG/GUI.h>


namespace {
    const GG::Pt INVALID_SHIFT_DRAG_POS(-1, -1);
}

////////////////////////////////////////////////////////////
// SelectionRect
////////////////////////////////////////////////////////////
CombatWnd::SelectionRect::SelectionRect() :
    ManualObject("SelectionRect")
{
    setUseIdentityProjection(true);
    setUseIdentityView(true);
    setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    setQueryFlags(0);
}

void CombatWnd::SelectionRect::Resize(const GG::Pt& pt1, const GG::Pt& pt2)
{
    const float APP_WIDTH = GG::GUI::GetGUI()->AppWidth();
    const float APP_HEIGHT = GG::GUI::GetGUI()->AppHeight();

    float left = std::min(pt1.x, pt2.x) / APP_WIDTH;
    float right = std::max(pt1.x, pt2.x) / APP_WIDTH;
    float top = std::min(pt1.y, pt2.y) / APP_HEIGHT;
    float bottom = std::max(pt1.y, pt2.y) / APP_HEIGHT;

    left = left * 2 - 1;
    right = right * 2 - 1;
    top = 1 - top * 2;
    bottom = 1 - bottom * 2;

    clear();
    begin("", Ogre::RenderOperation::OT_LINE_STRIP);
    position(left, top, -1);
    position(right, top, -1);
    position(right, bottom, -1);
    position(left, bottom, -1);
    position(left, top, -1);
    end();

    Ogre::AxisAlignedBox box;
    box.setInfinite();
    setBoundingBox(box);
}


////////////////////////////////////////////////////////////
// CombatWnd
////////////////////////////////////////////////////////////
CombatWnd::CombatWnd(Ogre::SceneManager* scene_manager,
                     Ogre::Camera* camera,
                     Ogre::Viewport* viewport) :
    Wnd(0, 0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(), GG::CLICKABLE),
    m_scene_manager(scene_manager),
    m_camera(camera),
    m_viewport(viewport),
    m_ray_scene_query(m_scene_manager->createRayQuery(Ogre::Ray())),
    m_volume_scene_query(m_scene_manager->createPlaneBoundedVolumeQuery(Ogre::PlaneBoundedVolumeList())),
    m_distance_to_lookat_point(m_camera->getPosition().length()),
    m_pitch(0.0),
    m_yaw(0.0),
    m_last_pos(),
    m_shift_drag_start(INVALID_SHIFT_DRAG_POS),
    m_shift_drag_stop(INVALID_SHIFT_DRAG_POS),
    m_mouse_dragged(false),
    m_currently_selected_scene_node(0),
    m_selection_rect(new SelectionRect),
    m_lookat_point(0, 0, 0),
    m_exit(false)
{
    Ogre::Root::getSingleton().addFrameListener(this);

    m_ray_scene_query->setSortByDistance(true);

    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load((ClientUI::ArtDir() / "combat" / "resources.cfg").native_file_string());

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator section_it = cf.getSectionIterator();
    while (section_it.hasMoreElements()) {
        Ogre::String section_name = section_it.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = section_it.getNext();
        for (Ogre::ConfigFile::SettingsMultiMap::iterator it = settings->begin();
             it != settings->end();
             ++it) {
            Ogre::String type_name = it->first, path_name = it->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                (ClientUI::ArtDir() / path_name).native_file_string(),
                type_name, section_name);
        }
    }

    // Initialise, parse scripts etc
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    m_scene_manager->getRootSceneNode()->createChildSceneNode()->attachObject(m_selection_rect);

    //////////////////////////////////////////////////////////////////
    // NOTE: Below is temporary code for combat system prototyping! //
    //////////////////////////////////////////////////////////////////

    m_scene_manager->setSkyBox(true, "backgrounds/sky_box_1", 50);

    // Load the "Durgha" ship mesh
    Ogre::MeshPtr m = Ogre::MeshManager::getSingleton().load(
        "durgha.mesh",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    Ogre::Vector3 light_dir(1, -0.15, 0.25);
    light_dir.normalise();

    Ogre::Entity* entity = m_scene_manager->createEntity("planet", "sphere.mesh");
    entity->setMaterialName("planet");
    assert(entity->getNumSubEntities() == 1u);
    entity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
    entity->setCastShadows(true);
    Ogre::SceneNode* planet_node = m_scene_manager->getRootSceneNode()->createChildSceneNode("planet node");
    planet_node->setScale(125.0, 125.0, 125.0);
    planet_node->attachObject(entity);

    entity = m_scene_manager->createEntity("atmosphere", "sphere.mesh");
    entity->setMaterialName("atmosphere");
    assert(entity->getNumSubEntities() == 1u);
    entity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
    planet_node->attachObject(entity);

    // Put a few Durghas into the scene
    entity = m_scene_manager->createEntity("lead durgha", "durgha.mesh");
    entity->setCastShadows(true);
    Ogre::SceneNode* lead_durgha_node = m_scene_manager->getRootSceneNode()->createChildSceneNode("lead durgha node");
    lead_durgha_node->setDirection(0, -1, 0);
    lead_durgha_node->yaw(Ogre::Radian(Ogre::Math::PI));
    lead_durgha_node->attachObject(entity);
    lead_durgha_node->setPosition(750, 0, 0);

    entity = m_scene_manager->createEntity("wing durgha 1", "durgha.mesh");
    entity->setCastShadows(true);
    Ogre::SceneNode* durgha_node = lead_durgha_node->createChildSceneNode("wing durgha 1 node");
    durgha_node->attachObject(entity);
    durgha_node->setPosition(250, 250, 0);

    m_scene_manager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
    m_scene_manager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
    Ogre::Light* star = m_scene_manager->createLight("Star");
    star->setType(Ogre::Light::LT_DIRECTIONAL);
    star->setDirection(light_dir);
}

CombatWnd::~CombatWnd()
{
    Ogre::Root::getSingleton().removeFrameListener(this);
    m_scene_manager->destroyQuery(m_ray_scene_query);
    m_scene_manager->destroyQuery(m_volume_scene_query);
    delete m_selection_rect;
}

void CombatWnd::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_last_pos = pt;
    m_mouse_dragged = false;
}

void CombatWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Pt delta_pos = pt - m_last_pos;
    if (m_mouse_dragged ||
        GG::GUI::GetGUI()->MinDragDistance() * GG::GUI::GetGUI()->MinDragDistance() <
        delta_pos.x * delta_pos.x + delta_pos.y * delta_pos.y) {
        if (mod_keys & GG::MOD_KEY_SHIFT) {
            if (m_shift_drag_start == INVALID_SHIFT_DRAG_POS) {
                m_shift_drag_start = pt;
                m_selection_rect->setVisible(true);
                m_selection_rect->clear();
            } else {
                m_shift_drag_stop = pt;
                m_selection_rect->Resize(m_shift_drag_start, m_shift_drag_stop);
            }
        } else {
            if (m_shift_drag_start != INVALID_SHIFT_DRAG_POS) {
                m_shift_drag_start = INVALID_SHIFT_DRAG_POS;
                m_selection_rect->clear();
            }

            m_last_pos = pt;

            Ogre::Radian delta_pitch =
                -delta_pos.y * 1.0 / GG::GUI::GetGUI()->AppHeight() * Ogre::Radian(Ogre::Math::PI);
            m_pitch += delta_pitch;
            if (m_pitch < Ogre::Radian(-Ogre::Math::HALF_PI))
                m_pitch = Ogre::Radian(-Ogre::Math::HALF_PI);
            if (Ogre::Radian(Ogre::Math::HALF_PI) < m_pitch)
                m_pitch = Ogre::Radian(Ogre::Math::HALF_PI);
            Ogre::Radian delta_yaw =
                -delta_pos.x * 1.0 / GG::GUI::GetGUI()->AppWidth() * Ogre::Radian(Ogre::Math::PI);
            m_yaw += delta_yaw;

            UpdateCameraPosition();

            m_mouse_dragged = true;
        }
    }
}

void CombatWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_shift_drag_start != INVALID_SHIFT_DRAG_POS)
        EndShiftDrag();
}

void CombatWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_shift_drag_start != INVALID_SHIFT_DRAG_POS) {
        SelectObjectsInVolume(mod_keys & GG::MOD_KEY_CTRL);
        EndShiftDrag();
    } else if (!m_mouse_dragged) {
        if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
            Ogre::SceneNode* clicked_scene_node = movable_object->getParentSceneNode();
            assert(clicked_scene_node);
            if (mod_keys & GG::MOD_KEY_CTRL) {
                std::set<Ogre::MovableObject*>::iterator it = m_current_selections.find(movable_object);
                if (it == m_current_selections.end()) {
                    movable_object->getParentSceneNode()->showBoundingBox(true); // TODO: Replace.
                    m_current_selections.insert(movable_object);
                } else {
                    (*it)->getParentSceneNode()->showBoundingBox(false); // TODO: Replace.
                    m_current_selections.erase(it);
                }
            } else {
                DeselectAll();
                movable_object->getParentSceneNode()->showBoundingBox(true); // TODO: Replace.
                m_current_selections.insert(movable_object);
                m_currently_selected_scene_node = clicked_scene_node;
                m_lookat_point = m_currently_selected_scene_node->getWorldPosition();
                UpdateCameraPosition();
            }
        } else {
            DeselectAll();
        }
    }
}

void CombatWnd::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{}

void CombatWnd::RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{}

void CombatWnd::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{}

void CombatWnd::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    const Ogre::Real c_move_incr = 10.0;
    const Ogre::Real c_min_distance = 50.0;
    Ogre::Real scale_factor = 1.0;
    if (mod_keys & GG::MOD_KEY_SHIFT)
        scale_factor *= 5.0;
    if (mod_keys & GG::MOD_KEY_CTRL)
        scale_factor /= 5.0;
    Ogre::Real total_move = c_move_incr * scale_factor * move;
    if (m_distance_to_lookat_point + total_move < c_min_distance)
        total_move -= c_min_distance - (m_distance_to_lookat_point + total_move);
    m_distance_to_lookat_point -= total_move;
    m_camera->moveRelative(Ogre::Vector3(0, 0, -total_move));
}

void CombatWnd::KeyPress(GG::Key key, GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_q && mod_keys & GG::MOD_KEY_CTRL)
        m_exit = true;
}

bool CombatWnd::frameStarted(const Ogre::FrameEvent& event)
{
    if (m_currently_selected_scene_node) {
        m_lookat_point = m_currently_selected_scene_node->getWorldPosition();
        UpdateCameraPosition();
    }
#if 0 // TODO: Remove this; it only here for profiling the number of triangles rendered.
    Ogre::RenderTarget::FrameStats stats = Ogre::Root::getSingleton().getRenderTarget("FreeOrion " + FreeOrionVersionString())->getStatistics();
    std::cout << "tris: " << stats.triangleCount << " batches: " << stats.batchCount << std::endl;
#endif
    return !m_exit;
}

void CombatWnd::UpdateCameraPosition()
{
    m_camera->setPosition(m_lookat_point);
    m_camera->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z);
    m_camera->pitch(m_pitch);
    m_camera->yaw(m_yaw);
    m_camera->moveRelative(Ogre::Vector3(0, 0, m_distance_to_lookat_point));
}

void CombatWnd::EndShiftDrag()
{
    m_shift_drag_start = INVALID_SHIFT_DRAG_POS;
    m_selection_rect->setVisible(false);
}

void CombatWnd::SelectObjectsInVolume(bool toggle_selected_items)
{
    const float APP_WIDTH = GG::GUI::GetGUI()->AppWidth();
    const float APP_HEIGHT = GG::GUI::GetGUI()->AppHeight();

    float left = std::min(m_shift_drag_start.x, m_shift_drag_stop.x) / APP_WIDTH;
    float right = std::max(m_shift_drag_start.x, m_shift_drag_stop.x) / APP_WIDTH;
    float top = std::min(m_shift_drag_start.y, m_shift_drag_stop.y) / APP_HEIGHT;
    float bottom = std::max(m_shift_drag_start.y, m_shift_drag_stop.y) / APP_HEIGHT;

    const float MIN_SELECTION_VOLUME = 0.0001;
    if ((right - left) * (bottom - top) < MIN_SELECTION_VOLUME)
        return;

    Ogre::Ray ul = m_camera->getCameraToViewportRay(left, top);
    Ogre::Ray ur = m_camera->getCameraToViewportRay(right, top);
    Ogre::Ray ll = m_camera->getCameraToViewportRay(left, bottom);
    Ogre::Ray lr = m_camera->getCameraToViewportRay(right, bottom);

    Ogre::PlaneBoundedVolume volume;
    volume.planes.push_back(
        Ogre::Plane(ul.getOrigin(), ur.getOrigin(), lr.getOrigin())); // front plane
    volume.planes.push_back(
        Ogre::Plane(ul.getOrigin(), ul.getPoint(1.0), ur.getPoint(1.0))); // top plane
    volume.planes.push_back(
        Ogre::Plane(ll.getOrigin(), lr.getPoint(1.0), ll.getPoint(1.0))); // bottom plane
    volume.planes.push_back(
        Ogre::Plane(ul.getOrigin(), ll.getPoint(1.0), ul.getPoint(1.0))); // left plane
    volume.planes.push_back(
        Ogre::Plane(ur.getOrigin(), ur.getPoint(1.0), lr.getPoint(1.0))); // right plane

    Ogre::PlaneBoundedVolumeList volume_list;
    volume_list.push_back(volume);
    m_volume_scene_query->setVolumes(volume_list);
    if (!toggle_selected_items)
        DeselectAll();
    Ogre::SceneQueryResult& result = m_volume_scene_query->execute();
    for (Ogre::SceneQueryResultMovableList::iterator it = result.movables.begin(); it != result.movables.end(); ++it) {
        std::pair<std::set<Ogre::MovableObject*>::iterator, bool> insertion_result = m_current_selections.insert(*it);
        if (insertion_result.second) {
            (*it)->getParentSceneNode()->showBoundingBox(true); // TODO: Remove.
        } else {
            (*insertion_result.first)->getParentSceneNode()->showBoundingBox(false); // TODO: Remove.
            m_current_selections.erase(insertion_result.first);
        }
    }
}

Ogre::MovableObject* CombatWnd::GetObjectUnderPt(const GG::Pt& pt)
{
    Ogre::MovableObject* retval = 0;
    Ogre::Ray ray = m_camera->getCameraToViewportRay(pt.x * 1.0 / GG::GUI::GetGUI()->AppWidth(),
                                                     pt.y * 1.0 / GG::GUI::GetGUI()->AppHeight());
    m_ray_scene_query->setRay(ray);
    Ogre::RaySceneQueryResult& result = m_ray_scene_query->execute();
    if (result.begin() != result.end())
        retval = result.begin()->movable;
    return retval;
}

void CombatWnd::DeselectAll()
{
    for (std::set<Ogre::MovableObject*>::iterator it = m_current_selections.begin(); it != m_current_selections.end(); ++it) {
        // TODO: Come up with a system for ensuring that the pointers in m_current_selections are not left dangling, so
        // that deselection code like that below doesn't crash the app.
        (*it)->getParentSceneNode()->showBoundingBox(false); // TODO: Replace.
    }
    m_current_selections.clear();
}
