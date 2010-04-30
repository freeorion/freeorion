#include "CombatWnd.h"

#include "ChatWnd.h"
#include "ClientUI.h"
#include "CollisionMeshConverter.h"
#include "CombatCamera.h"
#include "CombatSetupWnd.h"
#include "CUIControls.h"
#include "InGameMenu.h"
#include "../combat/OpenSteer/CombatFighter.h"
#include "../combat/OpenSteer/CombatShip.h"
#include "../combat/OpenSteer/Missile.h"
#include "../combat/OpenSteer/PathingEngine.h"
#include "../Empire/Empire.h"
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

// TODO: Remove this.  It is only here for prototyping.
#include "../universe/Fleet.h"

#include "PagedGeometry/BatchPage.h"
#include "PagedGeometry/ImpostorPage.h"
#include "PagedGeometry/PagedGeometry.h"
#include "PagedGeometry/TreeLoader3D.h"

#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <OgreCompositorManager.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>
#include <OgreRoot.h>
#include <OgreRenderQueueListener.h>
#include <OgreRenderSystem.h>
#include <OgreRenderTarget.h>
#include <OgreSceneQuery.h>
#include <OgreSubEntity.h>

#include <btBulletCollisionCommon.h>

#include <GG/GUI.h>

#include <boost/cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>


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

// query masks
const Ogre::uint32 UNSELECTABLE_OBJECT_MASK = 1 << 0;

namespace {
    const GG::Pt INVALID_SELECTION_DRAG_POS(-GG::X1, -GG::Y1);

    // collision dection system params
    btVector3 WORLD_AABB_MIN(-SystemRadius(), -SystemRadius(), -SystemRadius() / 10.0);
    btVector3 WORLD_AABB_MAX(SystemRadius(), SystemRadius(), SystemRadius() / 10.0);

    // visibility masks
    const Ogre::uint32 REGULAR_OBJECTS_MASK = 1 << 0;
    const Ogre::uint32 GLOWING_OBJECTS_MASK = 1 << 1;

    // stencil masks
    const Ogre::uint32 OUTLINE_SELECTION_HILITING_STENCIL_VALUE = 1 << 0;
    const Ogre::uint32 FULL_SELECTION_HILITING_STENCIL_VALUE    = 1 << 1;

    const unsigned int NO_CITY_LIGHTS = std::numeric_limits<unsigned int>::max();

    // HACK! The currently-used star cores only cover part of the texture.
    // Here, we adjust for this, so that the edge of the star as it appears
    // onscreen is actually what we use for the star radius below.
    const Ogre::Real STAR_RADIUS_ADJUSTMENT_FACTOR = 0.45;

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

    btQuaternion ToCollision(const Ogre::Quaternion& q)
    { return btQuaternion(q.x, q.y, q.z, q.w); }

    btVector3 ToCollision(const OpenSteer::Vec3& vec)
    { return btVector3(vec.x, vec.y, vec.z); }

    Ogre::Vector3 ToOgre(const btVector3& vec)
    { return Ogre::Vector3(vec.x(), vec.y(), vec.z()); }

    OpenSteer::Vec3 ToOpenSteer(const btVector3& vec)
    { return OpenSteer::Vec3(vec.x(), vec.y(), vec.z()); }

