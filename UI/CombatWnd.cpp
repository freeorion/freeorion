#include "CombatWnd.h"

#include "ChatWnd.h"
#include "ClientUI.h"
#include "CollisionMeshConverter.h"
#include "CUIControls.h"
#include "InGameMenu.h"
#include "../combat/OpenSteer/CombatFighter.h"
#include "../combat/OpenSteer/CombatShip.h"
#include "../combat/OpenSteer/Missile.h"
#include "../combat/OpenSteer/PathingEngine.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"

#include "OptionsWnd.h" // TODO: Remove this later, once the InGameMenu is in use for F10 presses instead.

// TODO: Remove these once the obstacle test code is removed.
#include "../combat/OpenSteer/AsteroidBeltObstacle.h"
#include "../combat/OpenSteer/Obstacle.h"
#include "../combat/OpenSteer/SimpleVehicle.h"

#include "PagedGeometry/BatchPage.h"
#include "PagedGeometry/ImpostorPage.h"
#include "PagedGeometry/PagedGeometry.h"
#include "PagedGeometry/TreeLoader3D.h"

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


#define TEST_STATIC_OPENSTEER_OBSTACLES 0

namespace {
    PathingEngine g_pathing_engine;
    std::map<const OpenSteer::AbstractObstacle*, std::string> g_obstacle_names;
    class FakeVehicle : public OpenSteer::SimpleVehicle
    {
        virtual void update(const float, const float) {}
    };

    const GG::Pt INVALID_SELECTION_DRAG_POS(-GG::X1, -GG::Y1);

    const Ogre::Real NEAR_CLIP = 0.01;
    const Ogre::Real FAR_CLIP = 3020.0;

    const Ogre::Real MAX_ZOOM_OUT_DISTANCE = 2.0 * SystemRadius();
    const Ogre::Real MIN_ZOOM_IN_DISTANCE = 0.5;

    // collision dection system params
    btVector3 WORLD_AABB_MIN(-SystemRadius(), -SystemRadius(), -SystemRadius() / 10.0);
    btVector3 WORLD_AABB_MAX(SystemRadius(), SystemRadius(), SystemRadius() / 10.0);

    // visibility masks
    const Ogre::uint32 REGULAR_OBJECTS_MASK = 1 << 0;
    const Ogre::uint32 GLOWING_OBJECTS_MASK = 1 << 1;

    // queue groups
    const int PAGED_GEOMETRY_IMPOSTOR_QUEUE =            Ogre::RENDER_QUEUE_MAIN - 1;

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

    const unsigned short LOOKAT_NODE_TRACK_HANDLE = 0;

    // HACK! The currently-used star cores only cover part of the texture.
    // Here, we adjust for this, so that the edge of the star as it appears
    // onscreen is actually what we use for the star radius below.
    const Ogre::Real STAR_RADIUS_ADJUSTMENT_FACTOR = 0.45;

