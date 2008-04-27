#include "CombatWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "InGameMenu.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"

#include "OptionsWnd.h" // TODO: Remove this later, once the InGameMenu is in use for F10 presses instead.

#include <OgreAnimation.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <OgreCamera.h>
#include <OgreCompositorManager.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>
#include <OgreRoot.h>
#include <OgreRenderQueueListener.h>
#include <OgreRenderSystem.h>
#include <OgreRenderTarget.h>
#include <OgreSceneManager.h>
#include <OgreSceneQuery.h>
#include <OgreSubEntity.h>

#include <GG/GUI.h>

#include <boost/cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/cerrno.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

namespace {
    const GG::Pt INVALID_SELECTION_DRAG_POS(-1, -1);

    const Ogre::Real SYSTEM_RADIUS = 1000.0;
    const Ogre::Real STAR_RADIUS = 80.0;

    const Ogre::Real NEAR_CLIP = 0.01;
    const Ogre::Real FAR_CLIP = 3020.0;

    const Ogre::Real MAX_ZOOM_OUT_DISTANCE = SYSTEM_RADIUS;
    const Ogre::Real MIN_ZOOM_IN_DISTANCE = 0.5;

    // visibility masks
    const Ogre::uint32 REGULAR_OBJECTS_MASK = 1 << 0;
    const Ogre::uint32 GLOWING_OBJECTS_MASK = 1 << 1;

    // queue groups
    const int SELECTION_HILITING_OBJECT_RENDER_QUEUE =   Ogre::RENDER_QUEUE_MAIN + 1;

    const int STAR_BACK_QUEUE =                          Ogre::RENDER_QUEUE_6 + 0;
    const int STAR_CORE_QUEUE =                          Ogre::RENDER_QUEUE_6 + 1;
    const int ALPHA_OBJECTS_QUEUE =                      Ogre::RENDER_QUEUE_6 + 2;

    const int SELECTION_HILITING_OUTLINED_RENDER_QUEUE = Ogre::RENDER_QUEUE_7 + 0;
    const int SELECTION_HILITING_FILLED_1_RENDER_QUEUE = Ogre::RENDER_QUEUE_7 + 1;
    const int SELECTION_HILITING_FILLED_2_RENDER_QUEUE = Ogre::RENDER_QUEUE_7 + 2;

    const std::set<int> STENCIL_OP_RENDER_QUEUES =
        boost::assign::list_of
        (SELECTION_HILITING_OBJECT_RENDER_QUEUE)
        (SELECTION_HILITING_OUTLINED_RENDER_QUEUE)
        (SELECTION_HILITING_FILLED_1_RENDER_QUEUE)
        (SELECTION_HILITING_FILLED_2_RENDER_QUEUE);

    // stencil masks
    const Ogre::uint32 OUTLINE_SELECTION_HILITING_STENCIL_VALUE = 1 << 0;
    const Ogre::uint32 FULL_SELECTION_HILITING_STENCIL_VALUE    = 1 << 1;

    // query masks
    const Ogre::uint32 UNSELECTABLE_OBJECT_MASK = 1 << 0;

    // The time it takes to recenter on a new point, in seconds.
    const Ogre::Real CAMERA_RECENTER_TIME = 0.33333;

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

    std::string AtmosphereNameFromBaseName(const std::string& base_name)
    {
        std::string retval =
            base_name.substr(0, base_name.size() - 2) +
            "_atmosphere_" +
            base_name.substr(base_name.size() - 2, 2);
        boost::algorithm::to_lower(retval);
        return retval;
    }