    void SetNodePositionAndOrientation(Ogre::SceneNode* node, const CombatObjectPtr& combat_object)
    {
        node->setPosition(::ToOgre(combat_object->position()));
        node->setOrientation(Ogre::Quaternion(::ToOgre(combat_object->side()),
                                              ::ToOgre(combat_object->forward()),
                                              ::ToOgre(combat_object->up())));
    }

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
                              ToCollision(ray.getPoint(SystemRadius() * 10.0)));
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

    std::pair<std::string, int> PlanetLightsChannel(const std::string& base_name, const Planet& planet)
    {
        std::pair<std::string, int> retval;

        double pop = planet.GetMeter(METER_POPULATION)->Current();
        unsigned int lights_level = NO_CITY_LIGHTS;
        const double MIN_POP_FOR_LIGHTS = 5.0;
        if (MIN_POP_FOR_LIGHTS < pop)
            lights_level = std::fmod(pop - 5.0, (100.0 - MIN_POP_FOR_LIGHTS) / 10.0);

        assert(lights_level == NO_CITY_LIGHTS || lights_level < 10);

        retval.first = base_name;
        if (lights_level == NO_CITY_LIGHTS) {
            retval.first += "LightsA.png";
            retval.second = -1;
        } else if (lights_level < 4) {
            retval.first += "LightsA.png";
            retval.second = lights_level;
        } else if (lights_level < 8) {
            retval.first += "LightsB.png";
            retval.second = lights_level - 4;
        } else if (lights_level == 8) {
            retval.first += "Day.png";
            retval.second = 3;
        } else {
            retval.first += "Night.png";
            retval.second = 3;
        }

        return retval;
    }

    // TODO: For prototyping only.
    void EndCombatButtonClicked()
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
                            const Ogre::Camera* camera)
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

    std::string ShipIDString(const Ship& ship)
    { return "ship_" + boost::lexical_cast<std::string>(ship.ID()) + "_"; }

    std::string ShipMeshName(const Ship& ship)
    { return ship.Design()->Model() + ".mesh"; }

    bool CloseTo(const GG::Pt& p1, const GG::Pt& p2)
    {
        const int EPSILON = 5;
        int delta = std::abs(Value(p1.x - p2.x)) + std::abs(Value(p1.y - p2.y));
        return delta < EPSILON;
    }

    const Ogre::Vector3& GetSystemColor(const std::string& star_or_skybox_base_name)
    {
        static std::map<std::string, Ogre::Vector3> colors;
        if (colors.empty()) {
            boost::filesystem::ifstream ifs(ClientUI::ArtDir() / "combat" / "backgrounds" / "system_colors.txt");
            std::string line, name;
            StreamableColor clr;
            while (ifs) {
                std::getline(ifs, line);
                if (!line.empty() && line[0] != '#') {
                    std::stringstream ss(line.c_str());
                    ss >> name >> clr;
                    colors[name] = Ogre::Vector3(clr.r / 255.0, clr.g / 255.0, clr.b / 255.0);
                }
            }
        }
        return colors[star_or_skybox_base_name];
    }

    void AddOptions(OptionsDB& db)
    {
        db.AddFlag("tech-demo",             "OPTIONS_DB_TECH_DEMO",                 false);
        db.Add("combat.enable-glow",        "OPTIONS_DB_COMBAT_ENABLE_GLOW",
               true, Validator<bool>());
        db.Add("combat.enable-skybox",      "OPTIONS_DB_COMBAT_ENABLE_SKYBOX",
               true, Validator<bool>());
        db.Add("combat.enable-lens-flare",  "OPTIONS_DB_COMBAT_ENABLE_LENS_FLARE",
               true, Validator<bool>());
        db.Add("combat.filled-selection",   "OPTIONS_DB_COMBAT_FILLED_SELECTION",
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
    m_viewport(viewport),
    m_volume_scene_query(m_scene_manager->createPlaneBoundedVolumeQuery(Ogre::PlaneBoundedVolumeList())),
    m_camera(0),
    m_ogre_camera(camera),
    m_last_pos(),
    m_last_click_pos(),
    m_selection_drag_start(INVALID_SELECTION_DRAG_POS),
    m_selection_drag_stop(INVALID_SELECTION_DRAG_POS),
    m_mouse_dragged(false),
    m_selection_rect(),
    m_star_back_billboard(0),
    m_star_brightness_factor(1.0),
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
    m_end_turn_button(new CUIButton(GG::X0, GG::Y0, GG::X(75), UserString("TURN"))),
    m_time_since_last_turn_update(0.0),
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

    // set up collision detection system
    m_collision_configuration = new btDefaultCollisionConfiguration;
    m_collision_dispatcher = new btCollisionDispatcher(m_collision_configuration);
    m_collision_broadphase = new bt32BitAxisSweep3(WORLD_AABB_MIN, WORLD_AABB_MAX);
    m_collision_world =
        new btCollisionWorld(m_collision_dispatcher, m_collision_broadphase, m_collision_configuration);

    // look at the star initially
    m_camera = new CombatCamera(*camera, m_scene_manager, star_node);
    GG::Connect(m_camera->CameraChangedSignal, &CombatWnd::UpdateStarFromCameraPosition, this);

    m_end_turn_button->MoveTo(
        GG::Pt(GG::X(5), GG::GUI::GetGUI()->AppHeight() - m_end_turn_button->Height() - GG::Y(5)));
    GG::Connect(m_end_turn_button->ClickedSignal, boost::bind(&CombatWnd::EndTurn, this));
    m_end_turn_button->Hide();

    AttachChild(m_end_turn_button);
    AttachChild(m_fps_text);

    if (GetOptionsDB().Get<bool>("tech-demo")) {
        //////////////////////////////////////////////////////////////////
        // NOTE: This is temporary code for combat system prototyping!  //
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
        }

        std::vector<CombatSetupGroup> setup_groups;
        InitCombat(*combat_data, setup_groups);
    } else {
        // TODO: For prototyping only.
        GG::X width(150);
        CUIButton* done_button =
            new CUIButton(GG::GUI::GetGUI()->AppWidth() - width - GG::X(5),
                          GG::GUI::GetGUI()->AppHeight() - GG::Y(25),
                          width,
                          "End Combat");
        GG::Connect(done_button->ClickedSignal, &EndCombatButtonClicked);
        AttachChild(done_button);

        // TODO: Add permanent (i.e. not just for prototyping) button for
        // combat auto-resolution.
    }
}