    Ogre::Vector3 Project(const Ogre::Camera& camera, const Ogre::Vector3& world_pt)
    {
        Ogre::Vector3 retval(-5.0, -5.0, 1.0);
        Ogre::Vector3 eye_space_pt = camera.getViewMatrix() * world_pt;
        if (eye_space_pt.z < 0.0)
            retval = camera.getProjectionMatrixWithRSDepth() * eye_space_pt;
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
        assert(INVALID_PLANET_TYPE < type && type < NUM_PLANET_TYPES);
        if (type == PT_GASGIANT)
            return "gas_giant";
        else if (type == PT_RADIATED || type == PT_BARREN)
            return "atmosphereless_planet";
        else if (type == PT_ASTEROIDS)
            return "asteroid";
        else
            return "planet";
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

    btVector3 ToCollision(const Ogre::Vector3& vec)
    { return btVector3(vec.x, vec.y, vec.z); }

    btVector3 ToCollision(const OpenSteer::Vec3& vec)
    { return btVector3(vec.x, vec.y, vec.z); }

    Ogre::Vector3 ToOgre(const btVector3& vec)
    { return Ogre::Vector3(vec.x(), vec.y(), vec.z()); }

    Ogre::Vector3 ToOgre(const OpenSteer::Vec3& vec)
    { return Ogre::Vector3(vec.x, vec.y, vec.z); }

    OpenSteer::Vec3 ToOpenSteer(const btVector3& vec)
    { return OpenSteer::Vec3(vec.x(), vec.y(), vec.z()); }

    OpenSteer::Vec3 ToOpenSteer(const Ogre::Vector3& vec)
    { return OpenSteer::Vec3(vec.x, vec.y, vec.z); }

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
            collision_results(ToCollision(ray.getOrigin()),
                              ToCollision(ray.getPoint(FAR_CLIP)));
        world.rayTest(collision_results.m_rayFromWorld,
                      collision_results.m_rayToWorld,
                      collision_results);
        if (collision_results.hasHit()) {
            retval.m_object = reinterpret_cast<Ogre::MovableObject*>(
                collision_results.m_collisionObject->getUserPointer());
            retval.m_point = ToOgre(collision_results.m_hitPointWorld);
            retval.m_normal = ToOgre(collision_results.m_hitNormalWorld);
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

    // TODO: For prototyping only.
    void DoneButtonClicked()
    {
        HumanClientApp::GetApp()->Networking().SendMessage(
            Message(Message::COMBAT_END,
                    HumanClientApp::GetApp()->PlayerID(),
                    -1,
                    ""));
    }

    const std::map<StarType, std::set<std::string> >& StarTextures()
    {
        static std::map<StarType, std::set<std::string> > star_textures;
        if (star_textures.empty())
        {
            namespace fs = boost::filesystem;
            fs::path dir = ClientUI::ArtDir() / "combat" / "backgrounds";
            assert(fs::is_directory(dir));
            fs::directory_iterator end_it;
            for (std::map<StarType, std::string>::const_iterator type_it =
                     ClientUI::StarTypeFilePrefixes().begin();
                 type_it != ClientUI::StarTypeFilePrefixes().end();
                 ++type_it) {
                std::set<std::string>& current_textures = star_textures[type_it->first];
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
        return star_textures;
    }

    const std::map<PlanetType, std::set<std::string> >& PlanetTextures()
    {
        static std::map<PlanetType, std::set<std::string> > planet_textures;
        if (planet_textures.empty())
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
        return planet_textures;
    }

    const std::set<std::string>& AsteroidSets()
    {
        static std::set<std::string> asteroid_sets;
        if (asteroid_sets.empty())
        {
            const std::string ASTEROID_BASE_NAME = "Asteroid";
            namespace fs = boost::filesystem;
            fs::path dir = ClientUI::ArtDir() / "combat" / "meshes" / "planets";
            fs::directory_iterator end_it;
            for (fs::directory_iterator it(dir); it != end_it; ++it) {
                try {
                    if (fs::exists(*it) &&
                        !fs::is_directory(*it) &&
                        boost::algorithm::starts_with(it->filename(), ASTEROID_BASE_NAME)) {
                        asteroid_sets.insert(it->filename().substr(0, ASTEROID_BASE_NAME.size() + 2));
                    }
                } catch (const fs::filesystem_error& e) {
                    // ignore files for which permission is denied, and rethrow other exceptions
                    if (e.code() != boost::system::posix_error::permission_denied)
                        throw;
                }
            }
        }
        return asteroid_sets;
    }

    void CreateAsteroidEntities(std::vector<Ogre::Entity*>& asteroid_entities,
                                Ogre::SceneManager* scene_manager)
    {
        const std::set<std::string>& asteroid_sets = AsteroidSets();
        for (std::set<std::string>::const_iterator it = asteroid_sets.begin();
             it != asteroid_sets.end();
             ++it) {
            std::string base_name = *it;
            Ogre::Entity* entity =
                scene_manager->createEntity("asteroid mesh " + base_name,
                                            base_name + ".mesh");
            Ogre::MaterialPtr material =
                Ogre::MaterialManager::getSingleton().getByName("asteroid");
            std::string new_material_name = "asteroid material " + base_name;
            material = material->clone(new_material_name);
            material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->
                setTextureName(*it + "Color.png");
            material->getTechnique(0)->getPass(0)->getTextureUnitState(1)->
                setTextureName(*it + "Normal.png");
            entity->setMaterialName(new_material_name);
            entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
            entity->setCastShadows(true);
            asteroid_entities.push_back(entity);
        }
    }

    void SetupPagedGeometry(Forests::PagedGeometry*& paged_geometry,
                            Forests::TreeLoader3D*& paged_geometry_loader,
                            Ogre::Camera* camera)
    {
        if (!paged_geometry) {
            paged_geometry = new Forests::PagedGeometry;
            paged_geometry->setCoordinateSystem(Ogre::Vector3::UNIT_Z);
            paged_geometry->setCamera(camera);
            paged_geometry->setPageSize(250);
            paged_geometry->setInfinite();
            paged_geometry->addDetailLevel<Forests::BatchPage>(250, 50);
            paged_geometry->addDetailLevel<Forests::ImpostorPage>(
                2.0 * SystemRadius(), 1.5 * SystemRadius(),
                Ogre::Any(PAGED_GEOMETRY_IMPOSTOR_QUEUE));
            paged_geometry_loader =
                new Forests::TreeLoader3D(
                    paged_geometry,
                    Forests::TBounds(-SystemRadius(), -SystemRadius(),
                                     SystemRadius(), SystemRadius()));
            paged_geometry->setPageLoader(paged_geometry_loader);
        }
    }

    void AddOptions(OptionsDB& db)
    {
        db.AddFlag("tech-demo", "Try out the 3D combat tech demo.", false);
        db.Add("combat.enable-glow", "OPTIONS_DB_COMBAT_ENABLE_GLOW",
               true, Validator<bool>());
        db.Add("combat.enable-skybox", "OPTIONS_DB_COMBAT_ENABLE_SKYBOX",
               true, Validator<bool>());
        db.Add("combat.enable-lens-flare", "OPTIONS_DB_COMBAT_ENABLE_LENS_FLARE",
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
// CombatWnd::ShipData
////////////////////////////////////////////////////////////
CombatWnd::ShipData::ShipData() :
    m_node(0),
    m_material(),
    m_bt_mesh(0),
    m_bt_shape(0),
    m_bt_object(0)
{}

CombatWnd::ShipData::ShipData(Ogre::SceneNode* node,
                              Ogre::MaterialPtr material,
                              btTriangleMesh* bt_mesh,
                              btBvhTriangleMeshShape* bt_shape,
                              btCollisionObject* bt_object) :
    m_node(node),
    m_material(material),
    m_bt_mesh(bt_mesh),
    m_bt_shape(bt_shape),
    m_bt_object(bt_object)
{}

////////////////////////////////////////////////////////////
// CombatWnd
////////////////////////////////////////////////////////////
CombatWnd::CombatWnd(Ogre::SceneManager* scene_manager,
                     Ogre::Camera* camera,
                     Ogre::Viewport* viewport) :
    Wnd(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(), GG::INTERACTIVE),
    m_scene_manager(scene_manager),
    m_camera(camera),
    m_camera_node(m_scene_manager->getRootSceneNode()->createChildSceneNode()),
    m_viewport(viewport),
    m_volume_scene_query(m_scene_manager->createPlaneBoundedVolumeQuery(Ogre::PlaneBoundedVolumeList())),
    m_camera_animation(m_scene_manager->createAnimation("CameraTrack", CAMERA_RECENTER_TIME)),
    m_camera_animation_state(m_scene_manager->createAnimationState("CameraTrack")),
    m_distance_to_look_at_point(SystemRadius() / 2.0),
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
    m_paged_geometry(0),
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
    cf.load((ClientUI::ArtDir() / "combat" / "resources.cfg").file_string());

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
                (ClientUI::ArtDir() / path_name).file_string(),
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
    star_billboard_set->setDefaultDimensions(StarRadius() * 2.0, StarRadius() * 2.0);
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
    star_billboard_set->setDefaultDimensions(StarRadius() * 2.0, StarRadius() * 2.0);
    star_billboard_set->createBillboard(Ogre::Vector3(0.0, 0.0, 0.0));
    star_billboard_set->setVisible(true);
    star_billboard_set->setVisibilityFlags(GLOWING_OBJECTS_MASK);
    star_node->attachObject(star_billboard_set);

    Ogre::Light* star = m_scene_manager->createLight("Star");
    star->setType(Ogre::Light::LT_POINT);
    star->setPosition(Ogre::Vector3(0.0, 0.0, 0.0));
    star->setAttenuation(SystemRadius() * 0.51, 1.0, 0.0, 0.0);

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

    if (GetOptionsDB().Get<bool>("tech-demo")) {
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
        planet_types[3] = PT_ASTEROIDS;//PT_INFERNO;
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
        planet_sizes[3] = SZ_ASTEROIDS;//SZ_MEDIUM;
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

#if TEST_STATIC_OPENSTEER_OBSTACLES
        OpenSteer::AbstractObstacle* o =
            new OpenSteer::SphereObstacle(STAR_RADIUS_ADJUSTMENT_FACTOR * StarRadius(),
                                          OpenSteer::Vec3());
        g_obstacle_names[o] = "Star";
        g_pathing_engine.AddObstacle(o);
#endif

        CombatData* combat_data = new CombatData;
        combat_data->m_system = new System(star_type, planets.size(), "Sample", 0.0, 0.0);
        System& system = *combat_data->m_system;
        std::map<int, UniverseObject*>& combat_universe = combat_data->m_combat_universe;
        for (std::size_t i = 0; i < planets.size(); ++i) {
            Planet* planet = planets[i];
            GetUniverse().InsertID(planet, planet_ids[i]);
            combat_universe[planet_ids[i]] = planet;
            system.Insert(planet_ids[i], i);
            assert(system.Contains(i));
#if TEST_STATIC_OPENSTEER_OBSTACLES
            double orbit_radius = OrbitalRadius(i);
            if (planet->Type() == PT_ASTEROIDS) {
                OpenSteer::AbstractObstacle* o =
                    new AsteroidBeltObstacle(orbit_radius, AsteroidBeltRadius());
                g_obstacle_names[o] =
                    "Asteroids in orbit " + boost::lexical_cast<std::string>(i);
                g_pathing_engine.AddObstacle(o);
            } else {
                double rads =
                    planet->OrbitalPositionOnTurn(ClientApp::GetApp()->CurrentTurn());
                OpenSteer::Vec3 position(orbit_radius * std::cos(rads),
                                         orbit_radius * std::sin(rads),
                                         0);
                OpenSteer::AbstractObstacle* o =
                    new OpenSteer::SphereObstacle(PlanetRadius(planet->Size()), position);
                g_obstacle_names[o] =
                    "Planet in orbit " + boost::lexical_cast<std::string>(i);
                g_pathing_engine.AddObstacle(o);
            }
#endif
        }

        InitCombat(*combat_data);
    } else {
        GG::X width(50);
        CUIButton* done_button =
            new CUIButton((GG::GUI::GetGUI()->AppWidth() - width) / 2,
                          GG::GUI::GetGUI()->AppHeight() - GG::Y(20),
                          width,
                          "Done");
        GG::Connect(done_button->ClickedSignal, &DoneButtonClicked);
        AttachChild(done_button);
    }
}

CombatWnd::~CombatWnd()
{
    delete m_paged_geometry;

    Ogre::Root::getSingleton().removeFrameListener(this);
    m_scene_manager->removeRenderQueueListener(m_stencil_op_frame_listener);
    m_scene_manager->destroyQuery(m_volume_scene_query);
    Ogre::CompositorManager::getSingleton().removeCompositor(m_viewport, "effects/glow");

    m_scene_manager->clearScene();

    for (std::map<int, ShipData>::iterator it = m_ship_assets.begin();
         it != m_ship_assets.end(); ++it) {
        delete it->second.m_bt_mesh;
        delete it->second.m_bt_shape;
        delete it->second.m_bt_object;
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

    RemoveAccelerators();
}

void CombatWnd::InitCombat(CombatData& combat_data)
{
    m_combat_data = &combat_data;

    SetAccelerators();

    assert(StarTextures().find(m_combat_data->m_system->Star()) != StarTextures().end());
    const std::set<std::string>& star_textures =
        StarTextures().find(m_combat_data->m_system->Star())->second;

    // pick and assign star textures
    {
        std::string base_name =
            *boost::next(star_textures.begin(), m_combat_data->m_system->ID() % star_textures.size());
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

    std::vector<Ogre::Entity*> asteroid_entities;
    CreateAsteroidEntities(asteroid_entities, m_scene_manager);

    // create planets
    for (System::const_orbit_iterator it = m_combat_data->m_system->begin();
         it != m_combat_data->m_system->end();
         ++it) {
        const Planet* planet = 0;
        planet = universe_object_cast<Planet*>(m_combat_data->m_combat_universe[it->second]);
        if (planet) {
            std::string material_name = PlanetNodeMaterial(planet->Type());
            if (material_name != "asteroid") {
                std::string planet_name =
                    "orbit " + boost::lexical_cast<std::string>(it->first) + " planet";

                Ogre::SceneNode* node =
                    m_scene_manager->getRootSceneNode()->createChildSceneNode(
                        planet_name + " node");
                Ogre::Real planet_radius = PlanetRadius(planet->Size());
                node->setScale(planet_radius, planet_radius, planet_radius);
                node->yaw(Ogre::Degree(planet->AxialTilt()));
                double orbit_radius = OrbitalRadius(it->first);
                double rads =
                    planet->OrbitalPositionOnTurn(ClientApp::GetApp()->CurrentTurn());
                Ogre::Vector3 position(orbit_radius * std::cos(rads),
                                       orbit_radius * std::sin(rads),
                                       0.0);
                node->setPosition(position);

                assert(PlanetTextures().find(planet->Type()) != PlanetTextures().end());
                const std::set<std::string>& planet_textures =
                    PlanetTextures().find(planet->Type())->second;
                std::string base_name =
                    *boost::next(planet_textures.begin(), planet->ID() % planet_textures.size());

                // set up a sphere in the collision detection system
                btSphereShape* collision_shape = new btSphereShape(planet_radius);
                btCollisionObject* collision_object = new btCollisionObject;
                m_collision_shapes.insert(collision_shape);
                m_collision_objects.insert(collision_object);
                btMatrix3x3 identity;
                identity.setIdentity();
                collision_object->getWorldTransform().setBasis(identity);
                collision_object->getWorldTransform().setOrigin(
                    ToCollision(position));
                collision_object->setCollisionShape(collision_shape);
                m_collision_world->addCollisionObject(collision_object);

                if (material_name == "gas_giant") {
                    Ogre::Entity* entity =
                        m_scene_manager->createEntity(planet_name, "sphere.mesh");
                    entity->setMaterialName("gas_giant_core");
                    assert(entity->getNumSubEntities() == 1u);
                    entity->setCastShadows(true);
                    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                    node->attachObject(entity);

                    collision_object->setUserPointer(
                        static_cast<Ogre::MovableObject*>(entity));

                    entity = m_scene_manager->createEntity(
                        planet_name + " atmosphere", "sphere.mesh");
                    entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
                    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                    entity->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
                    std::string new_material_name =
                        material_name + "_" + boost::lexical_cast<std::string>(it->first);
                    Ogre::MaterialPtr material =
                        Ogre::MaterialManager::getSingleton().getByName(material_name);
                    material = material->clone(new_material_name);
                    m_planet_assets[it->first].second.push_back(material);
                    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->
                        setTextureName(base_name + ".png");
                    entity->setMaterialName(new_material_name);
                    node->attachObject(entity);
                } else {
                    Ogre::Entity* entity =
                        m_scene_manager->createEntity(planet_name, "sphere.mesh");
                    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                    std::string new_material_name =
                        material_name + "_" + boost::lexical_cast<std::string>(it->first);
                    Ogre::MaterialPtr material =
                        Ogre::MaterialManager::getSingleton().getByName(
                            material_name == "planet" ?
                            PlanetMaterialName(base_name) :
                            material_name);
                    material = material->clone(new_material_name);
                    m_planet_assets[it->first].second.push_back(material);
                    assert(entity->getNumSubEntities() == 1u);
                    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->
                        setTextureName(base_name + "Day.png");
                    material->getTechnique(0)->getPass(0)->getTextureUnitState(1)->
                        setTextureName(base_name + "Night.png");
                    entity->setMaterialName(new_material_name);
                    entity->setCastShadows(true);
                    node->attachObject(entity);

                    collision_object->setUserPointer(
                        static_cast<Ogre::MovableObject*>(entity));

                    if (material_name == "planet") {
                        material->getTechnique(0)->getPass(0)->getTextureUnitState(2)->
                            setTextureName(base_name + "CloudGloss.png");
                        entity = m_scene_manager->createEntity(
                            planet_name + " atmosphere", "sphere.mesh");
                        entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
                        entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
                        entity->setQueryFlags(UNSELECTABLE_OBJECT_MASK);
                        Ogre::MaterialPtr material =
                            Ogre::MaterialManager::getSingleton().getByName(
                                AtmosphereMaterialName(base_name));
                        entity->setMaterialName(material->getName());
                        m_planet_assets[it->first].second.push_back(material);
                        node->attachObject(entity);
                    } else {
                        assert(material_name == "atmosphereless_planet");
                        material->getTechnique(0)->getPass(0)->getTextureUnitState(2)->
                            setTextureName(base_name + "Normal.png");
                        double pop = planet->GetMeter(METER_POPULATION)->Current();
                        unsigned int lights_level = NO_CITY_LIGHTS;
                        const double MIN_POP_FOR_LIGHTS = 5.0;
                        if (MIN_POP_FOR_LIGHTS < pop) {
                            lights_level =
                                std::fmod(pop - 5.0, (100.0 - MIN_POP_FOR_LIGHTS) / 10.0);
                        }
                        Ogre::TexturePtr texture = PlanetLightsTexture(base_name, lights_level);
                        m_city_lights_textures.push_back(texture);
                        material->getTechnique(0)->getPass(0)->getTextureUnitState(3)->
                            setTextureName(texture->getName());
                    }
                }

                m_planet_assets[it->first].first = node;
            } else {
                SetupPagedGeometry(m_paged_geometry, m_paged_geometry_loader, m_camera);

                const int ASTEROIDS_IN_BELT_AT_FOURTH_ORBIT = 1000;
                const int ORBITAL_RADIUS = OrbitalRadius(it->first);
                const int ASTEROIDS =
                    ORBITAL_RADIUS / OrbitalRadius(3) * ASTEROIDS_IN_BELT_AT_FOURTH_ORBIT;

                std::string planet_name =
                    "orbit " + boost::lexical_cast<std::string>(it->first) + " planet";

                const Ogre::Real DELTA_THETA = 2.0 * Ogre::Math::PI / ASTEROIDS;
                Ogre::Real theta = 0.0;
                for (int i = 0; i < ASTEROIDS; ++i, theta += DELTA_THETA) {
                    const Ogre::Real THICKNESS = AsteroidBeltRadius() * 2.0;
                    Ogre::Radian yaw(2.0 * Ogre::Math::PI * RandZeroToOne());
                    Ogre::Vector3 position;
                    position.z = THICKNESS * (RandZeroToOne() - 0.5);
                    position.y =
                        (ORBITAL_RADIUS + THICKNESS * (RandZeroToOne() - 0.5)) * std::sin(theta);
                    position.x =
                        (ORBITAL_RADIUS + THICKNESS * (RandZeroToOne() - 0.5)) * std::cos(theta);
                    Ogre::Real scale = Ogre::Math::RangeRandom(0.05f, 0.25f);
                    m_paged_geometry_loader->addTree(
                        asteroid_entities[i % asteroid_entities.size()],
                        position, yaw, scale
                    );
                }

#if 0
                // set up a sphere in the collision detection system
                m_collision_shapes.push_back(new btSphereShape(planet_radius));
                m_collision_objects.push_back(new btCollisionObject);
                btMatrix3x3 identity;
                identity.setIdentity();
                m_collision_objects.back().getWorldTransform().setBasis(identity);
                m_collision_objects.back().getWorldTransform().setOrigin(
                    ToCollision(position));
                m_collision_objects.back().setCollisionShape(&m_collision_shapes.back());
                m_collision_world->addCollisionObject(&m_collision_objects.back());

                m_collision_objects.back().setUserPointer(
                    static_cast<Ogre::MovableObject*>(entity));
#endif
            }
        }
    }

    // create starlane entrance points
    for (System::const_lane_iterator it = m_combat_data->m_system->begin_lanes();
         it != m_combat_data->m_system->begin_lanes();
         ++it) {
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
    }
    for (std::map<int, ShipData>::iterator it = m_ship_assets.begin();
         it != m_ship_assets.end();
         ++it) {
        it->second.get<0>()->yaw(Ogre::Radian(3.14159 / 180.0 / 3.0));
    }
#endif

    RenderLensFlare();

    if (m_selection_rect.ul != m_selection_rect.lr) {
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0, 1.0, 1.0, 0.5);
        glBegin(GL_LINE_LOOP);
        glVertex(m_selection_rect.lr.x, m_selection_rect.ul.y);
        glVertex(m_selection_rect.ul.x, m_selection_rect.ul.y);
        glVertex(m_selection_rect.ul.x, m_selection_rect.lr.y);
        glVertex(m_selection_rect.lr.x, m_selection_rect.lr.y);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }

#if TEST_STATIC_OPENSTEER_OBSTACLES
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0, 0.0, 0.0, 0.67);
    glBegin(GL_QUADS);
    glVertex(GG::GUI::GetGUI()->AppWidth() / 2 - 1,
             GG::GUI::GetGUI()->AppHeight() / 2 - 1);
    glVertex(GG::GUI::GetGUI()->AppWidth() / 2 - 1,
             GG::GUI::GetGUI()->AppHeight() / 2 + 1);
    glVertex(GG::GUI::GetGUI()->AppWidth() / 2 + 1,
             GG::GUI::GetGUI()->AppHeight() / 2 + 1);
    glVertex(GG::GUI::GetGUI()->AppWidth() / 2 + 1,
             GG::GUI::GetGUI()->AppHeight() / 2 - 1);
    glEnd();
    glEnable(GL_TEXTURE_2D);
#endif
}

void CombatWnd::RenderLensFlare()
{
    if (!GetOptionsDB().Get<bool>("combat.enable-lens-flare"))
        return;

    // render two small lens flares that oppose the star's position relative to
    // the center of the viewport
    GG::Pt star_pt = ProjectToPixel(*m_camera, Ogre::Vector3(0.0, 0.0, 0.0));
    if (InClient(star_pt)) {
        Ogre::Ray ray(m_camera->getRealPosition(), -m_camera->getRealPosition());
        RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
        if (!hit.m_object) {
            GG::Pt center(Width() / 2, Height() / 2);
            GG::Pt star_to_center = center - star_pt;

            const Ogre::Real QUADRATIC_ATTENUATION_FACTOR = 5.0e-6;
            Ogre::Real attenuation =
                QUADRATIC_ATTENUATION_FACTOR * m_camera->getRealPosition().squaredLength();

            int big_flare_width =
                static_cast<int>(180 * m_star_brightness_factor / (1 + attenuation));
            int small_flare_width =
                static_cast<int>(120 * m_star_brightness_factor / (1 + attenuation));

            GG::Pt big_flare_ul =
                center + GG::Pt(star_to_center.x / 2, star_to_center.y / 2) -
                GG::Pt(GG::X(big_flare_width / 2), GG::Y(big_flare_width / 2));
            GG::Pt small_flare_ul =
                center + GG::Pt(3 * star_to_center.x / 4, 3 * star_to_center.y / 4) -
                GG::Pt(GG::X(small_flare_width / 2), GG::Y(small_flare_width / 2));
            GG::Pt big_flare_lr =
                big_flare_ul + GG::Pt(GG::X(big_flare_width), GG::Y(big_flare_width));
            GG::Pt small_flare_lr =
                small_flare_ul + GG::Pt(GG::X(small_flare_width), GG::Y(small_flare_width));

            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            m_big_flare->OrthoBlit(big_flare_ul, big_flare_lr,
                                   m_big_flare->DefaultTexCoords());
            m_small_flare->OrthoBlit(small_flare_ul, small_flare_lr,
                                     m_small_flare->DefaultTexCoords());
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
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
    Ogre::NodeAnimationTrack* track =
        m_camera_animation->createNodeTrack(LOOKAT_NODE_TRACK_HANDLE, m_camera_node);
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

void CombatWnd::Zoom(int move, GG::Flags<GG::ModKey> mod_keys)
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

void CombatWnd::HandleRotation(const GG::Pt& delta)
{
    Ogre::Radian delta_pitch =
        Value(-delta.y * 1.0 / GG::GUI::GetGUI()->AppHeight()) * Ogre::Radian(Ogre::Math::PI);
    m_pitch += delta_pitch;
    if (m_pitch < Ogre::Radian(0.0))
        m_pitch = Ogre::Radian(0.0);
    if (Ogre::Radian(Ogre::Math::HALF_PI) < m_pitch)
        m_pitch = Ogre::Radian(Ogre::Math::HALF_PI);
    Ogre::Radian delta_roll =
        Value(-delta.x * 1.0 / GG::GUI::GetGUI()->AppWidth()) * Ogre::Radian(Ogre::Math::PI);
    m_roll += delta_roll;

    UpdateCameraPosition();
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
    if (!m_mouse_dragged && !m_camera_animation->hasNodeTrack(LOOKAT_NODE_TRACK_HANDLE)) {
        if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
            Ogre::SceneNode* clicked_scene_node = movable_object->getParentSceneNode();
            assert(clicked_scene_node);
            LookAt(clicked_scene_node);
        } else {
            Ogre::Ray ray = m_camera->getCameraToViewportRay(Value(pt.x * 1.0 / GG::GUI::GetGUI()->AppWidth()),
                                                             Value(pt.y * 1.0 / GG::GUI::GetGUI()->AppHeight()));
            std::pair<bool, Ogre::Real> intersection =
                Ogre::Math::intersects(ray, Ogre::Plane(Ogre::Vector3::UNIT_Z, Ogre::Vector3::ZERO));
            const Ogre::Real MAX_DISTANCE_SQ = SystemRadius() * SystemRadius();
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
        HandleRotation(delta_pos);
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
    if (move)
        Zoom(move, mod_keys);
}

void CombatWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    // TODO: This quick-quit is for prototyping only.
    if (key == GG::GGK_q && mod_keys & GG::MOD_KEY_CTRL)
        m_exit = true;

    const int SCALE = 5;
    switch (key) {
    case GG::GGK_UP: HandleRotation(GG::Pt(GG::X0, GG::Y(SCALE))); break;
    case GG::GGK_DOWN: HandleRotation(GG::Pt(GG::X0, GG::Y(-SCALE))); break;
    case GG::GGK_RIGHT: HandleRotation(GG::Pt(GG::X(2 * -SCALE), GG::Y0)); break;
    case GG::GGK_LEFT: HandleRotation(GG::Pt(GG::X(2 * SCALE), GG::Y0)); break;
    default: break;
    }
}

void CombatWnd::ShipPlaced(const CombatShipPtr &ship)
{ AddShip(ship); }

void CombatWnd::ShipFired(const CombatShipPtr &ship,
                          const CombatObjectPtr &target,
                          const std::string& part_name)
{
    // TODO
}

void CombatWnd::ShipDestroyed(const CombatShipPtr &ship)
{ RemoveShip(ship); }

void CombatWnd::ShipEnteredStarlane(const CombatShipPtr &ship)
{
    // TODO
}

void CombatWnd::FighterLaunched(const CombatFighterPtr &fighter)
{
    // TODO
}

void CombatWnd::FighterFired(const CombatFighterPtr &fighter,
                             const CombatObjectPtr &target)
{
    // TODO
}

void CombatWnd::FighterDestroyed(const CombatFighterPtr &fighter)
{
    // TODO
}

void CombatWnd::FighterDocked(const CombatFighterPtr &fighter)
{
    // TODO
}

void CombatWnd::MissileLaunched(const MissilePtr &missile)
{
    // TODO
}

void CombatWnd::MissileExploded(const MissilePtr &missile)
{
    // TODO
}

void CombatWnd::MissileRemoved(const MissilePtr &missile)
{
    // TODO
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
    if (m_camera_animation_state->hasEnded()) {
        // TODO: verify that the move actually happened, by placing the camera
        // at its final destination.  This is necessary, because sometimes
        // low-enough frame rates mean that the move isn't completed, or
        // doesn't happen at all.
        m_camera_animation->destroyAllTracks();
    }

    if (m_paged_geometry)
        m_paged_geometry->update();

    // TODO: For each combat entity, update its position and orientation based
    // on the corresponding Combat* object's position and orientation.

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
#if TEST_STATIC_OPENSTEER_OBSTACLES
    std::cout << "testing...\n";
    Ogre::Ray ray = m_camera->getCameraToViewportRay(0.5, 0.5);
    FakeVehicle vehicle;
    vehicle.reset();
    vehicle.regenerateOrthonormalBasis(ToOpenSteer(ray.getDirection()),
                                       OpenSteer::Vec3(0, 0, 1));
    vehicle.setPosition(ToOpenSteer(ray.getOrigin()));
    const PathingEngine::ObstacleVec& obstacles = g_pathing_engine.m_obstacles;
    for (PathingEngine::ObstacleVec::const_iterator it = obstacles.begin();
         it != obstacles.end();
         ++it) {
        OpenSteer::AbstractObstacle::PathIntersection pi;
        it->findIntersectionWithVehiclePath(vehicle, pi);
        if (pi.intersect)
            std::cout << "    Hit " << g_obstacle_names[&*it] << "\n";
    }
    std::cerr << '\n';
#endif
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
        const Ogre::Real SAMPLE_INCREMENT =
            StarRadius() * STAR_RADIUS_ADJUSTMENT_FACTOR * STAR_CORE_SCALE_FACTOR / SAMPLES_PER_SIDE;

        bool occlusions[TOTAL_SAMPLES];
        // left side positions
        for (int i = 0; i < SAMPLES_PER_SIDE; ++i) {
            Ogre::Vector3 direction =
                SUN_CENTER + (SAMPLES_PER_SIDE - i) * SAMPLE_INCREMENT * -RIGHT - CAMERA_POS;
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
            Ogre::Vector3 direction =
                SUN_CENTER + (i + 1) * SAMPLE_INCREMENT * RIGHT - CAMERA_POS;
            Ogre::Ray ray(CAMERA_POS, direction);
            RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
            occlusions[SAMPLES_PER_SIDE + 1 + i] = false;
            if (hit.m_object) {
                Ogre::Real distance_squared = (hit.m_point - CAMERA_POS).squaredLength();
                occlusions[SAMPLES_PER_SIDE + 1 + i] =
                    distance_squared < direction.squaredLength();
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
        m_left_horizontal_flare_scroll_offset =
            (SAMPLES_PER_SIDE - occlusion_params.get<0>()) * SAMPLE_INCREMENT /
            STAR_RADIUS_ADJUSTMENT_FACTOR / (2.0 * StarRadius());
        m_right_horizontal_flare_scroll_offset =
            -(SAMPLES_PER_SIDE - occlusion_params.get<1>()) * SAMPLE_INCREMENT /
            STAR_RADIUS_ADJUSTMENT_FACTOR / (2.0 * StarRadius());
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
        1.0 - angle_at_view_center_to_star.valueRadians() /
        (m_camera->getFOVy() / 2.0).valueRadians();
    m_star_brightness_factor =
        BRIGHTNESS_AT_MAX_FOVY + center_nearness_factor * (1.0 - BRIGHTNESS_AT_MAX_FOVY);
    // Raise the factor to a (smallish) power to create some nonlinearity in the scaling.
    m_star_brightness_factor = Ogre::Math::Pow(m_star_brightness_factor, 1.5);
    m_star_back_billboard->setColour(
        Ogre::ColourValue(1.0, 1.0, 1.0, m_star_brightness_factor));

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

void CombatWnd::AddShip(const CombatShipPtr& combat_ship)
{
    const Ship& ship = combat_ship->GetShip();
    std::string mesh_name = ship.Design()->Model();
    Ogre::Entity* entity = m_scene_manager->createEntity("ship_" + mesh_name, mesh_name);
    entity->setCastShadows(true);
    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
    entity->setMaterialName("ship");
    Ogre::SceneNode* node =
        m_scene_manager->getRootSceneNode()->createChildSceneNode("ship_" + mesh_name + "_node");
    node->attachObject(entity);
    node->setUserAny(Ogre::Any(combat_ship));

    // TODO: This is only here because the Durgha model is upside down.  Remove
    // it when this is fixed.
    node->yaw(Ogre::Radian(Ogre::Math::PI));

    node->setPosition(ToOgre(combat_ship->position()));
    node->setOrientation(Ogre::Quaternion(ToOgre(combat_ship->side()),
                                          ToOgre(combat_ship->forward()),
                                          ToOgre(combat_ship->up())));

    CollisionMeshConverter collision_mesh_converter(entity);
    btTriangleMesh* collision_mesh = 0;
    btBvhTriangleMeshShape* collision_shape = 0;
    boost::tie(collision_mesh, collision_shape) = collision_mesh_converter.CollisionShape();

    Ogre::MaterialPtr material =
        Ogre::MaterialManager::getSingleton().getByName("ship");

    m_collision_shapes.insert(collision_shape);
    btCollisionObject* collision_object = new btCollisionObject;
    m_collision_objects.insert(collision_object);
    btMatrix3x3 identity;
    identity.setIdentity();
    // TODO: Remove z-flip scaling when models are right.
    btMatrix3x3 scaled = identity.scaled(btVector3(1.0, 1.0, -1.0));
    collision_object->getWorldTransform().setBasis(scaled);
    collision_object->getWorldTransform().setOrigin(ToCollision(node->getPosition()));
    collision_object->setCollisionShape(collision_shape);
    m_collision_world->addCollisionObject(collision_object);
    collision_object->setUserPointer(static_cast<Ogre::MovableObject*>(entity));

    m_ship_assets[ship.ID()] =
        ShipData(node, material, collision_mesh, collision_shape, collision_object);
}

void CombatWnd::RemoveShip(const CombatShipPtr& combat_ship)
{
    ShipData& ship_data = m_ship_assets[combat_ship->GetShip().ID()];
    m_scene_manager->destroySceneNode(ship_data.m_node);
    m_collision_world->getCollisionObjectArray().remove(ship_data.m_bt_object);
    delete ship_data.m_bt_mesh;
    delete ship_data.m_bt_shape;
    delete ship_data.m_bt_object;
    m_collision_shapes.erase(ship_data.m_bt_shape);
    m_collision_objects.erase(ship_data.m_bt_object);
    m_ship_assets.erase(combat_ship->GetShip().ID());
}

bool CombatWnd::OpenChatWindow()
{
    bool retval = true;
    if (GetChatWnd()->OpenForInput())
        DisableAlphaNumAccels();
    else
        retval = false;
    return retval;
}

bool CombatWnd::EndTurn()
{
    // TODO
    return true;
}

bool CombatWnd::ShowMenu()
{
    if (!m_menu_showing) {
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
    return true;
}

bool CombatWnd::KeyboardZoomIn()
{
    Zoom(1, GG::Flags<GG::ModKey>());
    return true;
}

bool CombatWnd::KeyboardZoomOut()
{
    Zoom(-1, GG::Flags<GG::ModKey>());
    return true;
}

bool CombatWnd::ZoomToPrevIdleUnit()
{
    // TODO
    return true;
}

bool CombatWnd::ZoomToNextIdleUnit()
{
    // TODO
    return true;
}

bool CombatWnd::ZoomToPrevUnit()
{
    // TODO
    return true;
}

bool CombatWnd::ZoomToNextUnit()
{
    // TODO
    return true;
}

void CombatWnd::ConnectKeyboardAcceleratorSignals()
{
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_RETURN),
                    &CombatWnd::OpenChatWindow, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_ENTER),
                    &CombatWnd::OpenChatWindow, this));

    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_RETURN, GG::MOD_KEY_CTRL),
                    &CombatWnd::EndTurn, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_ENTER, GG::MOD_KEY_CTRL),
                    &CombatWnd::EndTurn, this));

    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F10),
                    &CombatWnd::ShowMenu, this));

    // Keys for zooming
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_e),
                    &CombatWnd::KeyboardZoomIn, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_PLUS),
                    &CombatWnd::KeyboardZoomIn, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_r),
                    &CombatWnd::KeyboardZoomOut, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_MINUS),
                    &CombatWnd::KeyboardZoomOut, this));

    // Keys for showing units
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_f),
                    &CombatWnd::ZoomToPrevIdleUnit, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_g),
                    &CombatWnd::ZoomToNextIdleUnit, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_v),
                    &CombatWnd::ZoomToPrevUnit, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_b),
                    &CombatWnd::ZoomToNextUnit, this));
}

