#ifndef GODOT_CPP_PHYSICSSERVER_HPP
#define GODOT_CPP_PHYSICSSERVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "PhysicsServer.hpp"

#include "Object.hpp"
namespace godot {

class Object;
class PhysicsDirectBodyState;
class PhysicsDirectSpaceState;

class PhysicsServer : public Object {
	static PhysicsServer *_singleton;

	PhysicsServer();

	struct ___method_bindings {
		godot_method_bind *mb_area_add_shape;
		godot_method_bind *mb_area_attach_object_instance_id;
		godot_method_bind *mb_area_clear_shapes;
		godot_method_bind *mb_area_create;
		godot_method_bind *mb_area_get_object_instance_id;
		godot_method_bind *mb_area_get_param;
		godot_method_bind *mb_area_get_shape;
		godot_method_bind *mb_area_get_shape_count;
		godot_method_bind *mb_area_get_shape_transform;
		godot_method_bind *mb_area_get_space;
		godot_method_bind *mb_area_get_space_override_mode;
		godot_method_bind *mb_area_get_transform;
		godot_method_bind *mb_area_is_ray_pickable;
		godot_method_bind *mb_area_remove_shape;
		godot_method_bind *mb_area_set_area_monitor_callback;
		godot_method_bind *mb_area_set_collision_layer;
		godot_method_bind *mb_area_set_collision_mask;
		godot_method_bind *mb_area_set_monitor_callback;
		godot_method_bind *mb_area_set_monitorable;
		godot_method_bind *mb_area_set_param;
		godot_method_bind *mb_area_set_ray_pickable;
		godot_method_bind *mb_area_set_shape;
		godot_method_bind *mb_area_set_shape_disabled;
		godot_method_bind *mb_area_set_shape_transform;
		godot_method_bind *mb_area_set_space;
		godot_method_bind *mb_area_set_space_override_mode;
		godot_method_bind *mb_area_set_transform;
		godot_method_bind *mb_body_add_central_force;
		godot_method_bind *mb_body_add_collision_exception;
		godot_method_bind *mb_body_add_force;
		godot_method_bind *mb_body_add_shape;
		godot_method_bind *mb_body_add_torque;
		godot_method_bind *mb_body_apply_central_impulse;
		godot_method_bind *mb_body_apply_impulse;
		godot_method_bind *mb_body_apply_torque_impulse;
		godot_method_bind *mb_body_attach_object_instance_id;
		godot_method_bind *mb_body_clear_shapes;
		godot_method_bind *mb_body_create;
		godot_method_bind *mb_body_get_collision_layer;
		godot_method_bind *mb_body_get_collision_mask;
		godot_method_bind *mb_body_get_direct_state;
		godot_method_bind *mb_body_get_kinematic_safe_margin;
		godot_method_bind *mb_body_get_max_contacts_reported;
		godot_method_bind *mb_body_get_mode;
		godot_method_bind *mb_body_get_object_instance_id;
		godot_method_bind *mb_body_get_param;
		godot_method_bind *mb_body_get_shape;
		godot_method_bind *mb_body_get_shape_count;
		godot_method_bind *mb_body_get_shape_transform;
		godot_method_bind *mb_body_get_space;
		godot_method_bind *mb_body_get_state;
		godot_method_bind *mb_body_is_axis_locked;
		godot_method_bind *mb_body_is_continuous_collision_detection_enabled;
		godot_method_bind *mb_body_is_omitting_force_integration;
		godot_method_bind *mb_body_is_ray_pickable;
		godot_method_bind *mb_body_remove_collision_exception;
		godot_method_bind *mb_body_remove_shape;
		godot_method_bind *mb_body_set_axis_lock;
		godot_method_bind *mb_body_set_axis_velocity;
		godot_method_bind *mb_body_set_collision_layer;
		godot_method_bind *mb_body_set_collision_mask;
		godot_method_bind *mb_body_set_enable_continuous_collision_detection;
		godot_method_bind *mb_body_set_force_integration_callback;
		godot_method_bind *mb_body_set_kinematic_safe_margin;
		godot_method_bind *mb_body_set_max_contacts_reported;
		godot_method_bind *mb_body_set_mode;
		godot_method_bind *mb_body_set_omit_force_integration;
		godot_method_bind *mb_body_set_param;
		godot_method_bind *mb_body_set_ray_pickable;
		godot_method_bind *mb_body_set_shape;
		godot_method_bind *mb_body_set_shape_disabled;
		godot_method_bind *mb_body_set_shape_transform;
		godot_method_bind *mb_body_set_space;
		godot_method_bind *mb_body_set_state;
		godot_method_bind *mb_cone_twist_joint_get_param;
		godot_method_bind *mb_cone_twist_joint_set_param;
		godot_method_bind *mb_free_rid;
		godot_method_bind *mb_generic_6dof_joint_get_flag;
		godot_method_bind *mb_generic_6dof_joint_get_param;
		godot_method_bind *mb_generic_6dof_joint_set_flag;
		godot_method_bind *mb_generic_6dof_joint_set_param;
		godot_method_bind *mb_get_process_info;
		godot_method_bind *mb_hinge_joint_get_flag;
		godot_method_bind *mb_hinge_joint_get_param;
		godot_method_bind *mb_hinge_joint_set_flag;
		godot_method_bind *mb_hinge_joint_set_param;
		godot_method_bind *mb_joint_create_cone_twist;
		godot_method_bind *mb_joint_create_generic_6dof;
		godot_method_bind *mb_joint_create_hinge;
		godot_method_bind *mb_joint_create_pin;
		godot_method_bind *mb_joint_create_slider;
		godot_method_bind *mb_joint_get_solver_priority;
		godot_method_bind *mb_joint_get_type;
		godot_method_bind *mb_joint_set_solver_priority;
		godot_method_bind *mb_pin_joint_get_local_a;
		godot_method_bind *mb_pin_joint_get_local_b;
		godot_method_bind *mb_pin_joint_get_param;
		godot_method_bind *mb_pin_joint_set_local_a;
		godot_method_bind *mb_pin_joint_set_local_b;
		godot_method_bind *mb_pin_joint_set_param;
		godot_method_bind *mb_set_active;
		godot_method_bind *mb_shape_create;
		godot_method_bind *mb_shape_get_data;
		godot_method_bind *mb_shape_get_type;
		godot_method_bind *mb_shape_set_data;
		godot_method_bind *mb_slider_joint_get_param;
		godot_method_bind *mb_slider_joint_set_param;
		godot_method_bind *mb_space_create;
		godot_method_bind *mb_space_get_direct_state;
		godot_method_bind *mb_space_get_param;
		godot_method_bind *mb_space_is_active;
		godot_method_bind *mb_space_set_active;
		godot_method_bind *mb_space_set_param;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline PhysicsServer *get_singleton()
	{
		if (!PhysicsServer::_singleton) {
			PhysicsServer::_singleton = new PhysicsServer;
		}
		return PhysicsServer::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "PhysicsServer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum BodyAxis {
		BODY_AXIS_LINEAR_X = 1,
		BODY_AXIS_LINEAR_Y = 2,
		BODY_AXIS_LINEAR_Z = 4,
		BODY_AXIS_ANGULAR_X = 8,
		BODY_AXIS_ANGULAR_Y = 16,
		BODY_AXIS_ANGULAR_Z = 32,
	};
	enum ProcessInfo {
		INFO_ACTIVE_OBJECTS = 0,
		INFO_COLLISION_PAIRS = 1,
		INFO_ISLAND_COUNT = 2,
	};
	enum AreaBodyStatus {
		AREA_BODY_ADDED = 0,
		AREA_BODY_REMOVED = 1,
	};
	enum BodyMode {
		BODY_MODE_STATIC = 0,
		BODY_MODE_KINEMATIC = 1,
		BODY_MODE_RIGID = 2,
		BODY_MODE_CHARACTER = 3,
	};
	enum ShapeType {
		SHAPE_PLANE = 0,
		SHAPE_RAY = 1,
		SHAPE_SPHERE = 2,
		SHAPE_BOX = 3,
		SHAPE_CAPSULE = 4,
		SHAPE_CYLINDER = 5,
		SHAPE_CONVEX_POLYGON = 6,
		SHAPE_CONCAVE_POLYGON = 7,
		SHAPE_HEIGHTMAP = 8,
		SHAPE_CUSTOM = 9,
	};
	enum PinJointParam {
		PIN_JOINT_BIAS = 0,
		PIN_JOINT_DAMPING = 1,
		PIN_JOINT_IMPULSE_CLAMP = 2,
	};
	enum SpaceParameter {
		SPACE_PARAM_CONTACT_RECYCLE_RADIUS = 0,
		SPACE_PARAM_CONTACT_MAX_SEPARATION = 1,
		SPACE_PARAM_BODY_MAX_ALLOWED_PENETRATION = 2,
		SPACE_PARAM_BODY_LINEAR_VELOCITY_SLEEP_THRESHOLD = 3,
		SPACE_PARAM_BODY_ANGULAR_VELOCITY_SLEEP_THRESHOLD = 4,
		SPACE_PARAM_BODY_TIME_TO_SLEEP = 5,
		SPACE_PARAM_BODY_ANGULAR_VELOCITY_DAMP_RATIO = 6,
		SPACE_PARAM_CONSTRAINT_DEFAULT_BIAS = 7,
		SPACE_PARAM_TEST_MOTION_MIN_CONTACT_DEPTH = 8,
	};
	enum ConeTwistJointParam {
		CONE_TWIST_JOINT_SWING_SPAN = 0,
		CONE_TWIST_JOINT_TWIST_SPAN = 1,
		CONE_TWIST_JOINT_BIAS = 2,
		CONE_TWIST_JOINT_SOFTNESS = 3,
		CONE_TWIST_JOINT_RELAXATION = 4,
	};
	enum JointType {
		JOINT_PIN = 0,
		JOINT_HINGE = 1,
		JOINT_SLIDER = 2,
		JOINT_CONE_TWIST = 3,
		JOINT_6DOF = 4,
	};
	enum BodyState {
		BODY_STATE_TRANSFORM = 0,
		BODY_STATE_LINEAR_VELOCITY = 1,
		BODY_STATE_ANGULAR_VELOCITY = 2,
		BODY_STATE_SLEEPING = 3,
		BODY_STATE_CAN_SLEEP = 4,
	};
	enum BodyParameter {
		BODY_PARAM_BOUNCE = 0,
		BODY_PARAM_FRICTION = 1,
		BODY_PARAM_MASS = 2,
		BODY_PARAM_GRAVITY_SCALE = 3,
		BODY_PARAM_LINEAR_DAMP = 4,
		BODY_PARAM_ANGULAR_DAMP = 5,
		BODY_PARAM_MAX = 6,
	};
	enum G6DOFJointAxisParam {
		G6DOF_JOINT_LINEAR_LOWER_LIMIT = 0,
		G6DOF_JOINT_LINEAR_UPPER_LIMIT = 1,
		G6DOF_JOINT_LINEAR_LIMIT_SOFTNESS = 2,
		G6DOF_JOINT_LINEAR_RESTITUTION = 3,
		G6DOF_JOINT_LINEAR_DAMPING = 4,
		G6DOF_JOINT_LINEAR_MOTOR_TARGET_VELOCITY = 5,
		G6DOF_JOINT_LINEAR_MOTOR_FORCE_LIMIT = 6,
		G6DOF_JOINT_ANGULAR_LOWER_LIMIT = 10,
		G6DOF_JOINT_ANGULAR_UPPER_LIMIT = 11,
		G6DOF_JOINT_ANGULAR_LIMIT_SOFTNESS = 12,
		G6DOF_JOINT_ANGULAR_DAMPING = 13,
		G6DOF_JOINT_ANGULAR_RESTITUTION = 14,
		G6DOF_JOINT_ANGULAR_FORCE_LIMIT = 15,
		G6DOF_JOINT_ANGULAR_ERP = 16,
		G6DOF_JOINT_ANGULAR_MOTOR_TARGET_VELOCITY = 17,
		G6DOF_JOINT_ANGULAR_MOTOR_FORCE_LIMIT = 18,
	};
	enum SliderJointParam {
		SLIDER_JOINT_LINEAR_LIMIT_UPPER = 0,
		SLIDER_JOINT_LINEAR_LIMIT_LOWER = 1,
		SLIDER_JOINT_LINEAR_LIMIT_SOFTNESS = 2,
		SLIDER_JOINT_LINEAR_LIMIT_RESTITUTION = 3,
		SLIDER_JOINT_LINEAR_LIMIT_DAMPING = 4,
		SLIDER_JOINT_LINEAR_MOTION_SOFTNESS = 5,
		SLIDER_JOINT_LINEAR_MOTION_RESTITUTION = 6,
		SLIDER_JOINT_LINEAR_MOTION_DAMPING = 7,
		SLIDER_JOINT_LINEAR_ORTHOGONAL_SOFTNESS = 8,
		SLIDER_JOINT_LINEAR_ORTHOGONAL_RESTITUTION = 9,
		SLIDER_JOINT_LINEAR_ORTHOGONAL_DAMPING = 10,
		SLIDER_JOINT_ANGULAR_LIMIT_UPPER = 11,
		SLIDER_JOINT_ANGULAR_LIMIT_LOWER = 12,
		SLIDER_JOINT_ANGULAR_LIMIT_SOFTNESS = 13,
		SLIDER_JOINT_ANGULAR_LIMIT_RESTITUTION = 14,
		SLIDER_JOINT_ANGULAR_LIMIT_DAMPING = 15,
		SLIDER_JOINT_ANGULAR_MOTION_SOFTNESS = 16,
		SLIDER_JOINT_ANGULAR_MOTION_RESTITUTION = 17,
		SLIDER_JOINT_ANGULAR_MOTION_DAMPING = 18,
		SLIDER_JOINT_ANGULAR_ORTHOGONAL_SOFTNESS = 19,
		SLIDER_JOINT_ANGULAR_ORTHOGONAL_RESTITUTION = 20,
		SLIDER_JOINT_ANGULAR_ORTHOGONAL_DAMPING = 21,
		SLIDER_JOINT_MAX = 22,
	};
	enum HingeJointParam {
		HINGE_JOINT_BIAS = 0,
		HINGE_JOINT_LIMIT_UPPER = 1,
		HINGE_JOINT_LIMIT_LOWER = 2,
		HINGE_JOINT_LIMIT_BIAS = 3,
		HINGE_JOINT_LIMIT_SOFTNESS = 4,
		HINGE_JOINT_LIMIT_RELAXATION = 5,
		HINGE_JOINT_MOTOR_TARGET_VELOCITY = 6,
		HINGE_JOINT_MOTOR_MAX_IMPULSE = 7,
	};
	enum G6DOFJointAxisFlag {
		G6DOF_JOINT_FLAG_ENABLE_LINEAR_LIMIT = 0,
		G6DOF_JOINT_FLAG_ENABLE_ANGULAR_LIMIT = 1,
		G6DOF_JOINT_FLAG_ENABLE_MOTOR = 4,
		G6DOF_JOINT_FLAG_ENABLE_LINEAR_MOTOR = 5,
	};
	enum HingeJointFlag {
		HINGE_JOINT_FLAG_USE_LIMIT = 0,
		HINGE_JOINT_FLAG_ENABLE_MOTOR = 1,
	};
	enum AreaSpaceOverrideMode {
		AREA_SPACE_OVERRIDE_DISABLED = 0,
		AREA_SPACE_OVERRIDE_COMBINE = 1,
		AREA_SPACE_OVERRIDE_COMBINE_REPLACE = 2,
		AREA_SPACE_OVERRIDE_REPLACE = 3,
		AREA_SPACE_OVERRIDE_REPLACE_COMBINE = 4,
	};
	enum AreaParameter {
		AREA_PARAM_GRAVITY = 0,
		AREA_PARAM_GRAVITY_VECTOR = 1,
		AREA_PARAM_GRAVITY_IS_POINT = 2,
		AREA_PARAM_GRAVITY_DISTANCE_SCALE = 3,
		AREA_PARAM_GRAVITY_POINT_ATTENUATION = 4,
		AREA_PARAM_LINEAR_DAMP = 5,
		AREA_PARAM_ANGULAR_DAMP = 6,
		AREA_PARAM_PRIORITY = 7,
	};

