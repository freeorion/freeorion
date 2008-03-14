#include "CombatWnd.h"

#include "ClientUI.h"
#include "../util/Version.h"

#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
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
    const GG::Pt INVALID_SELECTION_DRAG_POS(-1, -1);

    const double SYSTEM_WIDTH = 1000.0;
    const double STAR_RADIUS = 80.0;
    const double MEDIUM_PLANET_RADIUS = 5.0;

    const double NEAR_CLIP = 0.01;
    const double FAR_CLIP = 1000.0;

    const double MAX_ZOOM_OUT_DISTANCE = SYSTEM_WIDTH;
    const double MIN_ZOOM_IN_DISTANCE = 0.5;

    Ogre::Real OrbitRadius(unsigned int orbit)
    {
        assert(orbit < 10);
        return SYSTEM_WIDTH / 10 * (orbit + 1) - 20.0;
    }

    // TODO: These are for testing only.
    const double EARTH_TO_SUN = OrbitRadius(2); // third orbit
    const double MEDIUM_SHIP_LENGTH = 0.5;

    Ogre::Vector3 Project(const Ogre::Camera& camera, const Ogre::Vector3& world_pt)
    {
        Ogre::Vector3 retval(-5.0, -5.0, 1.0);

        Ogre::Matrix4 modelview_ = camera.getViewMatrix();
        if ((modelview_ * world_pt).z < 0.0) {
            GLdouble modelview[16];
            for (std::size_t i = 0; i < 16; ++i) {
                modelview[i] = modelview_[i % 4][i / 4];
            }
            GLdouble projection[16];
            Ogre::Matrix4 projection_ = camera.getProjectionMatrixWithRSDepth();
            for (std::size_t i = 0; i < 16; ++i) {
                projection[i] = projection_[i % 4][i / 4];
            }
            Ogre::Viewport* viewport_ = camera.getViewport();
            GLint viewport[4] = {
                static_cast<GLint>(viewport_->getLeft()),
                static_cast<GLint>(viewport_->getTop()),
                static_cast<GLint>(viewport_->getWidth()),
                static_cast<GLint>(viewport_->getHeight())
            };
            GLdouble x, y, z;
            if (gluProject(world_pt.x, world_pt.y, world_pt.z,
                           modelview, projection, viewport,
                           &x, &y, &z)) {
                retval = Ogre::Vector3(x * 2 - 1.0, y * 2 - 1.0, z * 2 - 1.0);
            }
            Ogre::Vector3 eyeSpacePos = modelview_ * world_pt;
        }

        return retval;
    }
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
    setUseIdentityProjection(true);
    setUseIdentityView(true);
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
// FlareRect
////////////////////////////////////////////////////////////
CombatWnd::FlareRect::FlareRect(const std::string& material_name) :
    ManualObject("FlareRect"),
    m_material_name(material_name)
{
    setUseIdentityProjection(true);
    setUseIdentityView(true);
    setRenderQueueGroup(Ogre::RENDER_QUEUE_9);
    setQueryFlags(0);
}

void CombatWnd::FlareRect::Resize(Ogre::Real left, Ogre::Real top, Ogre::Real right, Ogre::Real bottom)
{
    clear();
    begin(m_material_name, Ogre::RenderOperation::OT_TRIANGLE_FAN);
    position(right, top, -1);
    textureCoord(1.0, 0.0);
    position(left, top, -1);
    textureCoord(0.0, 0.0);
    position(left, bottom, -1);
    textureCoord(0.0, 1.0);
    position(right, bottom, -1);
    textureCoord(1.0, 1.0);
    end();

    Ogre::AxisAlignedBox box;
    box.setInfinite();
    setBoundingBox(box);
}

////////////////////////////////////////////////////////////
// SelectedObject
////////////////////////////////////////////////////////////
struct CombatWnd::SelectedObject::SelectedObjectImpl
{
    SelectedObjectImpl() :
        m_object(0)
        {}
    explicit SelectedObjectImpl(Ogre::MovableObject* object) :
        m_object(object)
        {
            m_object->getParentSceneNode()->showBoundingBox(true);
        }
    ~SelectedObjectImpl()
        {
            if (!m_object)
                return;
            m_object->getParentSceneNode()->showBoundingBox(false);
        }

    Ogre::MovableObject* m_object;
};

// SelectedObject
CombatWnd::SelectedObject::SelectedObject() :
    m_impl(new SelectedObjectImpl)
{}

CombatWnd::SelectedObject::SelectedObject(Ogre::MovableObject* object) :
    m_impl(new SelectedObjectImpl(object))
{}

bool CombatWnd::SelectedObject::operator<(const SelectedObject& rhs) const
{ return m_impl->m_object < rhs.m_impl->m_object; }