    void AddOptions(OptionsDB& db)
    {
        db.Add("combat.enable-glow", "OPTIONS_DB_COMBAT_ENABLE_GLOW",
               true, Validator<bool>());
        db.Add("combat.filled-selection", "OPTIONS_DB_COMBAT_FILLED_SELECTION",
               false, Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

////////////////////////////////////////////////////////////
// SelectedObject
////////////////////////////////////////////////////////////
// SelectedObjectImpl
struct CombatWnd::SelectedObject::SelectedObjectImpl
{

    SelectedObjectImpl() :
        m_object(0),
        m_core_entity(0),
        m_outline_entity(0),
        m_fill_entity(0),
        m_scene_node(0),
        m_scene_manager(0)
        {}
    explicit SelectedObjectImpl(Ogre::MovableObject* object) :
        m_object(object),
        m_core_entity(0),
        m_outline_entity(0),
        m_fill_entity(0),
        m_scene_node(0),
        m_scene_manager(0)
        {
            m_scene_node = object->getParentSceneNode();
            assert(m_scene_node);
            m_scene_manager = m_scene_node->getCreator();
            assert(m_scene_manager);
            m_core_entity = boost::polymorphic_downcast<Ogre::Entity*>(m_object);

            m_core_entity->setRenderQueueGroup(SELECTION_HILITING_OBJECT_RENDER_QUEUE);

            const bool FILLED = GetOptionsDB().Get<bool>("combat.filled-selection");

            m_outline_entity = m_core_entity->clone(m_core_entity->getName() + " hiliting outline");
            m_outline_entity->setRenderQueueGroup(FILLED ?
                                                  SELECTION_HILITING_FILLED_1_RENDER_QUEUE :
                                                  SELECTION_HILITING_OUTLINED_RENDER_QUEUE);
            m_outline_entity->setMaterialName(FILLED ?
                                              "effects/selection/filled_hiliting_1" :
                                              "effects/selection/outline_hiliting");
            m_outline_entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
            m_outline_entity->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
            m_scene_node->attachObject(m_outline_entity);

            if (FILLED) {
                m_fill_entity = m_core_entity->clone(m_core_entity->getName() + " hiliting fill");
                m_fill_entity->setRenderQueueGroup(SELECTION_HILITING_FILLED_2_RENDER_QUEUE);
                m_fill_entity->setMaterialName("effects/selection/filled_hiliting_2");
                m_fill_entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                m_fill_entity->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
                m_scene_node->attachObject(m_fill_entity);
            }
        }
    ~SelectedObjectImpl()
        {
            if (m_object) {
                m_core_entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
                m_scene_node->detachObject(m_outline_entity);
                m_scene_manager->destroyEntity(m_outline_entity);
                if (GetOptionsDB().Get<bool>("combat.filled-selection")) {
                    m_scene_node->detachObject(m_fill_entity);
                    m_scene_manager->destroyEntity(m_fill_entity);
                }
            }
        }

    Ogre::MovableObject* m_object;
    Ogre::Entity* m_core_entity;
    Ogre::Entity* m_outline_entity;
    Ogre::Entity* m_fill_entity;
    Ogre::SceneNode* m_scene_node;
    Ogre::SceneManager* m_scene_manager;
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


////////////////////////////////////////////////////////////
// StencilOpQueueListener
////////////////////////////////////////////////////////////
class CombatWnd::StencilOpQueueListener :
    public Ogre::RenderQueueListener
{
public:
    StencilOpQueueListener() : m_stencil_dirty(false) {}

    virtual void renderQueueStarted(Ogre::uint8 queue_group_id, const Ogre::String&, bool&)
        {
            Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();

            // Note that this assumes that all the selection hiliting-related queues will come after
            // Ogre::RENDER_QUEUE_MAIN.
            if (queue_group_id == Ogre::RENDER_QUEUE_MAIN)
                m_stencil_dirty = true;

            if (STENCIL_OP_RENDER_QUEUES.find(queue_group_id) != STENCIL_OP_RENDER_QUEUES.end()) {
                if (m_stencil_dirty) {
                    render_system->clearFrameBuffer(Ogre::FBT_STENCIL);
                    m_stencil_dirty = false;
                }
                render_system->setStencilCheckEnabled(true);
            }

            if (queue_group_id == SELECTION_HILITING_OBJECT_RENDER_QUEUE) { // outlined object
                render_system->setStencilBufferParams(
                    Ogre::CMPF_ALWAYS_PASS,
                    OUTLINE_SELECTION_HILITING_STENCIL_VALUE, 0xFFFFFFFF,
                    Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_REPLACE, false);
            } else if (queue_group_id == SELECTION_HILITING_OUTLINED_RENDER_QUEUE) { // outline object's selection hiliting
                render_system->setStencilBufferParams(
                    Ogre::CMPF_NOT_EQUAL,
                    OUTLINE_SELECTION_HILITING_STENCIL_VALUE, 0xFFFFFFFF,
                    Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_REPLACE, false);
            } else if (queue_group_id == SELECTION_HILITING_FILLED_1_RENDER_QUEUE) { // fully-hilited object's stencil-writing pass
                render_system->setStencilBufferParams(
                    Ogre::CMPF_ALWAYS_PASS,
                    FULL_SELECTION_HILITING_STENCIL_VALUE, 0xFFFFFFFF,
                    Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_REPLACE, false);
            } else if (queue_group_id == SELECTION_HILITING_FILLED_2_RENDER_QUEUE) { // fully-hilited object's rendering pass
                render_system->setStencilBufferParams(
                    Ogre::CMPF_EQUAL,
                    FULL_SELECTION_HILITING_STENCIL_VALUE, 0xFFFFFFFF,
                    Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_ZERO, false);
            }
        }

    virtual void renderQueueEnded(Ogre::uint8 queue_group_id, const Ogre::String&, bool&)
        {
            Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();
            if (STENCIL_OP_RENDER_QUEUES.find(queue_group_id) != STENCIL_OP_RENDER_QUEUES.end())
                render_system->setStencilCheckEnabled(false);

            if (*STENCIL_OP_RENDER_QUEUES.rbegin() <= queue_group_id)
                render_system->setStencilBufferParams();
        }

private:
    bool m_stencil_dirty;
};


////////////////////////////////////////////////////////////
// CombatWnd
////////////////////////////////////////////////////////////
CombatWnd::CombatWnd(Ogre::SceneManager* scene_manager,
                     Ogre::Camera* camera,
                     Ogre::Viewport* viewport) :
    Wnd(0, 0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(), GG::CLICKABLE),
    m_scene_manager(scene_manager),
    m_camera(camera),
    m_camera_node(m_scene_manager->getRootSceneNode()->createChildSceneNode()),
    m_viewport(viewport),
    m_ray_scene_query(m_scene_manager->createRayQuery(Ogre::Ray())),
    m_volume_scene_query(m_scene_manager->createPlaneBoundedVolumeQuery(Ogre::PlaneBoundedVolumeList())),
    m_camera_animation(m_scene_manager->createAnimation("CameraTrack", CAMERA_RECENTER_TIME)),
    m_camera_animation_state(m_scene_manager->createAnimationState("CameraTrack")),
    m_distance_to_lookat_point(SYSTEM_RADIUS / 2.0),
    m_pitch(0.0),
    m_roll(0.0),
    m_last_pos(),
    m_selection_drag_start(INVALID_SELECTION_DRAG_POS),
    m_selection_drag_stop(INVALID_SELECTION_DRAG_POS),
    m_mouse_dragged(false),
    m_lookat_scene_node(0),
    m_selection_rect(),
    m_lookat_point(0, 0, 0),
    m_star_back_billboard(0),
    m_initial_left_horizontal_flare_scroll(0.0),
    m_initial_right_horizontal_flare_scroll(0.0),
    m_left_horizontal_flare_scroll_offset(0.0),
    m_right_horizontal_flare_scroll_offset(0.0),
    m_stencil_op_frame_listener(new StencilOpQueueListener),
    m_fps_text(new FPSIndicator(5, 5)),
    m_menu_showing(false),
    m_exit(false)
{
    GG::Connect(GetOptionsDB().OptionChangedSignal("combat.enable-glow"),
                &CombatWnd::UpdateStarFromCameraPosition, this);

    Ogre::Root::getSingleton().addFrameListener(this);
    m_scene_manager->addRenderQueueListener(m_stencil_op_frame_listener);

    m_ray_scene_query->setSortByDistance(true);

    m_volume_scene_query->setQueryMask(~UNSELECTABLE_OBJECT_MASK);
    m_ray_scene_query->setQueryMask(~UNSELECTABLE_OBJECT_MASK);

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

    Ogre::CompositorManager::getSingleton().addCompositor(m_viewport, "effects/glow");

    m_scene_manager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
    m_scene_manager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);//STENCIL_MODULATIVE);

    Ogre::SceneNode* star_node = m_scene_manager->getRootSceneNode()->createChildSceneNode();
    Ogre::BillboardSet* star_billboard_set = m_scene_manager->createBillboardSet("StarBackBillboardSet");
    star_billboard_set->setRenderQueueGroup(STAR_BACK_QUEUE);
    star_billboard_set->setMaterialName("backgrounds/star_back");
    star_billboard_set->setDefaultDimensions(STAR_RADIUS * 2.0, STAR_RADIUS * 2.0);
    m_star_back_billboard = star_billboard_set->createBillboard(Ogre::Vector3(0.0, 0.0, 0.0));
    star_billboard_set->setVisible(true);
    star_billboard_set->setVisibilityFlags(REGULAR_OBJECTS_MASK);
    m_initial_left_horizontal_flare_scroll =
        star_billboard_set->getMaterial()->getTechnique(0)->getPass(3)->getTextureUnitState(0)->getTextureUScroll();
    m_initial_right_horizontal_flare_scroll =
        star_billboard_set->getMaterial()->getTechnique(0)->getPass(4)->getTextureUnitState(0)->getTextureUScroll();
    star_node->attachObject(star_billboard_set);

    star_billboard_set = m_scene_manager->createBillboardSet("StarCoreBillboardSet");
    star_billboard_set->setRenderQueueGroup(STAR_CORE_QUEUE);
    star_billboard_set->setMaterialName("backgrounds/star_core");
    star_billboard_set->setDefaultDimensions(STAR_RADIUS * 2.0, STAR_RADIUS * 2.0);
    star_billboard_set->createBillboard(Ogre::Vector3(0.0, 0.0, 0.0));
    star_billboard_set->setVisible(true);
    star_billboard_set->setVisibilityFlags(GLOWING_OBJECTS_MASK);
    star_node->attachObject(star_billboard_set);

    Ogre::Light* star = m_scene_manager->createLight("Star");
    star->setType(Ogre::Light::LT_POINT);
    star->setPosition(Ogre::Vector3(0.0, 0.0, 0.0));
    star->setAttenuation(SYSTEM_RADIUS * 0.51, 1.0, 0.0, 0.0);

    m_scene_manager->setSkyBox(true, "backgrounds/sky_box_1", 50.0);

    m_camera->setNearClipDistance(NEAR_CLIP);
    m_camera->setFarClipDistance(FAR_CLIP);
    m_camera->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
    m_camera_node->attachObject(m_camera);

    m_camera_animation->setInterpolationMode(Ogre::Animation::IM_SPLINE);
    m_camera_animation_state->setEnabled(true);
    m_camera_animation_state->setLoop(false);

    // look at the star initially
    m_lookat_scene_node = star_node;
    UpdateCameraPosition();

    AttachChild(m_fps_text);

    //////////////////////////////////////////////////////////////////
    // NOTE: Below is temporary code for combat system prototyping! //
    //////////////////////////////////////////////////////////////////

    // a sample system
    std::vector<Planet*> planets;
    planets.push_back(new Planet(PT_SWAMP, SZ_SMALL));
    planets.push_back(new Planet(PT_TOXIC, SZ_LARGE));
    planets.push_back(new Planet(PT_TERRAN, SZ_LARGE));
    planets.push_back(new Planet(PT_INFERNO, SZ_MEDIUM));
    planets.push_back(new Planet(PT_RADIATED, SZ_LARGE));
    planets.push_back(new Planet(PT_BARREN, SZ_SMALL));
    planets.push_back(new Planet(PT_TUNDRA, SZ_MEDIUM));
    planets.push_back(new Planet(PT_GASGIANT, SZ_GASGIANT));
    planets.push_back(new Planet(PT_DESERT, SZ_TINY));
    planets.push_back(new Planet(PT_OCEAN, SZ_HUGE));

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
    m_scene_manager->removeRenderQueueListener(m_stencil_op_frame_listener);
    m_scene_manager->destroyQuery(m_ray_scene_query);
    m_scene_manager->destroyQuery(m_volume_scene_query);
    Ogre::CompositorManager::getSingleton().removeCompositor(m_viewport, "effects/glow");

    // TODO: delete nodes and materials in m_planet_assets (or maybe everything
    // via some Ogre function?)
}

void CombatWnd::InitCombat(const System& system)
{
    // TODO: move all of this to the ctor after prototyping is complete

    // build list of available star textures, by type
    std::set<std::string> star_textures;
    {
        namespace fs = boost::filesystem;
        fs::path dir = ClientUI::ArtDir() / "combat" / "backgrounds";
        assert(fs::is_directory(dir));
        fs::directory_iterator end_it;
        std::string type_str = ClientUI::StarTypeFilePrefixes()[system.Star()];
        for (fs::directory_iterator it(dir); it != end_it; ++it) {
            try {
                if (fs::exists(*it) &&
                    !fs::is_directory(*it) &&
                    boost::algorithm::starts_with(it->leaf(), type_str)) {
                    star_textures.insert(it->leaf().substr(0, type_str.size() + 2));
                }
            } catch (const fs::filesystem_error& e) {
                // ignore files for which permission is denied, and rethrow other exceptions
                if (e.system_error() != EACCES)
                    throw;
            }
        }
    }

    // pick and assign star textures
    {
        std::string base_name = *boost::next(star_textures.begin(), system.ID() % star_textures.size());
        Ogre::MaterialPtr back_material =
            Ogre::MaterialManager::getSingleton().getByName("backgrounds/star_back");
        Ogre::Technique* technique = back_material->getTechnique(0);
        technique->getPass(0)->getTextureUnitState(0)->setTextureName(base_name + "back.png");
        technique->getPass(1)->getTextureUnitState(0)->setTextureName(base_name + "rainbow.png");
        technique->getPass(2)->getTextureUnitState(0)->setTextureName(base_name + "rays.png");
        technique->getPass(3)->getTextureUnitState(0)->setTextureName(base_name + "horizontal_flare.png");
        technique->getPass(4)->getTextureUnitState(0)->setTextureName(base_name + "horizontal_flare.png");
        Ogre::MaterialPtr core_material =
            Ogre::MaterialManager::getSingleton().getByName("backgrounds/star_core");
        technique = core_material->getTechnique(0);
        technique->getPass(0)->getTextureUnitState(0)->setTextureName(base_name + "core.png");
    }

    // build list of available planet textures, by type
    std::map<PlanetType, std::set<std::string> > planet_textures;
    {
        namespace fs = boost::filesystem;
        fs::path dir = ClientUI::ArtDir() / "combat" / "meshes" / "planets";
        assert(fs::is_directory(dir));
        fs::directory_iterator end_it;
        for (std::map<PlanetType, std::string>::const_iterator type_it =
                 ClientUI::PlanetTypeFilePrefixes().begin();
             type_it != ClientUI::PlanetTypeFilePrefixes().end();
             ++type_it) {
            std::set<std::string>& current_textures = planet_textures[type_it->first];
            for (fs::directory_iterator it(dir); it != end_it; ++it) {
                try {
                    if (fs::exists(*it) &&
                        !fs::is_directory(*it) &&
                        boost::algorithm::starts_with(it->leaf(), type_it->second)) {
                        current_textures.insert(it->leaf().substr(0, type_it->second.size() + 2));
                    }
                } catch (const fs::filesystem_error& e) {
                    // ignore files for which permission is denied, and rethrow other exceptions
                    if (e.system_error() != EACCES)
                        throw;
                }
            }
        }
    }

    // create planets
    for (System::const_orbit_iterator it = system.begin(); it != system.end(); ++it) {
        if (const Planet* planet = GetUniverse().Object<Planet>(it->second)) {
            std::string material_name = PlanetNodeMaterial(planet->Type());
            if (material_name == "don't create")
                continue;

            std::string planet_name = "orbit " + boost::lexical_cast<std::string>(it->first) + " planet";

            Ogre::SceneNode* node =
                m_scene_manager->getRootSceneNode()->createChildSceneNode(planet_name + " node");
            Ogre::Real planet_radius = PlanetRadius(planet->Size());
            node->setScale(planet_radius, planet_radius, planet_radius);
            node->yaw(Ogre::Degree(planet->AxialTilt()));
            Ogre::Vector3 position(OrbitRadius(it->first), 0.0, 0.0);
            // TODO: Include turn number and orbital period in calculating position_rotation.
            Ogre::Quaternion position_rotation(Ogre::Radian(planet->InitialOrbitalPosition()),
                                               Ogre::Vector3::UNIT_Z);
            position = position_rotation * position;
            node->setPosition(position);

            // light comes from the star, and the star is at the origin
            Ogre::Vector3 light_dir = -node->getPosition();
            light_dir.normalise();
            light_dir = node->getOrientation().Inverse() * light_dir;

            std::string base_name =
                *boost::next(planet_textures[planet->Type()].begin(),
                             planet->ID() % planet_textures[planet->Type()].size());

            if (material_name == "gas_giant") {
                Ogre::Entity* entity = m_scene_manager->createEntity(planet_name, "sphere.mesh");
                entity->setMaterialName("gas_giant_core");
                assert(entity->getNumSubEntities() == 1u);
                entity->setCastShadows(true);
                entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                node->attachObject(entity);

                entity = m_scene_manager->createEntity(planet_name + " atmosphere", "sphere.mesh");
                entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
                entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                entity->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
                std::string new_material_name =
                    material_name + "_" + boost::lexical_cast<std::string>(it->first);
                Ogre::MaterialPtr material =
                    Ogre::MaterialManager::getSingleton().getByName(material_name);
                material = material->clone(new_material_name);
                m_planet_assets[it->first].second.push_back(material);
                material->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(base_name + ".png");
                entity->setMaterialName(new_material_name);
                node->attachObject(entity);
            } else {
                Ogre::Entity* entity = m_scene_manager->createEntity(planet_name, "sphere.mesh");
                entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                std::string new_material_name =
                    material_name + "_" + boost::lexical_cast<std::string>(it->first);
                Ogre::MaterialPtr material =
                    Ogre::MaterialManager::getSingleton().getByName(material_name);
                material = material->clone(new_material_name);
                m_planet_assets[it->first].second.push_back(material);
                assert(entity->getNumSubEntities() == 1u);
                material->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(base_name + "Day.png");
                material->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName(base_name + "Night.png");
                entity->setMaterialName(new_material_name);
                entity->setCastShadows(true);
                node->attachObject(entity);

                if (material_name == "planet") {
                    material->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName(base_name + "CloudGloss.png");
                    entity = m_scene_manager->createEntity(planet_name + " atmosphere", "sphere.mesh");
                    entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
                    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                    entity->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
                    entity->setMaterialName(AtmosphereNameFromBaseName(base_name));
                    entity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                    node->attachObject(entity);
                }
            }

            m_planet_assets[it->first].first = node;
        }
    }

    // create starlane entrance points
    for (System::const_lane_iterator it = system.begin_lanes(); it != system.begin_lanes(); ++it) {
        // TODO
    }
}

void CombatWnd::Render()
{
    if (m_selection_rect.ul != m_selection_rect.lr) {
        glColor4f(1.0, 1.0, 1.0, 0.5);
        glBegin(GL_LINE_LOOP);
        glVertex2i(m_selection_rect.lr.x, m_selection_rect.ul.y);
        glVertex2i(m_selection_rect.ul.x, m_selection_rect.ul.y);
        glVertex2i(m_selection_rect.ul.x, m_selection_rect.lr.y);
        glVertex2i(m_selection_rect.lr.x, m_selection_rect.lr.y);
        glEnd();
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
            m_selection_rect = GG::Rect();
        } else {
            m_selection_drag_stop = pt;
            m_selection_rect = GG::Rect(m_selection_drag_start, m_selection_drag_stop);
        }
        m_mouse_dragged = true;
    }
}

void CombatWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_selection_drag_start != INVALID_SELECTION_DRAG_POS)
        EndSelectionDrag();
}

void CombatWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_selection_drag_start != INVALID_SELECTION_DRAG_POS) {
        SelectObjectsInVolume(mod_keys & GG::MOD_KEY_CTRL);
        EndSelectionDrag();
    } else if (!m_mouse_dragged) {
        if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
            Ogre::SceneNode* clicked_scene_node = movable_object->getParentSceneNode();
            assert(clicked_scene_node);
            std::map<Ogre::MovableObject*, SelectedObject>::iterator it =
                m_current_selections.find(movable_object);
            if (it == m_current_selections.end()) {
                if (!(mod_keys & GG::MOD_KEY_CTRL)) {
                    DeselectAll();
                    const Ogre::Vector3 DISTANCE =
                        clicked_scene_node->getWorldPosition() - m_lookat_scene_node->getWorldPosition();
                    m_lookat_scene_node = clicked_scene_node;
                    m_camera_animation_state->setTimePosition(0.0);
                    Ogre::NodeAnimationTrack* track = m_camera_animation->createNodeTrack(0, m_camera_node);
                    const int STEPS = 8;
                    const Ogre::Real TIME_INCREMENT = CAMERA_RECENTER_TIME / STEPS;
                    const Ogre::Vector3 DISTANCE_INCREMENT = DISTANCE / STEPS;
                    // the loop extends an extra 2 steps in either direction, to
                    // ensure smoothness (since splines are being used)
                    for (int i = -2; i < STEPS + 2; ++i) {
                        Ogre::TransformKeyFrame* key = track->createNodeKeyFrame(i * TIME_INCREMENT);
                        key->setTranslate(m_lookat_scene_node->getWorldPosition() - DISTANCE + i * DISTANCE_INCREMENT);
                    }
                }
                m_current_selections[movable_object] = SelectedObject(movable_object);
            } else {
                if (mod_keys & GG::MOD_KEY_CTRL)
                    m_current_selections.erase(it);
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
        if (m_pitch < Ogre::Radian(0.0))
            m_pitch = Ogre::Radian(0.0);
        if (Ogre::Radian(Ogre::Math::HALF_PI) < m_pitch)
            m_pitch = Ogre::Radian(Ogre::Math::HALF_PI);
        Ogre::Radian delta_roll =
            -delta_pos.x * 1.0 / GG::GUI::GetGUI()->AppWidth() * Ogre::Radian(Ogre::Math::PI);
        m_roll += delta_roll;

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
    Ogre::Sphere bounding_sphere(Ogre::Vector3(), 0.0);
    if (m_lookat_scene_node)
        bounding_sphere = m_lookat_scene_node->getAttachedObject(0)->getWorldBoundingSphere();
    const Ogre::Real EFFECTIVE_MIN_DISTANCE =
        std::max(bounding_sphere.getRadius() * Ogre::Real(1.05), MIN_ZOOM_IN_DISTANCE);

    Ogre::Real move_incr = m_distance_to_lookat_point * 0.25;
    Ogre::Real scale_factor = 1.0;
    if (mod_keys & GG::MOD_KEY_SHIFT)
        scale_factor *= 2.0;
    if (mod_keys & GG::MOD_KEY_CTRL)
        scale_factor /= 4.0;
    Ogre::Real total_move = move_incr * scale_factor * -move;
    if (m_distance_to_lookat_point + total_move < EFFECTIVE_MIN_DISTANCE)
        total_move += EFFECTIVE_MIN_DISTANCE - (m_distance_to_lookat_point + total_move);
    else if (MAX_ZOOM_OUT_DISTANCE < m_distance_to_lookat_point + total_move)
        total_move -= (m_distance_to_lookat_point + total_move) - MAX_ZOOM_OUT_DISTANCE;
    m_distance_to_lookat_point += total_move;
}

void CombatWnd::KeyPress(GG::Key key, GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_q && mod_keys & GG::MOD_KEY_CTRL)
        m_exit = true;

    if (key == GG::GGK_F10 && !mod_keys && !m_menu_showing) {
        m_menu_showing = true;
#if 0 // TODO: Use the full in-game menu when the code is a bit more developed.
        InGameMenu menu;
        menu.Run();
#else
        OptionsWnd options_wnd;
        options_wnd.Run();
#endif
        m_menu_showing = false;
    }
}

