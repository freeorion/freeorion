#include "CombatWnd.h"

#include "ClientUI.h"

#include <OgreCamera.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreMeshManager.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneQuery.h>

#include <GG/GUI.h>


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
    m_distance_to_lookat_point(m_camera->getPosition().length()),
    m_pitch(0.0),
    m_yaw(0.0),
    m_last_pos(),
    m_mouse_dragged(false),
    m_currently_selected_scene_node(0),
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


    //////////////////////////////////////////////////////////////////
    // NOTE: Below is temporary code for combat system prototyping! //
    //////////////////////////////////////////////////////////////////

    m_scene_manager->setSkyBox(true, "backgrounds/sky_box_1", 50);

    // Load the "Durgha" ship mesh
    Ogre::MeshPtr m = Ogre::MeshManager::getSingleton().load(
        "durgha.mesh",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    // Put a few Durghas into the scene
    Ogre::Entity* entity = m_scene_manager->createEntity("lead durgha", "durgha.mesh");
    entity->setCastShadows(true);
    Ogre::SceneNode* lead_durgha_node = m_scene_manager->getRootSceneNode()->createChildSceneNode("lead durgha node");
    lead_durgha_node->setDirection(0, -1, 0);
    lead_durgha_node->yaw(Ogre::Radian(Ogre::Math::PI));
    lead_durgha_node->attachObject(entity);

    entity = m_scene_manager->createEntity("wing durgha 1", "durgha.mesh");
    entity->setCastShadows(true);
    Ogre::SceneNode* durgha_node = lead_durgha_node->createChildSceneNode("wing durgha 1 node");
    durgha_node->attachObject(entity);
    durgha_node->setPosition(250, 250, 0);

    m_scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
    m_scene_manager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
    Ogre::Light* star = m_scene_manager->createLight("Star");
    star->setType(Ogre::Light::LT_DIRECTIONAL);
    star->setDirection(Ogre::Vector3(1, -0.15, 0.25).normalisedCopy());
}

CombatWnd::~CombatWnd()
{
    Ogre::Root::getSingleton().removeFrameListener(this);
    m_scene_manager->destroyQuery(m_ray_scene_query);
}

void CombatWnd::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_last_pos = pt;
    m_mouse_dragged = false;
}

void CombatWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Pt delta_pos = pt - m_last_pos;
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

void CombatWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{}

void CombatWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!m_mouse_dragged) {
        Ogre::Ray ray = m_camera->getCameraToViewportRay(pt.x * 1.0 / GG::GUI::GetGUI()->AppWidth(),
                                                         pt.y * 1.0 / GG::GUI::GetGUI()->AppHeight());
        m_ray_scene_query->setRay(ray);
        Ogre::RaySceneQueryResult &result = m_ray_scene_query->execute();
        if (result.begin() != result.end() && result.begin()->movable) {
            Ogre::SceneNode* clicked_scene_node = result.begin()->movable->getParentSceneNode();
            assert(clicked_scene_node);
            m_currently_selected_scene_node = clicked_scene_node;
            m_lookat_point = m_currently_selected_scene_node->getWorldPosition();
            UpdateCameraPosition();
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

bool CombatWnd::frameStarted(const Ogre::FrameEvent &event)
{
    if (m_currently_selected_scene_node) {
        m_lookat_point = m_currently_selected_scene_node->getWorldPosition();
        UpdateCameraPosition();
    }
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