CombatWnd::~CombatWnd()
{
    delete m_paged_geometry;

    delete m_camera;

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

    m_collision_shapes.clear();
    m_collision_objects.clear();
    delete m_collision_world;
    delete m_collision_broadphase;
    delete m_collision_dispatcher;
    delete m_collision_configuration;

    RemoveAccelerators();
}

void CombatWnd::InitCombat(CombatData& combat_data, const std::vector<CombatSetupGroup>& setup_groups)
{
    m_combat_data = &combat_data;

    SetAccelerators();

    const std::string& base_name = StarBaseName();

    // pick and assign star textures
    {
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
                        std::pair<std::string, int> lights_channel = PlanetLightsChannel(base_name, *planet);
                        material->getTechnique(0)->getPass(0)->getTextureUnitState(3)->
                            setTextureName(lights_channel.first);
                        material->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->
                            setNamedConstant("lights_channel", lights_channel.second);
                    }
                }

                m_planet_assets[it->first].first = node;
            } else {
                SetupPagedGeometry(m_paged_geometry, m_paged_geometry_loader, m_ogre_camera);

                const int ASTEROIDS_IN_BELT_AT_FOURTH_ORBIT = 1000;
                const int ORBITAL_RADIUS = OrbitalRadius(it->first);
                const int ASTEROIDS =
                    ORBITAL_RADIUS / OrbitalRadius(3) * ASTEROIDS_IN_BELT_AT_FOURTH_ORBIT;

                std::string planet_name =
                    "orbit " + boost::lexical_cast<std::string>(it->first) + " planet";

                const Ogre::Real DELTA_THETA = Ogre::Math::TWO_PI / ASTEROIDS;
                Ogre::Real theta = 0.0;
                for (int i = 0; i < ASTEROIDS; ++i, theta += DELTA_THETA) {
                    const Ogre::Real THICKNESS = AsteroidBeltRadius() * 2.0;
                    Ogre::Radian yaw(Ogre::Math::TWO_PI * RandZeroToOne());
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
            }
        }
    }

    // create starlane entrance points
    for (System::const_lane_iterator it = m_combat_data->m_system->begin_lanes();
         it != m_combat_data->m_system->end_lanes();
         ++it) {
        // this will break if/when we add support for wormholes, so we'll know to fix this code
        assert(!it->second);

        std::string starlane_id_str = boost::lexical_cast<std::string>(it->first);
        Ogre::SceneNode* node =
            m_scene_manager->getRootSceneNode()->createChildSceneNode(
                starlane_id_str + "_starlane_entrance_node");
        double theta = StarlaneEntranceOrbitalPosition(it->first, m_combat_data->m_system->ID());
        double r = SystemRadius() + 25.0;
        Ogre::Vector3 position(r * std::cos(theta), r * std::sin(theta), 0.0);
        node->setPosition(position);
        node->setOrientation(StarwardOrientationForPosition(position));

        Ogre::Entity* entity =
            m_scene_manager->createEntity(starlane_id_str + "_starlane_entity", "starlane_entry.mesh");
        entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
        entity->setMaterialName("starlane_entry");
        entity->setRenderQueueGroup(ALPHA_OBJECTS_QUEUE);
        node->attachObject(entity);
    }

    m_combat_setup_wnd =
        new CombatSetupWnd(setup_groups, this, m_combat_data, m_scene_manager,
                           boost::bind(&CombatCamera::IntersectMouseWithEcliptic, m_camera, _1),
                           boost::bind(&CombatWnd::GetShipMaterial, this, _1),
                           boost::bind(&CombatWnd::AddShipNode, this, _1, _2, _3, _4),
                           boost::bind(&CombatWnd::GetObjectUnderPt, this, _1),
                           boost::bind(&CombatWnd::RepositionShipNode, this, _1, _2, _3),
                           boost::bind(&CombatWnd::RemoveShip, this, _1),
                           boost::bind(&CombatCamera::LookAtPosition, m_camera, _1));
    AttachChild(m_combat_setup_wnd);
}

