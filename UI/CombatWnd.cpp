#include "CombatWnd.h"

#include "ClientUI.h"
#include "CollisionMeshConverter.h"
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

#include <btBulletCollisionCommon.h>

#include <GG/GUI.h>
#include "../GG/src/GIL/image.hpp"
#include "../GG/src/GIL/extension/io/png_dynamic_io.hpp"

#include <boost/cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>


namespace {
    const GG::Pt INVALID_SELECTION_DRAG_POS(-GG::X1, -GG::Y1);

    const Ogre::Real SYSTEM_RADIUS = 1000.0;
    const Ogre::Real STAR_RADIUS = 80.0;

    const Ogre::Real NEAR_CLIP = 0.01;
    const Ogre::Real FAR_CLIP = 3020.0;

    const Ogre::Real MAX_ZOOM_OUT_DISTANCE = SYSTEM_RADIUS;
    const Ogre::Real MIN_ZOOM_IN_DISTANCE = 0.5;

    // collision dection system params
    btVector3 WORLD_AABB_MIN(-SYSTEM_RADIUS, -SYSTEM_RADIUS, -SYSTEM_RADIUS / 10.0);
    btVector3 WORLD_AABB_MAX(SYSTEM_RADIUS, SYSTEM_RADIUS, SYSTEM_RADIUS / 10.0);

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

    const unsigned int NO_CITY_LIGHTS = std::numeric_limits<unsigned int>::max();

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
            // TODO: Use _m member of transposed projection_ instead of doing
            // this "manual" copy.
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

