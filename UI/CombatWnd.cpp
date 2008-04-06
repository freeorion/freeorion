#include "CombatWnd.h"

#include "ClientUI.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
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

    const double SYSTEM_RADIUS = 1000.0;
    const double STAR_RADIUS = 80.0;

    const double NEAR_CLIP = 0.01;
    const double FAR_CLIP = 2000.0;

    const double MAX_ZOOM_OUT_DISTANCE = SYSTEM_RADIUS;
    const double MIN_ZOOM_IN_DISTANCE = 0.5;

    Ogre::Real OrbitRadius(unsigned int orbit)
    {
        assert(orbit < 10);
        return SYSTEM_RADIUS / 10 * (orbit + 1) - 20.0;
    }

    Ogre::Real PlanetRadius(PlanetSize size)
    {
        Ogre::Real retval = 0;
        switch (size) {
        case INVALID_PLANET_SIZE: retval = 0.0; break;
        case SZ_NOWORLD:          retval = 0.0; break;
        case SZ_TINY:             retval = 2.0; break;
        case SZ_SMALL:            retval = 3.5; break;
        default:
        case SZ_MEDIUM:           retval = 5.0; break;
        case SZ_LARGE:            retval = 7.0; break;
        case SZ_HUGE:             retval = 9.0; break;
        case SZ_ASTEROIDS:        retval = 0.0; break;
        case SZ_GASGIANT:         retval = 11.0; break; // this one goes to eleven
        case NUM_PLANET_SIZES:    retval = 0.0; break;
        };
        return retval;
    }

    // TODO: These are for testing only.
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

    std::string PlanetNodeMaterial(PlanetType type)
    {
        if (type == PT_GASGIANT)
            return "gas_giant";
        else if (type == PT_RADIATED || type == PT_BARREN)
            return "atmosphereless_planet";
        else if (type != PT_ASTEROIDS && type != INVALID_PLANET_TYPE)
            return "planet";
        else
            return "don't create";
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


PlanetType g_planet_type = PT_TERRAN;
Ogre::Entity* planet_entity = 0;
Ogre::Entity* atmosphere_entity = 0;
Ogre::SceneNode* planet_node = 0;
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
    m_distance_to_lookat_point(SYSTEM_RADIUS / 2.0),
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

    Ogre::Light* star = m_scene_manager->createLight("Star");
    star->setType(Ogre::Light::LT_POINT);
    star->setPosition(Ogre::Vector3(0.0, 0.0, 0.0));
    star->setAttenuation(SYSTEM_RADIUS * 0.51, 1.0, 0.0, 0.0);

    m_scene_manager->setSkyBox(true, "backgrounds/sky_box_1", 50.0);

    m_camera->setNearClipDistance(NEAR_CLIP);
    m_camera->setFarClipDistance(FAR_CLIP);

    // look at the star initially
    m_currently_selected_scene_node = star_node;
    UpdateCameraPosition();

    //////////////////////////////////////////////////////////////////
    // NOTE: Below is temporary code for combat system prototyping! //
    //////////////////////////////////////////////////////////////////

    // a system that looks much like the solar system
    std::vector<Planet*> planets;
    planets.push_back(new Planet(PT_INFERNO, SZ_SMALL));
    planets.push_back(new Planet(PT_TOXIC, SZ_LARGE));
    planets.push_back(new Planet(PT_TERRAN, SZ_LARGE));
    planets.push_back(new Planet(PT_DESERT, SZ_MEDIUM));
    planets.push_back(new Planet(PT_ASTEROIDS, SZ_ASTEROIDS));
    planets.push_back(new Planet(PT_GASGIANT, SZ_GASGIANT));
    planets.push_back(new Planet(PT_GASGIANT, SZ_GASGIANT));
    planets.push_back(new Planet(PT_GASGIANT, SZ_GASGIANT));
    planets.push_back(new Planet(PT_BARREN, SZ_TINY));
    planets.push_back(new Planet(PT_BARREN, SZ_HUGE));

    System system;
    system.SetStarType(STAR_BLUE);
    for (std::size_t i = 0; i < planets.size(); ++i) {
        GetUniverse().InsertID(planets[i], i);
        system.Insert(i, i);
    }

    InitCombat(system);
}

CombatWnd::~CombatWnd()
{
    Ogre::Root::getSingleton().removeFrameListener(this);
    m_scene_manager->destroyQuery(m_ray_scene_query);
    m_scene_manager->destroyQuery(m_volume_scene_query);
    delete m_selection_rect;

    // TODO: delete m_planet_nodes (or maybe all nodes via some Ogre function?)
}

void CombatWnd::InitCombat(const System& system)
{
    // TODO: move all of this to the ctor after prototyping is complete

    // create planets
    for (System::const_orbit_iterator it = system.begin(); it != system.end(); ++it) {
        if (const Planet* planet = GetUniverse().Object<Planet>(it->second)) {
            std::string material_name = PlanetNodeMaterial(planet->Type());
            if (material_name == "don't create")
                continue;

            std::string planet_name = "orbit " + boost::lexical_cast<std::string>(it->first) + " planet";

            Ogre::SceneNode* node =
                m_scene_manager->getRootSceneNode()->createChildSceneNode(planet_name + " node");
            node->setPosition(OrbitRadius(it->first), 0.0, 0.0);
            Ogre::Real planet_radius = PlanetRadius(planet->Size());
            node->setScale(planet_radius, planet_radius, planet_radius);

            // light comes from the star, and the star is at the origin
            Ogre::Vector3 light_dir = node->getPosition();
            light_dir.normalise();

            if (material_name == "gas_giant") {
                Ogre::Entity* entity = entity = m_scene_manager->createEntity(planet_name, "sphere.mesh");
                entity->setMaterialName("gas_giant_core");
                assert(entity->getNumSubEntities() == 1u);
                entity->setCastShadows(true);
                node->attachObject(entity);

                entity = m_scene_manager->createEntity(planet_name + " atmosphere", "sphere.mesh");
                entity->setMaterialName(material_name);
                entity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                node->attachObject(entity);
            } else {
                Ogre::Entity* entity = entity = m_scene_manager->createEntity(planet_name, "sphere.mesh");
                entity->setMaterialName(material_name);
                assert(entity->getNumSubEntities() == 1u);
                entity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                entity->setCastShadows(true);
                node->attachObject(entity);

                if (material_name == "planet") {
                    entity = m_scene_manager->createEntity(planet_name + " atmosphere", "sphere.mesh");
                    entity->setMaterialName("atmosphere");
                    entity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                    node->attachObject(entity);
                }
            }

            m_planet_nodes[it->first] = node;
        }
    }

    // create starlane entrance points
    for (System::const_lane_iterator it = system.begin_lanes(); it != system.begin_lanes(); ++it) {
        // TODO
    }
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
    const PlanetType CYCLE[3] = {PT_TERRAN, PT_BARREN, PT_GASGIANT};
    static int cycle_index = 0;
    if (0) {//key == GG::GGK_SPACE) {
        cycle_index = (cycle_index + 1) % 3;
        g_planet_type = CYCLE[cycle_index];
        planet_node->detachObject(atmosphere_entity);
        if (g_planet_type == PT_TERRAN) {
            planet_entity->setMaterialName("planet");
            planet_node->attachObject(atmosphere_entity);
        } else if (g_planet_type == PT_BARREN) {
            planet_entity->setMaterialName("atmosphereless_planet");
        } else if (g_planet_type == PT_GASGIANT) {
            planet_entity->setMaterialName("gas_giant");
        }
    }
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