void CombatWnd::CombatTurnUpdate(CombatData& combat_data)
{
    m_combat_data = &combat_data;

    if (m_combat_setup_wnd) {
        delete m_combat_setup_wnd;
        m_combat_setup_wnd = 0;
    }

    for (PathingEngine::const_iterator it = m_combat_data->m_pathing_engine.begin();
         it != m_combat_data->m_pathing_engine.end();
         ++it) {
        if ((*it)->IsShip()) {
            assert(boost::dynamic_pointer_cast<CombatShip>(*it));
            CombatShipPtr combat_ship = boost::static_pointer_cast<CombatShip>(*it);
            combat_ship->SetListener(*this);
            Ship& ship = combat_ship->GetShip();
            std::map<int, ShipData>::iterator ship_data_it = m_ship_assets.find(ship.ID());
            if (ship_data_it == m_ship_assets.end())
                AddCombatShip(combat_ship);
            else
                UpdateObjectPosition(*it);
        }
    }

    if (m_combat_data->m_combat_turn_number)
        m_combat_data->m_pathing_engine.TurnStarted(m_combat_data->m_combat_turn_number);

    m_end_turn_button->Disable(false);
    m_time_since_last_turn_update = 0.0;

    m_end_turn_button->Show();

    // TODO: Handle object removals.
}

void CombatWnd::Render()
{
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
}

void CombatWnd::RenderLensFlare()
{
    if (!GetOptionsDB().Get<bool>("combat.enable-lens-flare"))
        return;

    // render two small lens flares that oppose the star's position relative to
    // the center of the viewport
    GG::Pt star_pt = m_camera->ProjectToPixel(Ogre::Vector3(0.0, 0.0, 0.0));
    if (InClient(star_pt)) {
        Ogre::Ray ray(m_camera->GetRealPosition(), -m_camera->GetRealPosition());
        RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
        if (!hit.m_object) {
            GG::Pt center(Width() / 2, Height() / 2);
            GG::Pt star_to_center = center - star_pt;

            const Ogre::Real QUADRATIC_ATTENUATION_FACTOR = 5.0e-6;
            Ogre::Real attenuation =
                QUADRATIC_ATTENUATION_FACTOR * m_camera->GetRealPosition().squaredLength();

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

void CombatWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_selection_drag_start != INVALID_SELECTION_DRAG_POS) {
        SelectObjectsInVolume(mod_keys & GG::MOD_KEY_CTRL);
        EndSelectionDrag();
    } else if (!m_mouse_dragged) {
        if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
            assert(movable_object->getParentSceneNode());
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
    m_last_click_pos = pt;
}

void CombatWnd::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (CloseTo(pt, m_last_click_pos)) {
        if (!m_mouse_dragged && !m_camera->Moving()) {
            if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
                Ogre::SceneNode* clicked_scene_node = movable_object->getParentSceneNode();
                assert(clicked_scene_node);
                m_camera->LookAtNode(clicked_scene_node);
            } else {
                std::pair<bool, Ogre::Vector3> intersection = m_camera->IntersectMouseWithEcliptic(pt);
                if (intersection.first)
                    m_camera->LookAtPosition(intersection.second);
            }
        }
    } else {
        LClick(pt, mod_keys);
    }
    m_last_click_pos = GG::Pt();
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
        m_camera->HandleRotation(delta_pos);
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
    if (!m_current_selections.empty()) {
        bool append = mod_keys & GG::MOD_KEY_SHIFT;

        if (Ogre::MovableObject* movable_object = GetObjectUnderPt(pt)) {
            Ogre::SceneNode* node = movable_object->getParentSceneNode();
            assert(node);
            // TODO: Handle attacking planets.  For now, one can only
            // explicitly attack ships and fighters with ships and fighters.
            if (const CombatObjectPtr* target = Ogre::any_cast<CombatObjectPtr>(&node->getUserAny())) {
                // TODO: use friends/enemies considerations later
                bool attack = (*target)->Owner() != HumanClientApp::GetApp()->PlayerID();
                for (std::map<Ogre::MovableObject*, SelectedObject>::iterator it =
                         m_current_selections.begin();
                     it != m_current_selections.end();
                     ++it) {
                    Ogre::SceneNode* node = it->first->getParentSceneNode();
                    assert(node);
                    const CombatObjectPtr* combat_object_ =
                        Ogre::any_cast<CombatObjectPtr>(&node->getUserAny());

                    // TODO: Handle planets (really, their defenses) attacking
                    // objects.  For now, one can only explicitly attack ships
                    // and fighters with ships and fighters.
                    if (!combat_object_ || (*combat_object_)->Owner() != HumanClientApp::GetApp()->PlayerID())
                        continue;

                    // don't defend yourself
                    if (*combat_object_ == *target)
                        continue;

                    CombatObjectPtr combat_object = *combat_object_;

                    if (combat_object->IsShip()) {
                        assert(boost::dynamic_pointer_cast<CombatShip>(combat_object));
                        CombatShipPtr combat_ship = boost::static_pointer_cast<CombatShip>(combat_object);
                        m_combat_order_set.push_back(
                            CombatOrder(
                                combat_ship->GetShip().ID(),
                                ShipMission(attack ?
                                            ShipMission::ATTACK_THIS : ShipMission::DEFEND_THIS,
                                            *target),
                                append));
                    } else if (combat_object->IsFighter()) {
                        assert(boost::dynamic_pointer_cast<CombatFighter>(combat_object));
                        CombatFighterPtr combat_fighter =
                            boost::static_pointer_cast<CombatFighter>(combat_object);
                        m_combat_order_set.push_back(
                            CombatOrder(
                                combat_fighter->ID(),
                                FighterMission(attack ?
                                               FighterMission::ATTACK_THIS : FighterMission::DEFEND_THIS,
                                               *target),
                                append));
                    }
                }
            }
        } else if (0 /* TODO: if starlane clicked */) {
            // TODO: queue append/replace MOVE_TO starlane, then queue append ENTER_STARLANE
        } else {
            std::pair<bool, Ogre::Vector3> intersection = m_camera->IntersectMouseWithEcliptic(pt);
            if (intersection.first) {
                bool patrol = mod_keys & GG::MOD_KEY_CTRL;
                for (std::map<Ogre::MovableObject*, SelectedObject>::iterator it =
                         m_current_selections.begin();
                     it != m_current_selections.end();
                     ++it) {
                    Ogre::SceneNode* node = it->first->getParentSceneNode();
                    assert(node);
                    const CombatObjectPtr* combat_object_ =
                        Ogre::any_cast<CombatObjectPtr>(&node->getUserAny());

                    if (!combat_object_ || (*combat_object_)->Owner() != HumanClientApp::GetApp()->PlayerID())
                        continue;

                    CombatObjectPtr combat_object = *combat_object_;

                    if (combat_object->IsShip()) {
                        assert(boost::dynamic_pointer_cast<CombatShip>(combat_object));
                        CombatShipPtr combat_ship = boost::static_pointer_cast<CombatShip>(combat_object);
                        m_combat_order_set.push_back(
                            CombatOrder(
                                combat_ship->GetShip().ID(),
                                ShipMission(patrol ? ShipMission::PATROL_TO : ShipMission::MOVE_TO,
                                            ToOpenSteer(intersection.second)),
                                append));
                    } else if (combat_object->IsFighter()) {
                        assert(boost::dynamic_pointer_cast<CombatFighter>(combat_object));
                        CombatFighterPtr combat_fighter =
                            boost::static_pointer_cast<CombatFighter>(combat_object);
                        m_combat_order_set.push_back(
                            CombatOrder(
                                combat_fighter->ID(),
                                FighterMission(patrol ? FighterMission::PATROL_TO : FighterMission::MOVE_TO,
                                               ToOpenSteer(intersection.second)),
                                append));
                    }
                }
            }
        }
    }
}

