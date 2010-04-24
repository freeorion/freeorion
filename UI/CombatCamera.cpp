#include "CombatCamera.h"

#include "CombatWndFwd.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../util/Math.h"

#include <OgreAnimation.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>

#include <GG/GUI.h>

#include <cfloat>


namespace {
    // The time it takes to recenter on a new point, in seconds.
    const Ogre::Real CAMERA_MOVE_TIME = 0.33333;
    const int CAMERA_ANIMATION_STEPS = 8;
    const Ogre::Real TIME_INCREMENT = CAMERA_MOVE_TIME / CAMERA_ANIMATION_STEPS;

    const unsigned short CAMERA_NODE_TRACK_HANDLE = 0;
    const unsigned short CAMERA_TRACK_HANDLE = 2;

    const Ogre::Real IGNORE_DISTANCE = FLT_MAX;

    const Ogre::Real NEAR_CLIP = 0.01;
    const Ogre::Real FAR_CLIP = 4.0 * SystemRadius();

    const Ogre::Real MAX_ZOOM_OUT_DISTANCE = 2.0 * SystemRadius();
    const Ogre::Real MIN_ZOOM_IN_DISTANCE = PlanetRadius(SZ_GASGIANT) * 1.05;

    const Ogre::Vector3 INVALID_MAP_LOCATION(FLT_MAX, FLT_MAX, FLT_MAX);

    Ogre::Real ZoomFactor(GG::Flags<GG::ModKey> mod_keys)
    {
        Ogre::Real retval = 1.0;
        if (mod_keys & GG::MOD_KEY_SHIFT)
            retval *= 2.0;
        if (mod_keys & GG::MOD_KEY_CTRL)
            retval /= 4.0;
        return retval;
    }

    Ogre::Real TotalMove(int move, GG::Flags<GG::ModKey> mod_keys, Ogre::Real current_distance)
    { return current_distance * 0.25 * ZoomFactor(mod_keys) * -move; }

    class AnimableCamera :
        public Ogre::AnimableValue
    {
    public:
        AnimableCamera(Ogre::Camera& camera, const Ogre::Vector3& v) :
            AnimableValue(VECTOR3),
            m_camera(camera)
            { std::memcpy(mBaseValueReal, v.ptr(), sizeof(Ogre::Real) * 3); }

        virtual void setValue(const Ogre::Vector3& v)
            { setAsBaseValue(v); }

        virtual void setCurrentStateAsBaseValue()
            {}

        virtual void applyDeltaValue(const Ogre::Vector3& v)
            {
                setAsBaseValue(v);
                m_camera.setPosition(v);
            }

    private:
        Ogre::Camera& m_camera;
    };
}

CombatCamera::CombatCamera(Ogre::Camera& camera,
                           Ogre::SceneManager* scene_manager,
                           Ogre::SceneNode* look_at_node) :
    m_camera(camera),
    m_camera_node(scene_manager->getRootSceneNode()->createChildSceneNode()),
    m_camera_animation(scene_manager->createAnimation("CameraTrack", CAMERA_MOVE_TIME)),
    m_camera_animation_state(scene_manager->createAnimationState("CameraTrack")),
    m_pitch(0.0),
    m_roll(0.0),
    m_look_at_scene_node(look_at_node),
    m_initial_zoom_in_position(INVALID_MAP_LOCATION),
    m_previous_zoom_in_time(0),
    m_look_at_point_target(0, 0, 0),
    m_distance_to_look_at_point_target(0)
{
    m_camera.setNearClipDistance(NEAR_CLIP);
    m_camera.setFarClipDistance(FAR_CLIP);
    m_camera.setQueryFlags(UNSELECTABLE_OBJECT_MASK);
    m_camera_node->attachObject(&m_camera);

    m_camera_animation->setInterpolationMode(Ogre::Animation::IM_SPLINE);
    m_camera_animation_state->setEnabled(true);
    m_camera_animation_state->setLoop(false);
}

bool CombatCamera::Moving() const
{ return m_camera_animation->hasNodeTrack(CAMERA_NODE_TRACK_HANDLE); }

Ogre::Vector3 CombatCamera::GetRealPosition() const
{ return m_camera.getRealPosition(); }

Ogre::Vector3 CombatCamera::GetRealDirection() const
{ return m_camera.getRealDirection(); }

Ogre::Vector3 CombatCamera::GetRealRight() const
{ return m_camera.getRealRight(); }

Ogre::Radian CombatCamera::GetFOVY() const
{ return m_camera.getFOVy(); }

