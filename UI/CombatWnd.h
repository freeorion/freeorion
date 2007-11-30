// -*- C++ -*-
#ifndef _CombatWnd_h_
#define _CombatWnd_h_

#include <OgreFrameListener.h>
#include <OgreManualObject.h>
#include <OgreMath.h>
#include <OgreVector3.h>

#include <GG/Wnd.h>


namespace Ogre {
    class Camera;
    class RaySceneQuery;
    class SceneManager;
    class SceneNode;
    class Viewport;
    class PlaneBoundedVolumeListSceneQuery;
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
    class SelectionRect : public Ogre::ManualObject
    {
    public:
        SelectionRect();
        void Resize(const GG::Pt& pt1, const GG::Pt& pt2);
    };

    virtual bool frameStarted(const Ogre::FrameEvent& event);

    void UpdateCameraPosition();
    void EndShiftDrag();
    void SelectObjectsInVolume();

    Ogre::SceneManager* m_scene_manager;
    Ogre::Camera* m_camera;
    Ogre::Viewport* m_viewport;
    Ogre::RaySceneQuery* m_ray_scene_query;
    Ogre::PlaneBoundedVolumeListSceneQuery* m_volume_scene_query;

    Ogre::Real m_distance_to_lookat_point;
    Ogre::Radian m_pitch;
    Ogre::Radian m_yaw;
    GG::Pt m_last_pos;
    GG::Pt m_shift_drag_start;
    GG::Pt m_shift_drag_stop;
    bool m_mouse_dragged;
    Ogre::SceneNode* m_currently_selected_scene_node;
    SelectionRect* m_selection_rect;
    Ogre::Vector3 m_lookat_point;

    bool m_exit; // TODO: Remove this; it is only here for prototyping.
    std::vector<Ogre::MovableObject*> m_current_selections;
};

#endif