    GG::Pt ProjectToPixel(const Ogre::Camera& camera, const Ogre::Vector3& world_pt)
    {
        Ogre::Vector3 projection = (Project(camera, world_pt) + Ogre::Vector3(1.0, 1.0, 1.0)) / 2.0;
        return GG::Pt(GG::X(static_cast<int>(projection.x * camera.getViewport()->getActualWidth())),
                      GG::Y(static_cast<int>((1.0 - projection.y) * camera.getViewport()->getActualHeight())));
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

    std::string AtmosphereMaterialName(const std::string& base_name)
    {
        std::string retval =
            base_name.substr(0, base_name.size() - 2) +
            "_atmosphere_" +
            base_name.substr(base_name.size() - 2, 2);
        boost::algorithm::to_lower(retval);
        return retval;
    }

    std::string PlanetMaterialName(const std::string& base_name)
    {
        std::string retval =
            base_name.substr(0, base_name.size() - 2) +
            "_planet_" +
            base_name.substr(base_name.size() - 2, 2);
        boost::algorithm::to_lower(retval);
        return retval;
    }

    btVector3 ToCollisionVector(const Ogre::Vector3& vec)
    { return btVector3(vec.x, vec.y, vec.z); }

    Ogre::Vector3 FromCollisionVector(const btVector3& vec)
    { return Ogre::Vector3(vec.x(), vec.y(), vec.z()); }

    struct RayIntersectionHit
    {
        RayIntersectionHit() : m_object(0) {}
        Ogre::MovableObject* m_object;
        Ogre::Vector3 m_point;
        Ogre::Vector3 m_normal;
    };

    RayIntersectionHit RayIntersection(btCollisionWorld& world, const Ogre::Ray& ray)
    {
        RayIntersectionHit retval;
        btCollisionWorld::ClosestRayResultCallback
            collision_results(ToCollisionVector(ray.getOrigin()),
                              ToCollisionVector(ray.getPoint(FAR_CLIP)));
        world.rayTest(collision_results.m_rayFromWorld,
                      collision_results.m_rayToWorld,
                      collision_results);
        if (collision_results.hasHit()) {
            retval.m_object = reinterpret_cast<Ogre::MovableObject*>(
                collision_results.m_collisionObject->getUserPointer());
            retval.m_point = FromCollisionVector(collision_results.m_hitPointWorld);
            retval.m_normal = FromCollisionVector(collision_results.m_hitNormalWorld);
        }
        return retval;
    }

    struct BadLightsTexture :
        GG::ExceptionBase
    {
        BadLightsTexture(const std::string& msg) throw() : ExceptionBase(msg) {}
        virtual const char* type() const throw() {return "BadLightsTexture";}
    };

    Ogre::TexturePtr PlanetLightsTexture(const std::string& base_name,
                                         unsigned int light_level)
    {
        assert(light_level == NO_CITY_LIGHTS || 0 <= light_level && light_level < 10);
        std::string filename = base_name;
        unsigned int channel = 0;
        if (light_level < 4) {
            filename += "LightsA.png";
            channel = light_level;
        } else if (light_level < 8) {
            filename += "LightsB.png";
            channel = light_level - 4;
        } else if (light_level == 8) {
            filename += "Day.png";
            channel = 3;
        } else {
            filename += "Night.png";
            channel = 3;
        }

        namespace fs = boost::filesystem;
        fs::path path(ClientUI::ArtDir() / "combat" / "meshes" / "planets" / filename);

        if (!fs::exists(path))
            throw BadLightsTexture("Texture file \"" + filename + "\" does not exist");

        if (!fs::is_regular_file(path))
            throw BadLightsTexture("Texture \"file\" \"" + filename + "\" is not a file");

        boost::gil::gray8_image_t final_image;
        if (light_level == NO_CITY_LIGHTS) {
            boost::gil::point2<std::ptrdiff_t> dimensions;
            try {
                dimensions = boost::gil::png_read_dimensions(path.string());
            } catch (const std::ios_base::failure &) {
                throw BadLightsTexture("Texture \"" + path.string() +
                                       "\" could not be read as a PNG file");
            }
            final_image.recreate(dimensions);
        } else {
            boost::gil::rgba8_image_t source_image;
            try {
                boost::gil::png_read_image(path.string(), source_image);
            } catch (const std::ios_base::failure &) {
                throw BadLightsTexture("Texture \"" + path.string() +
                                       "\" could not be read as a 32-bit RGBA PNG file");
            }
            final_image.recreate(source_image.dimensions());
            copy_pixels(nth_channel_view(const_view(source_image), channel), view(final_image));
        }

        const void* image_data = interleaved_view_get_raw_data(const_view(final_image));
        std::size_t image_size = final_image.width() * final_image.height();
        Ogre::DataStreamPtr stream(
            new Ogre::MemoryDataStream(const_cast<void*>(image_data), image_size));
        return Ogre::TextureManager::getSingleton().loadRawData(
            base_name + "_city_lights", "General", stream,
            final_image.width(), final_image.height(), Ogre::PF_BYTE_L);
    }

    void AddOptions(OptionsDB& db)
    {
        db.AddFlag("tech-demo", "Try out the 3D combat tech demo.");
        db.Add("combat.enable-glow", "OPTIONS_DB_COMBAT_ENABLE_GLOW",
               true, Validator<bool>());
        db.Add("combat.enable-skybox", "OPTIONS_DB_COMBAT_ENABLE_SKYBOX",
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
    Wnd(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(), GG::CLICKABLE),
    m_scene_manager(scene_manager),
    m_camera(camera),
    m_camera_node(m_scene_manager->getRootSceneNode()->createChildSceneNode()),
    m_viewport(viewport),
    m_volume_scene_query(m_scene_manager->createPlaneBoundedVolumeQuery(Ogre::PlaneBoundedVolumeList())),
    m_camera_animation(m_scene_manager->createAnimation("CameraTrack", CAMERA_RECENTER_TIME)),
    m_camera_animation_state(m_scene_manager->createAnimationState("CameraTrack")),
    m_distance_to_look_at_point(SYSTEM_RADIUS / 2.0),
    m_pitch(0.0),
    m_roll(0.0),
    m_last_pos(),
    m_selection_drag_start(INVALID_SELECTION_DRAG_POS),
    m_selection_drag_stop(INVALID_SELECTION_DRAG_POS),
    m_mouse_dragged(false),
    m_look_at_scene_node(0),
    m_selection_rect(),
    m_look_at_point(0, 0, 0),
    m_star_back_billboard(0),
    m_collision_configuration(0),
    m_collision_dispatcher(0),
    m_collision_broadphase(0),
    m_collision_world(0),
    m_initial_left_horizontal_flare_scroll(0.0),
    m_initial_right_horizontal_flare_scroll(0.0),
    m_left_horizontal_flare_scroll_offset(0.0),
    m_right_horizontal_flare_scroll_offset(0.0),
    m_stencil_op_frame_listener(new StencilOpQueueListener),
    m_fps_text(new FPSIndicator(GG::X(5), GG::Y(5))),
    m_menu_showing(false),
    m_exit(false)
{
    GG::Connect(GetOptionsDB().OptionChangedSignal("combat.enable-glow"),
                &CombatWnd::UpdateStarFromCameraPosition, this);

    GG::Connect(GetOptionsDB().OptionChangedSignal("combat.enable-skybox"),
                &CombatWnd::UpdateSkyBox, this);

    Ogre::Root::getSingleton().addFrameListener(this);
    m_scene_manager->addRenderQueueListener(m_stencil_op_frame_listener);

    m_volume_scene_query->setQueryMask(~UNSELECTABLE_OBJECT_MASK);

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

    UpdateSkyBox();

    m_camera->setNearClipDistance(NEAR_CLIP);
    m_camera->setFarClipDistance(FAR_CLIP);
    m_camera->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
    m_camera_node->attachObject(m_camera);

    m_camera_animation->setInterpolationMode(Ogre::Animation::IM_SPLINE);
    m_camera_animation_state->setEnabled(true);
    m_camera_animation_state->setLoop(false);

    // set up collision detection system
    m_collision_configuration = new btDefaultCollisionConfiguration;
    m_collision_dispatcher = new btCollisionDispatcher(m_collision_configuration);
    m_collision_broadphase = new bt32BitAxisSweep3(WORLD_AABB_MIN, WORLD_AABB_MAX);
    m_collision_world =
        new btCollisionWorld(m_collision_dispatcher, m_collision_broadphase, m_collision_configuration);

    // look at the star initially
    m_look_at_scene_node = star_node;
    UpdateCameraPosition();

    AttachChild(m_fps_text);


    //////////////////////////////////////////////////////////////////
    // NOTE: Below is temporary code for combat system prototyping! //
    //////////////////////////////////////////////////////////////////

    int system_id = 0;
    StarType star_type = STAR_BLUE;
    std::vector<int> planet_ids(10);
    for (int i = 0; i < 10; ++i) {
        planet_ids[i] = i;
    }
    std::vector<PlanetType> planet_types(10);
    planet_types[0] = PT_SWAMP;
    planet_types[1] = PT_TOXIC;
    planet_types[2] = PT_TERRAN;
    planet_types[3] = PT_INFERNO;
    planet_types[4] = PT_RADIATED;
    planet_types[5] = PT_BARREN;
    planet_types[6] = PT_TUNDRA;
    planet_types[7] = PT_GASGIANT;
    planet_types[8] = PT_DESERT;
    planet_types[9] = PT_OCEAN;
    std::vector<PlanetSize> planet_sizes(10);
    planet_sizes[0] = SZ_SMALL;
    planet_sizes[1] = SZ_LARGE;
    planet_sizes[2] = SZ_LARGE;
    planet_sizes[3] = SZ_MEDIUM;
    planet_sizes[4] = SZ_LARGE;
    planet_sizes[5] = SZ_SMALL;
    planet_sizes[6] = SZ_MEDIUM;
    planet_sizes[7] = SZ_GASGIANT;
    planet_sizes[8] = SZ_TINY;
    planet_sizes[9] = SZ_HUGE;

#if 1
    std::map<PlanetType, std::size_t> textures_available;
    std::size_t max_textures_available = 0;
    {
        namespace fs = boost::filesystem;
        fs::path dir = ClientUI::ArtDir() / "combat" / "meshes" / "planets";
        assert(fs::is_directory(dir));
        fs::directory_iterator end_it;
        for (std::map<PlanetType, std::string>::const_iterator type_it =
                 ClientUI::PlanetTypeFilePrefixes().begin();
             type_it != ClientUI::PlanetTypeFilePrefixes().end();
             ++type_it) {
            std::set<std::string> current_textures;
            for (fs::directory_iterator it(dir); it != end_it; ++it) {
                try {
                    if (fs::exists(*it) &&
                        !fs::is_directory(*it) &&
                        boost::algorithm::starts_with(it->filename(), type_it->second)) {
                        current_textures.insert(it->filename().substr(0, type_it->second.size() + 2));
                    }
                } catch (const fs::filesystem_error& e) {
                    // ignore files for which permission is denied, and rethrow other exceptions
                    if (e.code() != boost::system::posix_error::permission_denied)
                        throw;
                }
            }
            textures_available[type_it->first] = current_textures.size();
            max_textures_available = std::max(max_textures_available, current_textures.size());
        }
    }
    std::size_t planet_id_interval = 100 * max_textures_available;

    std::ifstream ifs("demo_planet_params.txt");
    if (ifs) {
        ifs >> system_id >> star_type;
        std::cout << system_id << " " << star_type << "\n";
        for (std::size_t i = 0; ifs && i < planet_ids.size(); ++i) {
            ifs >> planet_ids[i] >> planet_types[i] >> planet_sizes[i];
            std::cout << planet_ids[i] << " " << planet_types[i] << " " << planet_sizes[i] << "\n";
            int offset =
                planet_id_interval * i /
                textures_available[planet_types[i]] *
                textures_available[planet_types[i]];
            planet_ids[i] += offset;
            std::cout << planet_ids[i] << " "
                      << (planet_ids[i] % textures_available[planet_types[i]]) << "\n";
        }
    }
#endif

    // a sample system
    std::vector<Planet*> planets;
    planets.push_back(new Planet(planet_types[0], planet_sizes[0]));
    planets.push_back(new Planet(planet_types[1], planet_sizes[1]));
    planets.push_back(new Planet(planet_types[2], planet_sizes[2]));
    planets.push_back(new Planet(planet_types[3], planet_sizes[3]));
    planets.push_back(new Planet(planet_types[4], planet_sizes[4]));
    planets.push_back(new Planet(planet_types[5], planet_sizes[5]));
    planets.push_back(new Planet(planet_types[6], planet_sizes[6]));
    planets.push_back(new Planet(planet_types[7], planet_sizes[7]));
    planets.push_back(new Planet(planet_types[8], planet_sizes[8]));
    planets.push_back(new Planet(planet_types[9], planet_sizes[9]));

    System system(star_type, planets.size(), "Sample", 0.0, 0.0);
    for (std::size_t i = 0; i < planets.size(); ++i) {
        GetUniverse().InsertID(planets[i], planet_ids[i]);
        system.Insert(planet_ids[i], i);
        assert(system.Contains(i));
        assert(system.begin() != system.end());
    }

    InitCombat(system);

    AddShip("durgha.mesh", 250.0, 250.0);
}

CombatWnd::~CombatWnd()
{
    Ogre::Root::getSingleton().removeFrameListener(this);
    m_scene_manager->removeRenderQueueListener(m_stencil_op_frame_listener);
    m_scene_manager->destroyQuery(m_volume_scene_query);
    Ogre::CompositorManager::getSingleton().removeCompositor(m_viewport, "effects/glow");

    for (std::map<int, std::pair<Ogre::SceneNode*, btTriangleMesh*> >::iterator it = m_ship_assets.begin();
         it != m_ship_assets.end(); ++it) {
        // TODO: either delete these SceneNodes, or use an Ogre clear-everything function
        delete it->second.second;
    }

    for (std::size_t i = 0; i < m_city_lights_textures.size(); ++i) {
        Ogre::TextureManager::getSingleton().remove(m_city_lights_textures[i]->getName());
    }
    m_city_lights_textures.clear();

    m_collision_shapes.clear();
    m_collision_objects.clear();
    delete m_collision_world;
    delete m_collision_broadphase;
    delete m_collision_dispatcher;
    delete m_collision_configuration;

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
                    boost::algorithm::starts_with(it->filename(), type_str)) {
                    star_textures.insert(it->filename().substr(0, type_str.size() + 2));
                }
            } catch (const fs::filesystem_error& e) {
                // ignore files for which permission is denied, and rethrow other exceptions
                if (e.code() != boost::system::posix_error::permission_denied)
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

        m_big_flare =
            ClientUI::GetTexture(
                ClientUI::ArtDir() / "combat" / "backgrounds" / (base_name + "big_spark.png"),
                false);
        m_small_flare =
            ClientUI::GetTexture(
                ClientUI::ArtDir() / "combat" / "backgrounds" / (base_name + "small_spark.png"),
                false);
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
                        boost::algorithm::starts_with(it->filename(), type_it->second)) {
                        current_textures.insert(it->filename().substr(0, type_it->second.size() + 2));
                    }
                } catch (const fs::filesystem_error& e) {
                    // ignore files for which permission is denied, and rethrow other exceptions
                    if (e.code() != boost::system::posix_error::permission_denied)
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

            // set up a sphere in the collision detection system
            m_collision_shapes.push_back(new btSphereShape(planet_radius));
            m_collision_objects.push_back(new btCollisionObject);
            btMatrix3x3 identity;
            identity.setIdentity();
            m_collision_objects.back().getWorldTransform().setBasis(identity);
            m_collision_objects.back().getWorldTransform().setOrigin(ToCollisionVector(position));
            m_collision_objects.back().setCollisionShape(&m_collision_shapes.back());
            m_collision_world->addCollisionObject(&m_collision_objects.back());

            if (material_name == "gas_giant") {
                Ogre::Entity* entity = m_scene_manager->createEntity(planet_name, "sphere.mesh");
                entity->setMaterialName("gas_giant_core");
                assert(entity->getNumSubEntities() == 1u);
                entity->setCastShadows(true);
                entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                node->attachObject(entity);

                m_collision_objects.back().setUserPointer(static_cast<Ogre::MovableObject*>(entity));

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
                    Ogre::MaterialManager::getSingleton().getByName(material_name == "planet" ?
                                                                    PlanetMaterialName(base_name) :
                                                                    material_name);
                material = material->clone(new_material_name);
                m_planet_assets[it->first].second.push_back(material);
                assert(entity->getNumSubEntities() == 1u);
                material->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(base_name + "Day.png");
                material->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName(base_name + "Night.png");
                entity->setMaterialName(new_material_name);
                entity->setCastShadows(true);
                node->attachObject(entity);

                m_collision_objects.back().setUserPointer(static_cast<Ogre::MovableObject*>(entity));

                if (material_name == "planet") {
                    material->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName(base_name + "CloudGloss.png");
                    entity = m_scene_manager->createEntity(planet_name + " atmosphere", "sphere.mesh");
                    entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
                    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                    entity->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
                    std::string new_material_name =
                        material_name + "_atmosphere_" + boost::lexical_cast<std::string>(it->first);
                    Ogre::MaterialPtr material =
                        Ogre::MaterialManager::getSingleton().getByName(AtmosphereMaterialName(base_name));
                    material = material->clone(new_material_name);
                    m_planet_assets[it->first].second.push_back(material);
                    material->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
                    entity->setMaterialName(new_material_name);
                    node->attachObject(entity);
                } else {
                    assert(material_name == "atmosphereless_planet");
                    material->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName(base_name + "Normal.png");
                    double pop = planet->GetMeter(METER_POPULATION)->Current();
                    unsigned int lights_level = NO_CITY_LIGHTS;
                    const double MIN_POP_FOR_LIGHTS = 5.0;
                    if (MIN_POP_FOR_LIGHTS < pop)
                        lights_level = std::fmod(pop - 5.0, (100.0 - MIN_POP_FOR_LIGHTS) / 10.0);
                    Ogre::TexturePtr texture = PlanetLightsTexture(base_name, lights_level);
                    m_city_lights_textures.push_back(texture);
                    material->getTechnique(0)->getPass(0)->getTextureUnitState(3)->setTextureName(texture->getName());
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
#if 0 // TODO: Remove this.  It makes the planets all rotate, to test normal and parallax mapping.
    for (std::map<int, std::pair<Ogre::SceneNode*, std::vector<Ogre::MaterialPtr> > >::iterator it =
             m_planet_assets.begin();
         it != m_planet_assets.end();
         ++it) {
        it->second.first->yaw(Ogre::Radian(3.14159 / 180.0 / 3.0));
        if (dynamic_cast<Ogre::Entity*>(it->second.first->getAttachedObject(0))) {
            Ogre::Vector3 light_dir = -it->second.first->getPosition();
            light_dir.normalise();
            light_dir = it->second.first->getOrientation().Inverse() * light_dir;
            m_planet_assets[it->first].second.back()->getTechnique(0)->getPass(0)->getVertexProgramParameters()->setNamedConstant("light_dir", light_dir);
        }
    }
#endif

    // render two small lens flares that oppose the star's position relative to
    // the center of the viewport
    GG::Pt star_pt = ProjectToPixel(*m_camera, Ogre::Vector3(0.0, 0.0, 0.0));
    if (InClient(star_pt)) {
        Ogre::Ray ray(m_camera->getRealPosition(), -m_camera->getRealPosition());
        RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
        if (!hit.m_object) {
            GG::Pt center(Width() / 2, Height() / 2);
            GG::Pt star_to_center = center - star_pt;

            int big_flare_width = static_cast<int>(180 * m_star_brightness_factor);
            int small_flare_width = static_cast<int>(120 * m_star_brightness_factor);

            GG::Pt big_flare_ul = center + GG::Pt(star_to_center.x / 2, star_to_center.y / 2) -
                GG::Pt(GG::X(big_flare_width / 2), GG::Y(big_flare_width / 2));
            GG::Pt small_flare_ul = center + GG::Pt(3 * star_to_center.x / 4, 3 * star_to_center.y / 4) -
                GG::Pt(GG::X(small_flare_width / 2), GG::Y(small_flare_width / 2));
            GG::Pt big_flare_lr = big_flare_ul + GG::Pt(GG::X(big_flare_width), GG::Y(big_flare_width));
            GG::Pt small_flare_lr = small_flare_ul + GG::Pt(GG::X(small_flare_width), GG::Y(small_flare_width));

            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            m_big_flare->OrthoBlit(big_flare_ul, big_flare_lr, m_big_flare->DefaultTexCoords());
            m_small_flare->OrthoBlit(small_flare_ul, small_flare_lr, m_small_flare->DefaultTexCoords());
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    if (m_selection_rect.ul != m_selection_rect.lr) {
        glColor4f(1.0, 1.0, 1.0, 0.5);
        glBegin(GL_LINE_LOOP);
        glVertex(m_selection_rect.lr.x, m_selection_rect.ul.y);
        glVertex(m_selection_rect.ul.x, m_selection_rect.ul.y);
        glVertex(m_selection_rect.ul.x, m_selection_rect.lr.y);
        glVertex(m_selection_rect.lr.x, m_selection_rect.lr.y);
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
        static_cast<unsigned int>(Value(delta_pos.x * delta_pos.x) + Value(delta_pos.y * delta_pos.y))) {
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

void CombatWnd::LookAt(Ogre::SceneNode* look_at_node)
{
    m_look_at_scene_node = look_at_node;
    LookAt(m_look_at_scene_node->_getDerivedPosition());
}

void CombatWnd::LookAt(const Ogre::Vector3& look_at_point)
{
    const Ogre::Vector3 DISTANCE = look_at_point - m_look_at_point;
    m_look_at_point = look_at_point;
    UpdateCameraPosition();
    m_camera_animation_state->setTimePosition(0.0);
    Ogre::NodeAnimationTrack* track = m_camera_animation->createNodeTrack(0, m_camera_node);
    const int STEPS = 8;
    const Ogre::Real TIME_INCREMENT = CAMERA_RECENTER_TIME / STEPS;
    const Ogre::Vector3 DISTANCE_INCREMENT = DISTANCE / STEPS;
    // the loop extends an extra 2 steps in either direction, to
    // ensure smoothness (since splines are being used)
    for (int i = -2; i < STEPS + 2; ++i) {
        Ogre::TransformKeyFrame* key = track->createNodeKeyFrame(i * TIME_INCREMENT);
        key->setTranslate(look_at_point - DISTANCE + i * DISTANCE_INCREMENT);
    }
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
                if (!(mod_keys & GG::MOD_KEY_CTRL))
                    DeselectAll();
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
    if (!m_mouse_dragged) {
        if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
            Ogre::SceneNode* clicked_scene_node = movable_object->getParentSceneNode();
            assert(clicked_scene_node);
            LookAt(clicked_scene_node);
        } else {
            Ogre::Ray ray = m_camera->getCameraToViewportRay(Value(pt.x * 1.0 / GG::GUI::GetGUI()->AppWidth()),
                                                             Value(pt.y * 1.0 / GG::GUI::GetGUI()->AppHeight()));
            std::pair<bool, Ogre::Real> intersection =
                Ogre::Math::intersects(ray, Ogre::Plane(Ogre::Vector3::UNIT_Z, Ogre::Vector3::ZERO));
            const Ogre::Real MAX_DISTANCE_SQ = SYSTEM_RADIUS * SYSTEM_RADIUS;
            if (intersection.first) {
                Ogre::Vector3 intersection_point = ray.getPoint(intersection.second);
                if (intersection_point.squaredLength() < MAX_DISTANCE_SQ)
                    LookAt(intersection_point);
            }
        }
    }
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
        static_cast<unsigned int>(Value(delta_pos.x * delta_pos.x) + Value(delta_pos.y * delta_pos.y))) {
        m_last_pos = pt;

        Ogre::Radian delta_pitch =
            Value(-delta_pos.y * 1.0 / GG::GUI::GetGUI()->AppHeight()) * Ogre::Radian(Ogre::Math::PI);
        m_pitch += delta_pitch;
        if (m_pitch < Ogre::Radian(0.0))
            m_pitch = Ogre::Radian(0.0);
        if (Ogre::Radian(Ogre::Math::HALF_PI) < m_pitch)
            m_pitch = Ogre::Radian(Ogre::Math::HALF_PI);
        Ogre::Radian delta_roll =
            Value(-delta_pos.x * 1.0 / GG::GUI::GetGUI()->AppWidth()) * Ogre::Radian(Ogre::Math::PI);
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
    if (m_look_at_scene_node)
        bounding_sphere = m_look_at_scene_node->getAttachedObject(0)->getWorldBoundingSphere();
    const Ogre::Real EFFECTIVE_MIN_DISTANCE =
        std::max(bounding_sphere.getRadius() * Ogre::Real(1.05), MIN_ZOOM_IN_DISTANCE);

    Ogre::Real move_incr = m_distance_to_look_at_point * 0.25;
    Ogre::Real scale_factor = 1.0;
    if (mod_keys & GG::MOD_KEY_SHIFT)
        scale_factor *= 2.0;
    if (mod_keys & GG::MOD_KEY_CTRL)
        scale_factor /= 4.0;
    Ogre::Real total_move = move_incr * scale_factor * -move;
    if (m_distance_to_look_at_point + total_move < EFFECTIVE_MIN_DISTANCE)
        total_move += EFFECTIVE_MIN_DISTANCE - (m_distance_to_look_at_point + total_move);
    else if (MAX_ZOOM_OUT_DISTANCE < m_distance_to_look_at_point + total_move)
        total_move -= (m_distance_to_look_at_point + total_move) - MAX_ZOOM_OUT_DISTANCE;
    m_distance_to_look_at_point += total_move;
    UpdateCameraPosition();
}

void CombatWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
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
{ return !m_exit; }

void CombatWnd::UpdateCameraPosition()
{
    m_camera_node->setPosition(m_look_at_point);
    m_camera->setPosition(Ogre::Vector3::ZERO);
    m_camera->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z);
    m_camera->roll(m_roll);
    m_camera->pitch(m_pitch);
    m_camera->moveRelative(Ogre::Vector3(0, 0, m_distance_to_look_at_point));
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
            RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
            occlusions[i] = false;
            if (hit.m_object) {
                Ogre::Real distance_squared = (hit.m_point - CAMERA_POS).squaredLength();
                occlusions[i] = distance_squared < direction.squaredLength();
            }
        }

        // center position
        {
            Ogre::Vector3 direction = SUN_CENTER - CAMERA_POS;
            Ogre::Ray center_ray(CAMERA_POS, direction);
            RayIntersectionHit hit = RayIntersection(*m_collision_world, center_ray);
            occlusions[SAMPLES_PER_SIDE] = false;
            if (hit.m_object) {
                Ogre::Real distance_squared = (hit.m_point - CAMERA_POS).squaredLength();
                occlusions[SAMPLES_PER_SIDE] = distance_squared < direction.squaredLength();
            }
        }

        // right side positions
        for (int i = 0; i < SAMPLES_PER_SIDE; ++i) {
            Ogre::Vector3 direction = SUN_CENTER + (i + 1) * SAMPLE_INCREMENT * RIGHT - CAMERA_POS;
            Ogre::Ray ray(CAMERA_POS, direction);
            RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
            occlusions[SAMPLES_PER_SIDE + 1 + i] = false;
            if (hit.m_object) {
                Ogre::Real distance_squared = (hit.m_point - CAMERA_POS).squaredLength();
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
    m_star_brightness_factor =
        BRIGHTNESS_AT_MAX_FOVY + center_nearness_factor * (1.0 - BRIGHTNESS_AT_MAX_FOVY);
    // Raise the factor to a (smallish) power to create some nonlinearity in the scaling.
    m_star_brightness_factor = Ogre::Math::Pow(m_star_brightness_factor, 1.5);
    m_star_back_billboard->setColour(Ogre::ColourValue(1.0, 1.0, 1.0, m_star_brightness_factor));

    Ogre::MaterialPtr back_material =
        Ogre::MaterialManager::getSingleton().getByName("backgrounds/star_back");
    back_material->getTechnique(0)->getPass(3)->getTextureUnitState(0)->setTextureUScroll(
        m_initial_left_horizontal_flare_scroll + m_left_horizontal_flare_scroll_offset);
    back_material->getTechnique(0)->getPass(4)->getTextureUnitState(0)->setTextureUScroll(
        m_initial_right_horizontal_flare_scroll + m_right_horizontal_flare_scroll_offset);
}

void CombatWnd::UpdateSkyBox()
{
    Ogre::String skybox_material_name = "backgrounds/sky_box_1";
    Ogre::Real skybox_distance = 50.0;
    if (GetOptionsDB().Get<bool>("combat.enable-skybox")) {
        m_scene_manager->setSkyBox(true, skybox_material_name, skybox_distance);
    } else {
        m_scene_manager->setSkyBox(false, skybox_material_name, skybox_distance);
    }
}

void CombatWnd::EndSelectionDrag()
{
    m_selection_drag_start = INVALID_SELECTION_DRAG_POS;
    m_selection_rect = GG::Rect();
}

void CombatWnd::SelectObjectsInVolume(bool toggle_selected_items)
{
    const GG::X APP_WIDTH = GG::GUI::GetGUI()->AppWidth();
    const GG::Y APP_HEIGHT = GG::GUI::GetGUI()->AppHeight();

    float left = Value(std::min(m_selection_drag_start.x, m_selection_drag_stop.x) / APP_WIDTH);
    float right = Value(std::max(m_selection_drag_start.x, m_selection_drag_stop.x) / APP_WIDTH);
    float top = Value(std::min(m_selection_drag_start.y, m_selection_drag_stop.y) / APP_HEIGHT);
    float bottom = Value(std::max(m_selection_drag_start.y, m_selection_drag_stop.y) / APP_HEIGHT);

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
        // This if statement ensures that the center of the object is in the
        // volume, so we don't pick parts of objects' bounding rects that don't
        // actually contain any of the mesh itself.
        if (volume.intersects(Ogre::Sphere((*it)->getWorldBoundingBox().getCenter(), 0.001))) {
            std::map<Ogre::MovableObject*, SelectedObject>::iterator object_it = m_current_selections.find(*it);
            if (object_it != m_current_selections.end())
                m_current_selections.erase(object_it);
            else
                m_current_selections[*it] = SelectedObject(*it);
        }
    }
}

Ogre::MovableObject* CombatWnd::GetObjectUnderPt(const GG::Pt& pt)
{
    Ogre::Ray ray = m_camera->getCameraToViewportRay(Value(pt.x * 1.0 / GG::GUI::GetGUI()->AppWidth()),
                                                     Value(pt.y * 1.0 / GG::GUI::GetGUI()->AppHeight()));
    RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
    return hit.m_object;
}

void CombatWnd::DeselectAll()
{ m_current_selections.clear(); }

void CombatWnd::AddShip(const std::string& mesh_name, Ogre::Real x, Ogre::Real y)
{
    Ogre::Entity* entity = m_scene_manager->createEntity("ship_" + mesh_name, mesh_name);
    entity->setCastShadows(true);
    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
    Ogre::SceneNode* node =
        m_scene_manager->getRootSceneNode()->createChildSceneNode("ship_" + mesh_name + "_node");
    node->attachObject(entity);

    // TODO: This is only here because the Durgha model is upside down.  Remove
    // it when this is fixed.
    node->yaw(Ogre::Radian(Ogre::Math::PI));

    node->setPosition(x, y, 0.0);

    CollisionMeshConverter collision_mesh_converter(entity);
    btTriangleMesh* collision_mesh = 0;
    btBvhTriangleMeshShape* collision_shape = 0;
    boost::tie(collision_mesh, collision_shape) = collision_mesh_converter.CollisionShape();

    // TODO: use ship's ID
    m_ship_assets[0] = std::make_pair(node, collision_mesh);

    m_collision_shapes.push_back(collision_shape);
    m_collision_objects.push_back(new btCollisionObject);
    btMatrix3x3 identity;
    identity.setIdentity();
    // TODO: Remove z-flip scaling when models are right.
    btMatrix3x3 scaled = identity.scaled(btVector3(1.0, 1.0, -1.0));
    m_collision_objects.back().getWorldTransform().setBasis(scaled);
    m_collision_objects.back().getWorldTransform().setOrigin(ToCollisionVector(node->getPosition()));
    m_collision_objects.back().setCollisionShape(&m_collision_shapes.back());
    m_collision_world->addCollisionObject(&m_collision_objects.back());
    m_collision_objects.back().setUserPointer(static_cast<Ogre::MovableObject*>(entity));
}