std::pair<bool, Ogre::Vector3> CombatCamera::IntersectMouseWithEcliptic(const GG::Pt& pt) const
{
    std::pair<bool, Ogre::Vector3> retval(false, Ogre::Vector3());
    double ray_origin[3];
    double ray_direction[3];
    ViewportRay(Value(pt.x * 1.0 / GG::GUI::GetGUI()->AppWidth()),
                Value(pt.y * 1.0 / GG::GUI::GetGUI()->AppHeight()),
                ray_origin, ray_direction);
    double unit_z[3] = { 0, 0, 1.0 };
    double origin[3] = { 0, 0, 0 };
    std::pair<bool, double> intersection = Intersects(ray_origin, ray_direction, unit_z, origin);
    if (intersection.first) {
        double point[3] = {
            ray_origin[0] + ray_direction[0] * intersection.second,
            ray_origin[1] + ray_direction[1] * intersection.second,
            ray_origin[2] + ray_direction[2] * intersection.second
        };
        const double MAX_DISTANCE_SQ = SystemRadius() * SystemRadius();
        if ((point[0] * point[0] + point[1] * point[1] + point[2] * point[2]) < MAX_DISTANCE_SQ) {
            retval.first = true;
            retval.second.x = point[0];
            retval.second.y = point[1];
            retval.second.z = point[2];
        }
    }
    return retval;
}

Ogre::Vector3 CombatCamera::Project(const Ogre::Vector3& world_pt) const
{
    Ogre::Vector3 retval(-5.0, -5.0, 1.0);
    Ogre::Vector3 eye_space_pt = m_camera.getViewMatrix() * world_pt;
    if (eye_space_pt.z < 0.0)
        retval = m_camera.getProjectionMatrixWithRSDepth() * eye_space_pt;
    return retval;
}

GG::Pt CombatCamera::ProjectToPixel(const Ogre::Vector3& world_pt) const
{
    Ogre::Vector3 projection = (Project(world_pt) + Ogre::Vector3(1.0, 1.0, 1.0)) / 2.0;
    return GG::Pt(GG::X(static_cast<int>(projection.x * m_camera.getViewport()->getActualWidth())),
                  GG::Y(static_cast<int>((1.0 - projection.y) * m_camera.getViewport()->getActualHeight())));
}

void CombatCamera::ViewportRay(double screen_x, double screen_y, Ogre::Ray& ray) const
{
    double out_origin[3];
    double ray_direction[3];
    ViewportRay(screen_x,  screen_y, out_origin, ray_direction);
    ray.setOrigin(Ogre::Vector3(out_origin[0], out_origin[1], out_origin[2]));
    ray.setDirection(Ogre::Vector3(ray_direction[0], ray_direction[1], ray_direction[2]));
}

void CombatCamera::Update(Ogre::Real time_since_last_frame)
{
    m_camera_animation_state->addTime(time_since_last_frame);
    if (m_camera_animation_state->hasEnded()) {
        // TODO: verify that the move actually happened, by placing the camera
        // at its final destination.  This is necessary, because sometimes
        // low-enough frame rates mean that the move isn't completed, or
        // doesn't happen at all.
        m_camera_animation->destroyAllTracks();
    }
}

void CombatCamera::LookAtNode(Ogre::SceneNode* look_at_node)
{
    m_look_at_scene_node = look_at_node;
    LookAtPositionImpl(m_look_at_scene_node->_getDerivedPosition(), IGNORE_DISTANCE);
}

void CombatCamera::LookAtPosition(const Ogre::Vector3& look_at_point)
{
    m_look_at_scene_node = 0;
    LookAtPositionImpl(look_at_point, IGNORE_DISTANCE);
}

void CombatCamera::LookAtPositionAndZoom(const Ogre::Vector3& look_at_point, Ogre::Real distance)
{
    m_look_at_scene_node = 0;
    LookAtPositionImpl(look_at_point, distance);
}

void CombatCamera::Zoom(int move, GG::Flags<GG::ModKey> mod_keys)
{ ZoomImpl(TotalMove(move, mod_keys, DistanceToLookAtPoint())); }

void CombatCamera::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    Ogre::Real total_move = TotalMove(move, mod_keys, DistanceToLookAtPoint());
    if (0 < move)
    {
        const unsigned int TICKS = GG::GUI::GetGUI()->Ticks();

        const unsigned int ZOOM_IN_TIMEOUT = 750u;
        if (m_initial_zoom_in_position == INVALID_MAP_LOCATION ||
            ZOOM_IN_TIMEOUT < TICKS - m_previous_zoom_in_time)
        {
            std::pair<bool, Ogre::Vector3> intersection = IntersectMouseWithEcliptic(pt);
            m_initial_zoom_in_position = intersection.first ? intersection.second : INVALID_MAP_LOCATION;
        }

        if (m_initial_zoom_in_position != INVALID_MAP_LOCATION) {
            const double CLOSE_FACTOR = move * 0.333;
            Ogre::Vector3 start = Moving() ? m_look_at_point_target : LookAtPoint();
            Ogre::Vector3 delta = m_initial_zoom_in_position - start;
            double delta_length = delta.length();
            double distance = std::min(std::max(1.0, delta_length * CLOSE_FACTOR), delta_length);
            delta.normalise();
            Ogre::Vector3 new_center = start + delta * distance;
            if (new_center.length() < SystemRadius())
                LookAtPositionAndZoom(new_center, ZoomResult(total_move));
        }

        m_previous_zoom_in_time = TICKS;
    } else if (move < 0) {
        ZoomImpl(total_move);
    }
}

