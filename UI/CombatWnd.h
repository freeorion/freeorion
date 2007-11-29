// -*- C++ -*-
#ifndef _CombatWnd_h_
#define _CombatWnd_h_

#include <OgreFrameListener.h>
#include <OgreMath.h>

#include <GG/Wnd.h>


namespace Ogre {
    class Camera;
    class SceneManager;
    class SceneNode;
    class Viewport;
}

class CombatWnd :
    public GG::Wnd,
    public Ogre::FrameListener
{
public:
    CombatWnd (Ogre::SceneManager* scene_manager,
               Ogre::Camera* camera,
               Ogre::Viewport* viewport);
    virtual ~CombatWnd();

    virtual void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void KeyPress(GG::Key key, GG::Flags<GG::ModKey> mod_keys);

private:
    virtual bool frameStarted(const Ogre::FrameEvent &event);

    Ogre::SceneManager* m_scene_manager;
    Ogre::Camera* m_camera;
    Ogre::Viewport* m_viewport;
    Ogre::SceneNode* m_durgha_node;

    Ogre::Real m_distance_to_lookat_point;
    Ogre::Radian m_pitch;
    Ogre::Radian m_yaw;
    GG::Pt m_last_pos;

    bool m_exit; // TODO: Remove this; it is only here for prototyping.
};

#endif