bool CombatWnd::frameStarted(const Ogre::FrameEvent& event)
{
    if (m_lookat_scene_node) {
        m_lookat_point = m_lookat_scene_node->getWorldPosition();
        UpdateCameraPosition();
    }

    Ogre::RenderTarget::FrameStats stats =
        Ogre::Root::getSingleton().getRenderTarget("FreeOrion " + FreeOrionVersionString())->getStatistics();
    m_fps_text->SetText(boost::lexical_cast<std::string>(stats.lastFPS) + " FPS");

    const bool ENABLE_GLOW = GetOptionsDB().Get<bool>("combat.enable-glow");
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_viewport, "effects/glow", ENABLE_GLOW);
    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("backgrounds/star_back");
    material->getTechnique(0)->getPass(3)->setDepthCheckEnabled(!ENABLE_GLOW);
    material->getTechnique(0)->getPass(4)->setDepthCheckEnabled(!ENABLE_GLOW);

    m_camera_animation_state->addTime(event.timeSinceLastFrame);
    if (m_camera_animation_state->hasEnded())
        m_camera_animation->destroyAllTracks();

    return !m_exit;
}

bool CombatWnd::frameEnded(const Ogre::FrameEvent& event)
{
    return !m_exit;
}

void CombatWnd::UpdateCameraPosition()
{
    m_camera_node->setPosition(m_lookat_point);
    m_camera->setPosition(Ogre::Vector3::ZERO);
    m_camera->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z);
    m_camera->roll(m_roll);
    m_camera->pitch(m_pitch);
    m_camera->moveRelative(Ogre::Vector3(0, 0, m_distance_to_lookat_point));
    UpdateStarFromCameraPosition();
}