void CombatCamera::HandleRotation(const GG::Pt& delta)
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

void CombatCamera::ViewportRay(double screen_x, double screen_y,
                               double out_ray_origin[3], double out_ray_direction[3]) const
{
    Matrix projection(4, 4);
    std::copy(m_camera.getProjectionMatrix()[0], m_camera.getProjectionMatrix()[0] + 16,
              projection.data().begin());

    Matrix view(4, 4);
    std::copy(m_camera.getViewMatrix(true)[0], m_camera.getViewMatrix(true)[0] + 16, view.data().begin());

    Matrix inverse_vp = Inverse4(prod(projection, view));

    double nx = (2.0 * screen_x) - 1.0;
    double ny = 1.0 - (2.0 * screen_y);
    Matrix near_point(3, 1);
    near_point(0, 0) = nx;
    near_point(1, 0) = ny;
    near_point(2, 0) = -1.0;
    // Use mid_point rather than far point to avoid issues with infinite projection
    Matrix mid_point(3, 1);
    mid_point(0, 0) = nx;
    mid_point(1, 0) = ny;
    mid_point(2, 0) = 0.0;

    // Get ray origin and ray target on near plane in world space
    Matrix ray_origin = Matrix4xVector3(inverse_vp, near_point);
    Matrix ray_target = Matrix4xVector3(inverse_vp, mid_point);

    Matrix ray_direction = ray_target - ray_origin;
    ray_direction /=
        ray_direction(0, 0) * ray_direction(0, 0) +
        ray_direction(1, 0) * ray_direction(1, 0) +
        ray_direction(2, 0) * ray_direction(2, 0);

    std::copy(ray_origin.data().begin(), ray_origin.data().end(), out_ray_origin);
    std::copy(ray_direction.data().begin(), ray_direction.data().end(), out_ray_direction);
}

std::pair<Ogre::Vector3, Ogre::Quaternion>
CombatCamera::CameraPositionAndOrientation(Ogre::Real distance) const
{
    // Here, we calculate where m_camera should be relative to its parent
    // m_camera_node.  m_camera_node always stays on the ecliptic, and the
    // camera moves away from it a bit to look at the position it occupies.
    // This code was originally written using the high-level Ogre::Camera API,
    // but now the camera position sometimes needs to be known without
    // actually moving the camera.  The original lines of code are preserved
    // here as comments, and under each one is the equivalent code cut from
    // OgreCamera.cpp.

    std::pair<Ogre::Vector3, Ogre::Quaternion> retval;

    // Ogre::Camera::setPosition(Ogre::Vector3::ZERO);
    retval.first = Ogre::Vector3::ZERO;

    // Ogre::Camera::setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z);
    {
        Ogre::Vector3 zAdjustVec = -Ogre::Vector3::NEGATIVE_UNIT_Z;
        Ogre::Vector3 xVec = Ogre::Vector3::UNIT_Y.crossProduct( zAdjustVec );
        xVec.normalise();
        Ogre::Vector3 yVec = zAdjustVec.crossProduct( xVec );
        yVec.normalise();
        retval.second.FromAxes( xVec, yVec, zAdjustVec );
    }

    // Ogre::Camera::roll(m_roll);
    {
        Ogre::Vector3 zAxis = retval.second * Ogre::Vector3::UNIT_Z;
        Ogre::Quaternion roll_q(m_roll, zAxis);
        roll_q.normalise();
        retval.second = roll_q * retval.second;
    }

    // Ogre::Camera::pitch(m_pitch);
    {
        Ogre::Vector3 xAxis = retval.second * Ogre::Vector3::UNIT_X;
        Ogre::Quaternion pitch_q(m_pitch, xAxis);
        pitch_q.normalise();
        retval.second = pitch_q * retval.second;
    }

    // Ogre::Camera::moveRelative(Ogre::Vector3(0, 0, distance));
    {
        Ogre::Vector3 trans = retval.second * Ogre::Vector3(0, 0, distance);
        retval.first += trans;
    }

    return retval;
}

