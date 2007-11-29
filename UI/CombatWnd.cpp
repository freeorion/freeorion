#include "CombatWnd.h"

#include "ClientUI.h"

#include <OgreCamera.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreMeshManager.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>

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
    m_distance_to_lookat_point(m_camera->getPosition().length()),
    m_pitch(0.0),
    m_yaw(0.0),
    m_last_pos(),
    m_exit(false)
{
    Ogre::Root::getSingleton().addFrameListener(this);

    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load((ClientUI::ArtDir() / "combat" / "resources.cfg").native_file_string());

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator section_it = cf.getSectionIterator();
    while (section_it.hasMoreElements())
    {
        Ogre::String section_name = section_it.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = section_it.getNext();
        for (Ogre::ConfigFile::SettingsMultiMap::iterator it = settings->begin();
             it != settings->end();
             ++it)
        {
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

    // Put an "Durgha" ship model in the middle
    Ogre::MeshPtr m = Ogre::MeshManager::getSingleton().load(
        "durgha.mesh",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::Entity* e = m_scene_manager->createEntity("durgha", "durgha.mesh");
    e->setCastShadows(true);
    m_durgha_node = m_scene_manager->getRootSceneNode()->createChildSceneNode("durgha node");
    m_durgha_node->setDirection(0, -1, 0);
    m_durgha_node->yaw(Ogre::Radian(Ogre::Math::PI));
    m_durgha_node->attachObject(e);

    m_scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
    m_scene_manager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
    Ogre::Light* star = m_scene_manager->createLight("Star");
    star->setType(Ogre::Light::LT_DIRECTIONAL);
    star->setDirection(Ogre::Vector3(1, -0.15, 0.25).normalisedCopy());
}

CombatWnd::~CombatWnd()
{ Ogre::Root::getSingleton().removeFrameListener(this); }

void CombatWnd::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ m_last_pos = GG::Pt(); }

void CombatWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_last_pos == GG::Pt())
        m_last_pos = pt - move;

    GG::Pt delta_pos = pt - m_last_pos;
    m_last_pos = pt;

    // set position to look-at point
    m_camera->setPosition(Ogre::Vector3()); // TODO (leave it at the origin for now)

    // set orientation
    m_camera->setDirection(-Ogre::Vector3::UNIT_Z);
    Ogre::Radian delta_pitch = -delta_pos.y * 1.0 / GG::GUI::GetGUI()->AppHeight() * Ogre::Radian(Ogre::Math::PI);
    m_pitch += delta_pitch;
    if (m_pitch < Ogre::Radian(-Ogre::Math::HALF_PI))
        m_pitch = Ogre::Radian(-Ogre::Math::HALF_PI);
    if (Ogre::Radian(Ogre::Math::HALF_PI) < m_pitch)
        m_pitch = Ogre::Radian(Ogre::Math::HALF_PI);
    m_camera->pitch(m_pitch);
    Ogre::Radian delta_yaw = -delta_pos.x * 1.0 / GG::GUI::GetGUI()->AppWidth() * Ogre::Radian(Ogre::Math::PI);
    m_yaw += delta_yaw;
    m_camera->yaw(m_yaw);

    // back up to final position
    m_camera->moveRelative(Ogre::Vector3(0, 0, m_distance_to_lookat_point));
}

void CombatWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{}

void CombatWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
#if 0 // this was a test to see how easy it is to clean up the Ogre scene graph in order to go back to GG-only mode
    if (m_durgha_node->isInSceneGraph()) {
        m_scene_manager->getRootSceneNode()->removeChild(m_durgha_node);
        m_scene_manager->setSkyBox(false, "backgrounds/sky_box_1");
    } else {
        m_scene_manager->getRootSceneNode()->addChild(m_durgha_node);
        m_scene_manager->setSkyBox(true, "backgrounds/sky_box_1", 50);
    }
#endif
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
{ return !m_exit; }