void CombatWnd::UpdateStarFromCameraPosition()
{
    // Determine occlusion of the horizontal midline across the star by objects
    // in the scene.  This is only enabled if glow in in play, since the effect
    // doesn't look right when glow is not used.
    if (GetOptionsDB().Get<bool>("combat.enable-glow")) {
        const Ogre::Vector3 RIGHT = m_camera->getRealRight();
        const Ogre::Vector3 CAMERA_POS = m_camera->getRealPosition();
        const Ogre::Vector3 SUN_CENTER(0.0, 0.0, 0.0);

        const int SAMPLES_PER_SIDE = 2;
        const int TOTAL_SAMPLES = 5;
        Ogre::MaterialPtr core_material =
            Ogre::MaterialManager::getSingleton().getByName("backgrounds/star_core");
        const Ogre::Real STAR_CORE_SCALE_FACTOR =
            core_material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureUScale();
        // HACK! The currently-used star cores only cover part of the texture.  Here, we adjust for this, so that the edge
        // of the star as it appears onscreen is actually what we use for the star radius below.
        const Ogre::Real RADIUS_ADJUSTMENT_FACTOR = 155.0 / 250.0;
        const Ogre::Real SAMPLE_INCREMENT = STAR_RADIUS * RADIUS_ADJUSTMENT_FACTOR * STAR_CORE_SCALE_FACTOR / SAMPLES_PER_SIDE;

        bool occlusions[TOTAL_SAMPLES];
        // left side positions
        for (int i = 0; i < SAMPLES_PER_SIDE; ++i) {
            Ogre::Vector3 direction = SUN_CENTER + (SAMPLES_PER_SIDE - i) * SAMPLE_INCREMENT * -RIGHT - CAMERA_POS;
            Ogre::Ray ray(CAMERA_POS, direction);
            m_ray_scene_query->setRay(ray);
            Ogre::RaySceneQueryResult& result = m_ray_scene_query->execute();
            occlusions[i] = false;
            if (result.begin() != result.end()) {
                Ogre::Real distance_squared = (result.begin()->movable->getParentSceneNode()->getWorldPosition() - CAMERA_POS).squaredLength();
                occlusions[i] = distance_squared < direction.squaredLength();
            }
        }

        // center position
        {
            Ogre::Vector3 direction = SUN_CENTER - CAMERA_POS;
            Ogre::Ray center_ray(CAMERA_POS, direction);
            m_ray_scene_query->setRay(center_ray);
            Ogre::RaySceneQueryResult& result = m_ray_scene_query->execute();
            occlusions[SAMPLES_PER_SIDE] = false;
            if (result.begin() != result.end()) {
                Ogre::Real distance_squared = (result.begin()->movable->getParentSceneNode()->getWorldPosition() - CAMERA_POS).squaredLength();
                occlusions[SAMPLES_PER_SIDE] = distance_squared < direction.squaredLength();
            }
        }

        // right side positions
        for (int i = 0; i < SAMPLES_PER_SIDE; ++i) {
            Ogre::Vector3 direction = SUN_CENTER + (i + 1) * SAMPLE_INCREMENT * RIGHT - CAMERA_POS;
            Ogre::Ray ray(CAMERA_POS, direction);
            m_ray_scene_query->setRay(ray);
            Ogre::RaySceneQueryResult& result = m_ray_scene_query->execute();
            occlusions[SAMPLES_PER_SIDE + 1 + i] = false;
            if (result.begin() != result.end()) {
                Ogre::Real distance_squared = (result.begin()->movable->getParentSceneNode()->getWorldPosition() - CAMERA_POS).squaredLength();
                occlusions[SAMPLES_PER_SIDE + 1 + i] = distance_squared < direction.squaredLength();
            }
        }

        unsigned int occlusion_index = 0;
        for (int i = 0; i < TOTAL_SAMPLES; ++i) {
            occlusion_index += occlusions[i] << (TOTAL_SAMPLES - 1 - i);
        }
        typedef boost::tuple<int, int, float> OcclusionParams;
        const OcclusionParams OCCLUSION_PARAMS[32] = {
            OcclusionParams(2, 2, 1.0),
            OcclusionParams(2, 2, 1.0),
            OcclusionParams(2, 2, 1.0),
            OcclusionParams(2, 2, 0.8),

            OcclusionParams(1, 3, 0.8),
            OcclusionParams(1, 1, 0.4),
            OcclusionParams(1, 1, 0.4),
            OcclusionParams(1, 1, 0.4),

            OcclusionParams(2, 2, 1.0),
            OcclusionParams(2, 2, 0.8),
            OcclusionParams(2, 2, 0.8),
            OcclusionParams(2, 2, 0.6),

            OcclusionParams(3, 3, 0.4),
            OcclusionParams(0, 3, 0.4),
            OcclusionParams(0, 4, 0.4),
            OcclusionParams(0, 0, 0.2),

            OcclusionParams(2, 2, 1.0),
            OcclusionParams(2, 2, 0.8),
            OcclusionParams(2, 2, 0.8),
            OcclusionParams(2, 2, 0.6),

            OcclusionParams(3, 3, 0.4),
            OcclusionParams(1, 3, 0.4),
            OcclusionParams(1, 1, 0.2),
            OcclusionParams(1, 1, 0.2),

            OcclusionParams(2, 2, 0.8),
            OcclusionParams(2, 2, 0.6),
            OcclusionParams(2, 2, 0.4),
            OcclusionParams(2, 2, 0.4),

            OcclusionParams(3, 3, 0.4),
            OcclusionParams(3, 3, 0.2),
            OcclusionParams(4, 4, 0.2),
            OcclusionParams(-1, -1, 0.0)
        };

        OcclusionParams occlusion_params = OCCLUSION_PARAMS[occlusion_index];
        m_left_horizontal_flare_scroll_offset = (SAMPLES_PER_SIDE - occlusion_params.get<0>()) * SAMPLE_INCREMENT / RADIUS_ADJUSTMENT_FACTOR / (2.0 * STAR_RADIUS);
        m_right_horizontal_flare_scroll_offset = -(SAMPLES_PER_SIDE - occlusion_params.get<1>()) * SAMPLE_INCREMENT / RADIUS_ADJUSTMENT_FACTOR / (2.0 * STAR_RADIUS);
        if (occlusion_params.get<0>() < 0)
            m_left_horizontal_flare_scroll_offset = 1.0;
        if (occlusion_params.get<1>() < 0)
            m_right_horizontal_flare_scroll_offset = 1.0;
    } else {
        m_left_horizontal_flare_scroll_offset = 0.0;
        m_right_horizontal_flare_scroll_offset = 0.0;
    }

    Ogre::Vector3 star_direction = Ogre::Vector3(0.0, 0.0, 0.0) - m_camera->getRealPosition();
    star_direction.normalise();
    Ogre::Radian angle_at_view_center_to_star =
        Ogre::Math::ACos(m_camera->getRealDirection().dotProduct(star_direction));
    Ogre::Real BRIGHTNESS_AT_MAX_FOVY = 0.25;
    Ogre::Real center_nearness_factor =
        1.0 - angle_at_view_center_to_star.valueRadians() / (m_camera->getFOVy() / 2.0).valueRadians();
    Ogre::Real star_back_brightness_factor =
        BRIGHTNESS_AT_MAX_FOVY + center_nearness_factor * (1.0 - BRIGHTNESS_AT_MAX_FOVY);
    // Raise the factor to a (smallish) power to create some nonlinearity in the scaling.
    star_back_brightness_factor = Ogre::Math::Pow(star_back_brightness_factor, 1.5);
    m_star_back_billboard->setColour(Ogre::ColourValue(1.0, 1.0, 1.0, star_back_brightness_factor));

    Ogre::MaterialPtr back_material =
        Ogre::MaterialManager::getSingleton().getByName("backgrounds/star_back");
    back_material->getTechnique(0)->getPass(3)->getTextureUnitState(0)->setTextureUScroll(
        m_initial_left_horizontal_flare_scroll + m_left_horizontal_flare_scroll_offset);
    back_material->getTechnique(0)->getPass(4)->getTextureUnitState(0)->setTextureUScroll(
        m_initial_right_horizontal_flare_scroll + m_right_horizontal_flare_scroll_offset);
}

void CombatWnd::EndSelectionDrag()
{
    m_selection_drag_start = INVALID_SELECTION_DRAG_POS;
    m_selection_rect = GG::Rect();
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
        std::map<Ogre::MovableObject*, SelectedObject>::iterator object_it = m_current_selections.find(*it);
        if (object_it != m_current_selections.end())
            m_current_selections.erase(object_it);
        else
            m_current_selections[*it] = SelectedObject(*it);
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
