// -*- C++ -*-
#ifndef _CombatCamera_h_
#define _CombatCamera_h_

#include <OgreVector3.h>

#include <GG/WndEvent.h>

#include <boost/signal.hpp>


namespace Ogre {
    class Animation;
    class AnimationState;
    class Camera;
}

class CombatCamera
{
public:
    CombatCamera(Ogre::Camera& camera, Ogre::SceneManager* scene_manager, Ogre::SceneNode* look_at_node);

    bool Moving() const;
    Ogre::Vector3 GetRealPosition() const;
    Ogre::Vector3 GetRealDirection() const;
    Ogre::Vector3 GetRealRight() const;
    Ogre::Radian GetFOVY() const;
    std::pair<bool, Ogre::Vector3> IntersectMouseWithEcliptic(const GG::Pt& pt) const;
    Ogre::Vector3 Project(const Ogre::Vector3& world_pt) const;
    GG::Pt ProjectToPixel(const Ogre::Vector3& world_pt) const;
    void ViewportRay(double screen_x, double screen_y, Ogre::Ray& ray) const;

    void Update(Ogre::Real time_since_last_frame);
    void LookAtNode(Ogre::SceneNode* look_at_node);
    void LookAtPosition(const Ogre::Vector3& look_at_point);
    void LookAtPositionAndZoom(const Ogre::Vector3& look_at_point, Ogre::Real distance);
    void Zoom(int move, GG::Flags<GG::ModKey> mod_keys);
    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    void HandleRotation(const GG::Pt& delta);

    mutable boost::signal<void ()> CameraChangedSignal;

private:
    void ViewportRay(double screen_x, double screen_y,
                     double out_ray_origin[3], double out_ray_direction[3]) const;
    std::pair<Ogre::Vector3, Ogre::Quaternion> CameraPositionAndOrientation(Ogre::Real distance) const;
    Ogre::Real ZoomResult(Ogre::Real total_move) const;
    Ogre::Real DistanceToLookAtPoint() const;

    void LookAtPositionImpl(const Ogre::Vector3& look_at_point, Ogre::Real zoom);
    void ZoomImpl(Ogre::Real total_move);
    void UpdateCameraPosition();

    Ogre::Camera& m_camera;

    Ogre::SceneNode* m_camera_node;
    Ogre::Animation* m_camera_animation;
    Ogre::AnimationState* m_camera_animation_state;

    Ogre::Radian m_pitch;
    Ogre::Radian m_roll;
    Ogre::SceneNode* m_look_at_scene_node;
    Ogre::Vector3 m_look_at_point;
    Ogre::Vector3 m_initial_zoom_in_position;
    unsigned int m_previous_zoom_in_time;
};

#endif