	// constants

	// methods
	void area_add_shape(const RID area, const RID shape, const Transform transform = Transform(), const bool disabled = false);
	void area_attach_object_instance_id(const RID area, const int64_t id);
	void area_clear_shapes(const RID area);
	RID area_create();
	int64_t area_get_object_instance_id(const RID area) const;
	Variant area_get_param(const RID area, const int64_t param) const;
	RID area_get_shape(const RID area, const int64_t shape_idx) const;
	int64_t area_get_shape_count(const RID area) const;
	Transform area_get_shape_transform(const RID area, const int64_t shape_idx) const;
	RID area_get_space(const RID area) const;
	PhysicsServer::AreaSpaceOverrideMode area_get_space_override_mode(const RID area) const;
	Transform area_get_transform(const RID area) const;
	bool area_is_ray_pickable(const RID area) const;
	void area_remove_shape(const RID area, const int64_t shape_idx);
	void area_set_area_monitor_callback(const RID area, const Object *receiver, const String method);
	void area_set_collision_layer(const RID area, const int64_t layer);
	void area_set_collision_mask(const RID area, const int64_t mask);
	void area_set_monitor_callback(const RID area, const Object *receiver, const String method);
	void area_set_monitorable(const RID area, const bool monitorable);
	void area_set_param(const RID area, const int64_t param, const Variant value);
	void area_set_ray_pickable(const RID area, const bool enable);
	void area_set_shape(const RID area, const int64_t shape_idx, const RID shape);
	void area_set_shape_disabled(const RID area, const int64_t shape_idx, const bool disabled);
	void area_set_shape_transform(const RID area, const int64_t shape_idx, const Transform transform);
	void area_set_space(const RID area, const RID space);
	void area_set_space_override_mode(const RID area, const int64_t mode);
	void area_set_transform(const RID area, const Transform transform);
	void body_add_central_force(const RID body, const Vector3 force);
	void body_add_collision_exception(const RID body, const RID excepted_body);
	void body_add_force(const RID body, const Vector3 force, const Vector3 position);
	void body_add_shape(const RID body, const RID shape, const Transform transform = Transform(), const bool disabled = false);
	void body_add_torque(const RID body, const Vector3 torque);
	void body_apply_central_impulse(const RID body, const Vector3 impulse);
	void body_apply_impulse(const RID body, const Vector3 position, const Vector3 impulse);
	void body_apply_torque_impulse(const RID body, const Vector3 impulse);
	void body_attach_object_instance_id(const RID body, const int64_t id);
	void body_clear_shapes(const RID body);
	RID body_create(const int64_t mode = 2, const bool init_sleeping = false);
	int64_t body_get_collision_layer(const RID body) const;
	int64_t body_get_collision_mask(const RID body) const;
	PhysicsDirectBodyState *body_get_direct_state(const RID body);
	real_t body_get_kinematic_safe_margin(const RID body) const;
	int64_t body_get_max_contacts_reported(const RID body) const;
	PhysicsServer::BodyMode body_get_mode(const RID body) const;
	int64_t body_get_object_instance_id(const RID body) const;
	real_t body_get_param(const RID body, const int64_t param) const;
	RID body_get_shape(const RID body, const int64_t shape_idx) const;
	int64_t body_get_shape_count(const RID body) const;
	Transform body_get_shape_transform(const RID body, const int64_t shape_idx) const;
	RID body_get_space(const RID body) const;
	Variant body_get_state(const RID body, const int64_t state) const;
	bool body_is_axis_locked(const RID body, const int64_t axis) const;
	bool body_is_continuous_collision_detection_enabled(const RID body) const;
	bool body_is_omitting_force_integration(const RID body) const;
	bool body_is_ray_pickable(const RID body) const;
	void body_remove_collision_exception(const RID body, const RID excepted_body);
	void body_remove_shape(const RID body, const int64_t shape_idx);
	void body_set_axis_lock(const RID body, const int64_t axis, const bool lock);
	void body_set_axis_velocity(const RID body, const Vector3 axis_velocity);
	void body_set_collision_layer(const RID body, const int64_t layer);
	void body_set_collision_mask(const RID body, const int64_t mask);
	void body_set_enable_continuous_collision_detection(const RID body, const bool enable);
	void body_set_force_integration_callback(const RID body, const Object *receiver, const String method, const Variant userdata = Variant());
	void body_set_kinematic_safe_margin(const RID body, const real_t margin);
	void body_set_max_contacts_reported(const RID body, const int64_t amount);
	void body_set_mode(const RID body, const int64_t mode);
	void body_set_omit_force_integration(const RID body, const bool enable);
	void body_set_param(const RID body, const int64_t param, const real_t value);
	void body_set_ray_pickable(const RID body, const bool enable);
	void body_set_shape(const RID body, const int64_t shape_idx, const RID shape);
	void body_set_shape_disabled(const RID body, const int64_t shape_idx, const bool disabled);
	void body_set_shape_transform(const RID body, const int64_t shape_idx, const Transform transform);
	void body_set_space(const RID body, const RID space);
	void body_set_state(const RID body, const int64_t state, const Variant value);
	real_t cone_twist_joint_get_param(const RID joint, const int64_t param) const;
	void cone_twist_joint_set_param(const RID joint, const int64_t param, const real_t value);
	void free_rid(const RID rid);
	bool generic_6dof_joint_get_flag(const RID joint, const int64_t axis, const int64_t flag);
	real_t generic_6dof_joint_get_param(const RID joint, const int64_t axis, const int64_t param);
	void generic_6dof_joint_set_flag(const RID joint, const int64_t axis, const int64_t flag, const bool enable);
	void generic_6dof_joint_set_param(const RID joint, const int64_t axis, const int64_t param, const real_t value);
	int64_t get_process_info(const int64_t process_info);
	bool hinge_joint_get_flag(const RID joint, const int64_t flag) const;
	real_t hinge_joint_get_param(const RID joint, const int64_t param) const;
	void hinge_joint_set_flag(const RID joint, const int64_t flag, const bool enabled);
	void hinge_joint_set_param(const RID joint, const int64_t param, const real_t value);
	RID joint_create_cone_twist(const RID body_A, const Transform local_ref_A, const RID body_B, const Transform local_ref_B);
	RID joint_create_generic_6dof(const RID body_A, const Transform local_ref_A, const RID body_B, const Transform local_ref_B);
	RID joint_create_hinge(const RID body_A, const Transform hinge_A, const RID body_B, const Transform hinge_B);
	RID joint_create_pin(const RID body_A, const Vector3 local_A, const RID body_B, const Vector3 local_B);
	RID joint_create_slider(const RID body_A, const Transform local_ref_A, const RID body_B, const Transform local_ref_B);
	int64_t joint_get_solver_priority(const RID joint) const;
	PhysicsServer::JointType joint_get_type(const RID joint) const;
	void joint_set_solver_priority(const RID joint, const int64_t priority);
	Vector3 pin_joint_get_local_a(const RID joint) const;
	Vector3 pin_joint_get_local_b(const RID joint) const;
	real_t pin_joint_get_param(const RID joint, const int64_t param) const;
	void pin_joint_set_local_a(const RID joint, const Vector3 local_A);
	void pin_joint_set_local_b(const RID joint, const Vector3 local_B);
	void pin_joint_set_param(const RID joint, const int64_t param, const real_t value);
	void set_active(const bool active);
	RID shape_create(const int64_t type);
	Variant shape_get_data(const RID shape) const;
	PhysicsServer::ShapeType shape_get_type(const RID shape) const;
	void shape_set_data(const RID shape, const Variant data);
	real_t slider_joint_get_param(const RID joint, const int64_t param) const;
	void slider_joint_set_param(const RID joint, const int64_t param, const real_t value);
	RID space_create();
	PhysicsDirectSpaceState *space_get_direct_state(const RID space);
	real_t space_get_param(const RID space, const int64_t param) const;
	bool space_is_active(const RID space) const;
	void space_set_active(const RID space, const bool active);
	void space_set_param(const RID space, const int64_t param, const real_t value);

};

}

#endif