void CombatWnd::RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    // TODO: for all below, shift-double-click means append to queue, other
    // double-clicks mean replace queue

    // TODO: treat double-click on target as ATTACK_THIS_STANDOFF for ships,
    // and treat double-click on base as RETURN_TO_BASE
}

void CombatWnd::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ m_camera->MouseWheel(pt, move, mod_keys); }

void CombatWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    // TODO: This quick-quit is for prototyping only.
    if (key == GG::GGK_q && mod_keys & GG::MOD_KEY_CTRL)
        m_exit = true;

    const int SCALE = 5;
    switch (key) {
    case GG::GGK_UP: m_camera->HandleRotation(GG::Pt(GG::X0, GG::Y(SCALE))); break;
    case GG::GGK_DOWN: m_camera->HandleRotation(GG::Pt(GG::X0, GG::Y(-SCALE))); break;
    case GG::GGK_RIGHT: m_camera->HandleRotation(GG::Pt(GG::X(2 * -SCALE), GG::Y0)); break;
    case GG::GGK_LEFT: m_camera->HandleRotation(GG::Pt(GG::X(2 * SCALE), GG::Y0)); break;
    default: break;
    }
}

#define INSTRUMENT_COMBAT_LISTENER_INTERFACE 1

void CombatWnd::ShipPlaced(const CombatShipPtr &ship)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::ShipPlaced()\n";
#endif
    AddCombatShip(ship);
}

void CombatWnd::ShipFired(const CombatShipPtr &ship,
                          const CombatObjectPtr &target,
                          const std::string& part_name)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::ShipFired()\n";
#endif
    // TODO
}

void CombatWnd::ShipDestroyed(const CombatShipPtr &ship)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::ShipDestroyed()\n";
#endif
    RemoveCombatShip(ship);
}