void CombatWnd::SetAccelerators()
{
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_RETURN);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_ENTER);

    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_RETURN, GG::MOD_KEY_CTRL);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_ENTER, GG::MOD_KEY_CTRL);

    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_F10);

    // Keys for zooming
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_e);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_r);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_PLUS);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_MINUS);

    // Keys for showing units
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_f);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_g);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_v);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_b);

    ConnectKeyboardAcceleratorSignals();
}

void CombatWnd::RemoveAccelerators()
{
    GG::GUI::accel_iterator i = GG::GUI::GetGUI()->accel_begin();
    while (i != GG::GUI::GetGUI()->accel_end()) {
        GG::GUI::GetGUI()->RemoveAccelerator(i);
        i = GG::GUI::GetGUI()->accel_begin();
    }
    m_disabled_accels_list.clear();

    for (std::set<boost::signals::connection>::iterator it =
             m_keyboard_accelerator_signals.begin();
         it != m_keyboard_accelerator_signals.end();
         ++it) {
        it->disconnect();
    }
    m_keyboard_accelerator_signals.clear();
}

void CombatWnd::DisableAlphaNumAccels()
{
    for (GG::GUI::const_accel_iterator i = GG::GUI::GetGUI()->accel_begin();
         i != GG::GUI::GetGUI()->accel_end(); ++i) {
        if (i->second != 0) // we only want to disable mod_keys without modifiers
            continue; 
        GG::Key key = i->first;
        if ((key >= GG::GGK_a && key <= GG::GGK_z) || 
            (key >= GG::GGK_0 && key <= GG::GGK_9)) {
            m_disabled_accels_list.insert(key);
        }
    }
    for (std::set<GG::Key>::iterator i = m_disabled_accels_list.begin();
         i != m_disabled_accels_list.end(); ++i) {
        GG::GUI::GetGUI()->RemoveAccelerator(*i);
    }
}

void CombatWnd::EnableAlphaNumAccels()
{
    for (std::set<GG::Key>::iterator i = m_disabled_accels_list.begin();
         i != m_disabled_accels_list.end(); ++i) {
        GG::GUI::GetGUI()->SetAccelerator(*i);
    }
    m_disabled_accels_list.clear();
}

void CombatWnd::ChatMessageSentSlot()
{
    if (!m_disabled_accels_list.empty()) {
        EnableAlphaNumAccels();
        GG::GUI::GetGUI()->SetFocusWnd(this);
    }
}