Ogre::Real CombatCamera::ZoomResult(Ogre::Real total_move) const
{
    Ogre::Sphere bounding_sphere(Ogre::Vector3(), 0.0);
    if (m_look_at_scene_node)
        bounding_sphere = m_look_at_scene_node->getAttachedObject(0)->getWorldBoundingSphere();
    const Ogre::Real EFFECTIVE_MIN_DISTANCE =
        std::max(bounding_sphere.getRadius() * Ogre::Real(1.05), MIN_ZOOM_IN_DISTANCE);

    Ogre::Real distance_to_look_at_point = DistanceToLookAtPoint();
    if (distance_to_look_at_point + total_move < EFFECTIVE_MIN_DISTANCE)
        total_move += EFFECTIVE_MIN_DISTANCE - (distance_to_look_at_point + total_move);
    else if (MAX_ZOOM_OUT_DISTANCE < distance_to_look_at_point + total_move)
        total_move -= (distance_to_look_at_point + total_move) - MAX_ZOOM_OUT_DISTANCE;
    return distance_to_look_at_point + total_move;
}

Ogre::Vector3 CombatCamera::LookAtPoint() const
{ return m_camera_node->getPosition(); }

Ogre::Real CombatCamera::DistanceToLookAtPoint() const
{ return m_camera.getPosition().length(); }

void CombatCamera::LookAtPositionImpl(const Ogre::Vector3& look_at_point, Ogre::Real distance)
{
    if (distance == IGNORE_DISTANCE)
        distance = DistanceToLookAtPoint();

    // We interpolate two things in our animation: the position of
    // m_camera_node, which always stays on the ecliptic, and the
    // parent-relative position of m_camera.  The latter is interpolated in
    // ZoomImpl().

    ZoomImpl(distance - DistanceToLookAtPoint());

    Ogre::Vector3 node_start = LookAtPoint();
    Ogre::Vector3 node_stop = look_at_point;

    const Ogre::Vector3 NODE_POS_DELTA = node_stop - node_start;

    Ogre::NodeAnimationTrack* node_track =
        m_camera_animation->createNodeTrack(CAMERA_NODE_TRACK_HANDLE, m_camera_node);

    const Ogre::Vector3 NODE_POS_INCREMENT = NODE_POS_DELTA / CAMERA_ANIMATION_STEPS;

    // the loop extends an extra 2 steps in either direction, to
    // ensure smoothness (since splines are being used)
    for (int i = -2; i < CAMERA_ANIMATION_STEPS + 2; ++i) {
        Ogre::TransformKeyFrame* node_key = node_track->createNodeKeyFrame(i * TIME_INCREMENT);
        node_key->setTranslate(node_stop - NODE_POS_DELTA + i * NODE_POS_INCREMENT);
    }

    m_look_at_point_target = look_at_point;
}

void CombatCamera::ZoomImpl(Ogre::Real total_move)
{
    m_camera_animation->destroyAllTracks();

    if (Moving())
        total_move += m_distance_to_look_at_point_target - DistanceToLookAtPoint();

    Ogre::Real distance = ZoomResult(total_move);

    Ogre::Vector3 camera_start = CameraPositionAndOrientation(DistanceToLookAtPoint()).first;
    Ogre::Vector3 camera_stop = CameraPositionAndOrientation(distance).first;

    const Ogre::Vector3 CAMERA_DELTA = camera_stop - camera_start;

    m_camera_animation_state->setTimePosition(0.0);
    Ogre::AnimableValuePtr animable_camera_pos(new AnimableCamera(m_camera, camera_start));
    Ogre::NumericAnimationTrack* camera_track =
        m_camera_animation->createNumericTrack(CAMERA_TRACK_HANDLE, animable_camera_pos);

    const Ogre::Vector3 CAMERA_INCREMENT = CAMERA_DELTA / CAMERA_ANIMATION_STEPS;

    // the loop extends an extra 2 steps in either direction, to
    // ensure smoothness (since splines are being used)
    for (int i = -2; i < CAMERA_ANIMATION_STEPS + 2; ++i) {
        Ogre::NumericKeyFrame* camera_key = camera_track->createNumericKeyFrame(i * TIME_INCREMENT);
        camera_key->setValue(camera_stop - CAMERA_DELTA + i * CAMERA_INCREMENT);
    }

    m_distance_to_look_at_point_target = distance;
}

void CombatCamera::UpdateCameraPosition()
{
    std::pair<Ogre::Vector3, Ogre::Quaternion> position_and_orientation =
        CameraPositionAndOrientation(DistanceToLookAtPoint());
    m_camera.setPosition(position_and_orientation.first);
    m_camera.setOrientation(position_and_orientation.second);
    CameraChangedSignal();
}