void CombatWnd::ShipEnteredStarlane(const CombatShipPtr &ship)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::ShipEnteredStarlane()\n";
#endif
    // TODO
}

void CombatWnd::FighterLaunched(const CombatFighterPtr &fighter)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::FighterLaunched()\n";
#endif
    // TODO
}

void CombatWnd::FighterFired(const CombatFighterPtr &fighter,
                             const CombatObjectPtr &target)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::FighterFired()\n";
#endif
    // TODO
}

void CombatWnd::FighterDestroyed(const CombatFighterPtr &fighter)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::FighterDestroyed()\n";
#endif
    // TODO
}

void CombatWnd::FighterDocked(const CombatFighterPtr &fighter)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::FighterDocked()\n";
#endif
    // TODO
}

void CombatWnd::MissileLaunched(const MissilePtr &missile)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::MissileLaunched()\n";
#endif
    // TODO
}

void CombatWnd::MissileExploded(const MissilePtr &missile)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::MissileExploded()\n";
#endif
    // TODO
}

void CombatWnd::MissileRemoved(const MissilePtr &missile)
{
#if INSTRUMENT_COMBAT_LISTENER_INTERFACE
    std::cerr << "CombatWnd::MissileRemoved()\n";
#endif
    // TODO
}

#undef INSTRUMENT_COMBAT_LISTENER_INTERFACE

const std::string& CombatWnd::StarBaseName() const
{
    assert(StarTextures().find(m_combat_data->m_system->GetStarType()) != StarTextures().end());
    const std::set<std::string>& star_textures =
        StarTextures().find(m_combat_data->m_system->GetStarType())->second;
    return *boost::next(star_textures.begin(), m_combat_data->m_system->ID() % star_textures.size());
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

    m_camera->Update(event.timeSinceLastFrame);

    if (m_paged_geometry)
        m_paged_geometry->update();

    m_time_since_last_turn_update += event.timeSinceLastFrame;

    if (m_combat_data &&
        m_combat_data->m_combat_turn_number &&
        m_time_since_last_turn_update < PathingEngine::SECONDS_PER_TURN) {
        m_combat_data->m_pathing_engine.Update(event.timeSinceLastFrame, false);
        for (PathingEngine::const_iterator it = m_combat_data->m_pathing_engine.begin();
             it != m_combat_data->m_pathing_engine.end();
             ++it) {
            UpdateObjectPosition(*it);
        }
    }

    return !m_exit;
}

bool CombatWnd::frameEnded(const Ogre::FrameEvent& event)
{ return !m_exit; }