CombatWnd::SelectedObject CombatWnd::SelectedObject::Key(Ogre::MovableObject* object)
{
    SelectedObject retval;
    retval.m_impl->m_object = object;
    return retval;
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
    m_distance_to_lookat_point(SYSTEM_WIDTH / 2.0),
    m_pitch(-Ogre::Math::HALF_PI),
    m_yaw(0.0),
    m_last_pos(),
    m_selection_drag_start(INVALID_SELECTION_DRAG_POS),
    m_selection_drag_stop(INVALID_SELECTION_DRAG_POS),
    m_mouse_dragged(false),
    m_currently_selected_scene_node(0),
    m_selection_rect(new SelectionRect),
    m_lookat_point(0, 0, 0),
    m_star_back_billboard(0),
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

    m_scene_manager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
    m_scene_manager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);//STENCIL_MODULATIVE);

    m_scene_manager->getRootSceneNode()->createChildSceneNode()->attachObject(m_selection_rect);

    Ogre::SceneNode* star_node = m_scene_manager->getRootSceneNode()->createChildSceneNode();
    Ogre::BillboardSet* star_billboard_set = m_scene_manager->createBillboardSet("StarBackBillboardSet");
    star_billboard_set->setRenderQueueGroup(Ogre::RENDER_QUEUE_8);
    star_billboard_set->setMaterialName("backgrounds/star_back");
    star_billboard_set->setDefaultDimensions(STAR_RADIUS * 2.0, STAR_RADIUS * 2.0);
    m_star_back_billboard = star_billboard_set->createBillboard(Ogre::Vector3(0.0, 0.0, 0.0));
    star_billboard_set->setVisible(true);
    star_node->attachObject(star_billboard_set);

    star_billboard_set = m_scene_manager->createBillboardSet("StarCoreBillboardSet");
    star_billboard_set->setRenderQueueGroup(Ogre::RENDER_QUEUE_9);
    star_billboard_set->setMaterialName("backgrounds/star_core");
    star_billboard_set->setDefaultDimensions(STAR_RADIUS * 2.0, STAR_RADIUS * 2.0);
    star_billboard_set->createBillboard(Ogre::Vector3(0.0, 0.0, 0.0));
    star_billboard_set->setVisible(true);
    star_node->attachObject(star_billboard_set);

#if 0
    m_scene_manager->setSkyBox(true, "backgrounds/sky_box_1");
#endif

    m_camera->setNearClipDistance(NEAR_CLIP);
    m_camera->setFarClipDistance(FAR_CLIP);

    UpdateCameraPosition();

    //////////////////////////////////////////////////////////////////
    // NOTE: Below is temporary code for combat system prototyping! //
    //////////////////////////////////////////////////////////////////

    // Load the "Durgha" ship mesh
    Ogre::MeshPtr m = Ogre::MeshManager::getSingleton().load(
        "durgha.mesh",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    Ogre::SceneNode* planet_node = m_scene_manager->getRootSceneNode()->createChildSceneNode("planet node");
    planet_node->setPosition(EARTH_TO_SUN, 0.0, 0.0);
    planet_node->setScale(MEDIUM_PLANET_RADIUS, MEDIUM_PLANET_RADIUS, MEDIUM_PLANET_RADIUS);

    // light comes from the star, and the star is at the origin
    Ogre::Vector3 light_dir = planet_node->getPosition();
    light_dir.normalise();

    Ogre::Entity* entity = m_scene_manager->createEntity("planet", "sphere.mesh");
    entity->setMaterialName("planet");
    assert(entity->getNumSubEntities() == 1u);
    entity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
    entity->setCastShadows(true);
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
    lead_durgha_node->setPosition(EARTH_TO_SUN * 0.9, 0.0, 0.0);

    Ogre::Vector3 bbox_size = entity->getBoundingBox().getSize();
    Ogre::Real bbox_max = std::max(bbox_size.x, std::max(bbox_size.y, bbox_size.z));
    Ogre::Real durgha_scale = MEDIUM_SHIP_LENGTH / bbox_max;
    lead_durgha_node->setScale(durgha_scale, durgha_scale, durgha_scale);

    // look at the planet initially
    m_currently_selected_scene_node = lead_durgha_node;
    UpdateCameraPosition();

    entity = m_scene_manager->createEntity("wing durgha 1", "durgha.mesh");
    entity->setCastShadows(true);
    Ogre::SceneNode* durgha_node = lead_durgha_node->createChildSceneNode("wing durgha 1 node");
    durgha_node->attachObject(entity);
    durgha_node->setPosition(250, 250, 0);

    Ogre::Light* star = m_scene_manager->createLight("Star");
    star->setType(Ogre::Light::LT_POINT);
    star->setPosition(Ogre::Vector3(0.0, 0.0, 0.0));
    star->setAttenuation(SYSTEM_WIDTH * 0.51, 1.0, 0.0, 0.0);
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
        if (m_selection_drag_start == INVALID_SELECTION_DRAG_POS) {
            m_selection_drag_start = pt;
            m_selection_rect->setVisible(true);
            m_selection_rect->clear();
        } else {
            m_selection_drag_stop = pt;
            m_selection_rect->Resize(m_selection_drag_start, m_selection_drag_stop);
        }
        m_mouse_dragged = true;
    }
}

void CombatWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_selection_drag_start != INVALID_SELECTION_DRAG_POS)
        EndShiftDrag();
}

void CombatWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_selection_drag_start != INVALID_SELECTION_DRAG_POS) {
        SelectObjectsInVolume(mod_keys & GG::MOD_KEY_CTRL);
        EndShiftDrag();
    } else if (!m_mouse_dragged) {
        if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
            Ogre::SceneNode* clicked_scene_node = movable_object->getParentSceneNode();
            assert(clicked_scene_node);
            if (mod_keys & GG::MOD_KEY_CTRL) {
                std::set<SelectedObject>::iterator it =
                    m_current_selections.find(SelectedObject::Key(movable_object));
                if (it == m_current_selections.end())
                    m_current_selections.insert(SelectedObject(movable_object));
                else
                    m_current_selections.erase(it);
            } else {
                DeselectAll();
                m_current_selections.insert(SelectedObject(movable_object));
                m_currently_selected_scene_node = clicked_scene_node;
            }
        } else {
            DeselectAll();
        }
    }
}

void CombatWnd::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::MButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_last_pos = pt;
    m_mouse_dragged = false;
}

void CombatWnd::MDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Pt delta_pos = pt - m_last_pos;
    if (m_mouse_dragged ||
        GG::GUI::GetGUI()->MinDragDistance() * GG::GUI::GetGUI()->MinDragDistance() <
        delta_pos.x * delta_pos.x + delta_pos.y * delta_pos.y) {
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

void CombatWnd::MButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::MClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::MDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::RDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::RButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
}

void CombatWnd::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    Ogre::Real move_incr = m_distance_to_lookat_point * 0.25;
    Ogre::Real scale_factor = 1.0;
    if (mod_keys & GG::MOD_KEY_SHIFT)
        scale_factor *= 2.0;
    if (mod_keys & GG::MOD_KEY_CTRL)
        scale_factor /= 4.0;
    Ogre::Real total_move = move_incr * scale_factor * -move;
    if (m_distance_to_lookat_point + total_move < MIN_ZOOM_IN_DISTANCE)
        total_move += MIN_ZOOM_IN_DISTANCE - (m_distance_to_lookat_point + total_move);
    else if (MAX_ZOOM_OUT_DISTANCE < m_distance_to_lookat_point + total_move)
        total_move -= (m_distance_to_lookat_point + total_move) - MAX_ZOOM_OUT_DISTANCE;
    m_distance_to_lookat_point += total_move;
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

    // update star
    {
        Ogre::Vector3 star_direction = Ogre::Vector3(0.0, 0.0, 0.0) - m_camera->getPosition();
        star_direction.normalise();
        Ogre::Radian angle_at_view_center_to_star =
            Ogre::Math::ACos(m_camera->getDirection().dotProduct(star_direction));
        Ogre::Real BRIGHTNESS_AT_MAX_FOVY = 0.25;
        Ogre::Real center_nearness_factor =
            1.0 - angle_at_view_center_to_star.valueRadians() / (m_camera->getFOVy() / 2.0).valueRadians();
        Ogre::Real star_back_brightness_factor =
            BRIGHTNESS_AT_MAX_FOVY + center_nearness_factor * (1.0 - BRIGHTNESS_AT_MAX_FOVY);
        // Raise the factor to a (smallish) power to create some nonlinearity in the scaling.
        star_back_brightness_factor = Ogre::Math::Pow(star_back_brightness_factor, 1.5);
        m_star_back_billboard->setColour(Ogre::ColourValue(1.0, 1.0, 1.0, star_back_brightness_factor));
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
    m_selection_drag_start = INVALID_SELECTION_DRAG_POS;
    m_selection_rect->setVisible(false);
}

void CombatWnd::SelectObjectsInVolume(bool toggle_selected_items)
{
    const float APP_WIDTH = GG::GUI::GetGUI()->AppWidth();
    const float APP_HEIGHT = GG::GUI::GetGUI()->AppHeight();

    float left = std::min(m_selection_drag_start.x, m_selection_drag_stop.x) / APP_WIDTH;
    float right = std::max(m_selection_drag_start.x, m_selection_drag_stop.x) / APP_WIDTH;
    float top = std::min(m_selection_drag_start.y, m_selection_drag_stop.y) / APP_HEIGHT;
    float bottom = std::max(m_selection_drag_start.y, m_selection_drag_stop.y) / APP_HEIGHT;

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
        std::pair<std::set<SelectedObject>::iterator, bool> insertion_result =
            m_current_selections.insert(SelectedObject(*it));
        if (!insertion_result.second)
            m_current_selections.erase(insertion_result.first);
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
    m_current_selections.clear();
}