void CombatWnd::UpdateStarFromCameraPosition()
{
    // Determine occlusion of the horizontal midline across the star by objects
    // in the scene.  This is only enabled if glow is in play, since the effect
    // doesn't look right when glow is not used.
    if (GetOptionsDB().Get<bool>("combat.enable-glow")) {
        const Ogre::Vector3 RIGHT = m_camera->GetRealRight();
        const Ogre::Vector3 CAMERA_POS = m_camera->GetRealPosition();
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

    Ogre::Vector3 star_direction = Ogre::Vector3(0.0, 0.0, 0.0) - m_camera->GetRealPosition();
    star_direction.normalise();
    Ogre::Radian angle_at_view_center_to_star =
        Ogre::Math::ACos(m_camera->GetRealDirection().dotProduct(star_direction));
    Ogre::Real BRIGHTNESS_AT_MAX_FOVY = 0.25;
    Ogre::Real center_nearness_factor =
        1.0 - angle_at_view_center_to_star.valueRadians() /
        (m_camera->GetFOVY() / 2.0).valueRadians();
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
    // TODO: Select an appropriate skybox, once we have more than one.
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

    double left = Value(1.0 * std::min(m_selection_drag_start.x, m_selection_drag_stop.x) / APP_WIDTH);
    double right = Value(1.0 * std::max(m_selection_drag_start.x, m_selection_drag_stop.x) / APP_WIDTH);
    double top = Value(1.0 * std::min(m_selection_drag_start.y, m_selection_drag_stop.y) / APP_HEIGHT);
    double bottom = Value(1.0 * std::max(m_selection_drag_start.y, m_selection_drag_stop.y) / APP_HEIGHT);

    const double MIN_SELECTION_VOLUME = 0.0001;
    if ((right - left) * (bottom - top) < MIN_SELECTION_VOLUME)
        return;

    Ogre::Ray ul, ur, ll, lr;
    m_camera->ViewportRay(left, top, ul);
    m_camera->ViewportRay(right, top, ur);
    m_camera->ViewportRay(left, bottom, ll);
    m_camera->ViewportRay(right, bottom, lr);

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
    Ogre::Ray ray;
    m_camera->ViewportRay(Value(pt.x * 1.0 / GG::GUI::GetGUI()->AppWidth()),
                          Value(pt.y * 1.0 / GG::GUI::GetGUI()->AppHeight()),
                          ray);
    RayIntersectionHit hit = RayIntersection(*m_collision_world, ray);
    return hit.m_object;
}

void CombatWnd::DeselectAll()
{ m_current_selections.clear(); }

const Ogre::MaterialPtr& CombatWnd::GetShipMaterial(const Ship& ship)
{
    assert(ship.Design());
    const ShipDesign& ship_design = *ship.Design();

    Ogre::MaterialPtr ship_material =
        Ogre::MaterialManager::getSingleton().getByName("ship");
    std::string modified_material_name = ShipMaterialName(ship_design, *ship.Owners().begin());
    Ogre::MaterialPtr& modified_material = m_ship_materials[modified_material_name];
    if (!modified_material.get()) {
        modified_material = ship_material->clone(modified_material_name);
        modified_material->getTechnique(0)->getPass(1)->getTextureUnitState(0)->
            setTextureName(ship_design.Model() + "_Color.png");
        modified_material->getTechnique(0)->getPass(1)->getTextureUnitState(1)->
            setTextureName(ship_design.Model() + "_Glow.png");
        modified_material->getTechnique(0)->getPass(1)->getTextureUnitState(2)->
            setTextureName(ship_design.Model() + "_Normal.png");
    }

    modified_material->getTechnique(0)->getPass(1)->getFragmentProgramParameters()->
        setNamedConstant("star_light_color", GetSystemColor(StarBaseName()));

    // TODO: Use the current skybox, once we have more than one.
    modified_material->getTechnique(0)->getPass(1)->getFragmentProgramParameters()->
        setNamedConstant("skybox_light_color", GetSystemColor("sky_box_1"));

    assert(ship.Owners().size() == 1u);
    GG::Clr color = Empires().Lookup(*ship.Owners().begin())->Color();
    modified_material->getTechnique(0)->getPass(1)->getFragmentProgramParameters()->
        setNamedConstant("decal_color", Ogre::Vector3(color.r / 255.0, color.g / 255.0, color.b / 255.0));

    return modified_material;
}

void CombatWnd::AddShipNode(int ship_id, Ogre::SceneNode* node, Ogre::Entity* entity,
                            const Ogre::MaterialPtr& material)
{
    CollisionMeshConverter collision_mesh_converter(entity);
    btTriangleMesh* collision_mesh = 0;
    btBvhTriangleMeshShape* collision_shape = 0;
    boost::tie(collision_mesh, collision_shape) = collision_mesh_converter.CollisionShape();

    m_collision_shapes.insert(collision_shape);
    btCollisionObject* collision_object = new btCollisionObject;
    m_collision_objects.insert(collision_object);
    btMatrix3x3 identity;
    identity.setIdentity();
    collision_object->getWorldTransform().setOrigin(ToCollision(node->getPosition()));
    collision_object->getWorldTransform().setRotation(ToCollision(node->getOrientation()));
    collision_object->setCollisionShape(collision_shape);
    m_collision_world->addCollisionObject(collision_object);
    collision_object->setUserPointer(static_cast<Ogre::MovableObject*>(entity));

    m_ship_assets[ship_id] =
        ShipData(node, material, collision_mesh, collision_shape, collision_object);
}

void CombatWnd::RepositionShipNode(int ship_id,
                                   const Ogre::Vector3& position,
                                   const Ogre::Quaternion& orientation)
{
    assert(m_ship_assets.find(ship_id) != m_ship_assets.end());
    ShipData& ship_data = m_ship_assets[ship_id];
    ship_data.m_node->setPosition(position);
    ship_data.m_node->setOrientation(orientation);
    m_collision_world->removeCollisionObject(ship_data.m_bt_object);
    ship_data.m_bt_object->getWorldTransform().setOrigin(ToCollision(position));
    ship_data.m_bt_object->getWorldTransform().setRotation(ToCollision(orientation));
    m_collision_world->addCollisionObject(ship_data.m_bt_object);
}

void CombatWnd::UpdateObjectPosition(const CombatObjectPtr& combat_object)
{
    if (combat_object->IsShip()) {
        assert(boost::dynamic_pointer_cast<CombatShip>(combat_object));
        CombatShipPtr combat_ship = boost::static_pointer_cast<CombatShip>(combat_object);
        Ship& ship = combat_ship->GetShip();
        std::map<int, ShipData>::iterator ship_data_it = m_ship_assets.find(ship.ID());
        assert(ship_data_it != m_ship_assets.end());
                
        ShipData& ship_data = ship_data_it->second;

        SetNodePositionAndOrientation(ship_data.m_node, combat_ship);

        Ogre::Vector3 position = ship_data.m_node->getPosition();
        Ogre::Quaternion orientation = ship_data.m_node->getOrientation();

        ship_data.m_node->setPosition(position);
        ship_data.m_node->setOrientation(orientation);
        m_collision_world->removeCollisionObject(ship_data.m_bt_object);
        ship_data.m_bt_object->getWorldTransform().setOrigin(ToCollision(position));
        ship_data.m_bt_object->getWorldTransform().setRotation(ToCollision(orientation));
        m_collision_world->addCollisionObject(ship_data.m_bt_object);
    } else if (combat_object->IsFighter()) {
        assert(boost::dynamic_pointer_cast<CombatFighter>(combat_object));
        CombatFighterPtr combat_fighter = boost::static_pointer_cast<CombatFighter>(combat_object);
        // TODO
    } else if (MissilePtr missile = boost::dynamic_pointer_cast<Missile>(combat_object)) {
        // TODO
    }
}

void CombatWnd::RemoveShip(int ship_id)
{
    ShipData& ship_data = m_ship_assets[ship_id];
    m_collision_world->removeCollisionObject(ship_data.m_bt_object);
    ship_data.m_node->setVisible(false);
}

void CombatWnd::AddCombatShip(const CombatShipPtr& combat_ship)
{
    const Ship& ship = combat_ship->GetShip();

    const Ogre::MaterialPtr& material = GetShipMaterial(ship);
    Ogre::Entity* entity = CreateShipEntity(m_scene_manager, ship, material);
    Ogre::SceneNode* node = CreateShipSceneNode(m_scene_manager, ship);
    node->attachObject(entity);
    node->setUserAny(Ogre::Any(CombatObjectPtr(combat_ship)));

    SetNodePositionAndOrientation(node, combat_ship);

    AddShipNode(ship.ID(), node, entity, material);
}

void CombatWnd::RemoveCombatShip(const CombatShipPtr& combat_ship)
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
    HumanClientApp::GetApp()->Networking().SendMessage(
        CombatTurnOrdersMessage(
            HumanClientApp::GetApp()->PlayerID(),
            m_combat_order_set));
    m_combat_order_set.clear();
    m_end_turn_button->Disable();
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
    m_camera->Zoom(1, GG::Flags<GG::ModKey>());
    return true;
}

bool CombatWnd::KeyboardZoomOut()
{
    m_camera->Zoom(-1, GG::Flags<GG::ModKey>());
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

////////////////////////////////////////
// Free function(s)
////////////////////////////////////////
bool IsVisible(const Ogre::SceneNode& node)
{
    bool retval = true;
    Ogre::SceneNode::ConstObjectIterator iterator = node.getAttachedObjectIterator();
    while (retval && iterator.hasMoreElements()) {
        retval &= iterator.getNext()->isVisible();
    }
    return retval;
}

Ogre::SceneNode* CreateShipSceneNode(Ogre::SceneManager* scene_manager, const Ship& ship)
{
    return scene_manager->getRootSceneNode()->createChildSceneNode(
        ShipIDString(ship) + ShipMeshName(ship) + "_node");
}

Ogre::Entity* CreateShipEntity(Ogre::SceneManager* scene_manager, const Ship& ship,
                               const Ogre::MaterialPtr& material)
{
    std::string mesh_name = ShipMeshName(ship);
    Ogre::Entity* entity = scene_manager->createEntity(ShipIDString(ship) + mesh_name, mesh_name);
    entity->setCastShadows(true);
    entity->setVisibilityFlags(REGULAR_OBJECTS_MASK);
    entity->setMaterialName(material->getName());
    return entity;
}

Ogre::Vector3 ToOgre(const OpenSteer::Vec3& vec)
{ return Ogre::Vector3(vec.x, vec.y, vec.z); }

OpenSteer::Vec3 ToOpenSteer(const Ogre::Vector3& vec)
{ return OpenSteer::Vec3(vec.x, vec.y, vec.z); }

std::string ShipMaterialName(const ShipDesign& ship_design, int empire_id)
{ return "ship material " + ship_design.Model() + " empire " + boost::lexical_cast<std::string>(empire_id); }

Ogre::Quaternion StarwardOrientationForPosition(const Ogre::Vector3& position)
{
    return Ogre::Quaternion(Ogre::Radian(std::atan2(-position.y, -position.x) -
                                         Ogre::Math::HALF_PI),
                            Ogre::Vector3(0.0, 0.0, 1.0));
}
