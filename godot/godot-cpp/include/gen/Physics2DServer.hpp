#ifndef GODOT_CPP_PHYSICS2DSERVER_HPP
#define GODOT_CPP_PHYSICS2DSERVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Physics2DServer.hpp"

#include "Object.hpp"
namespace godot {

class Object;
class Physics2DDirectBodyState;
class Physics2DTestMotionResult;
class Physics2DDirectSpaceState;

class Physics2DServer : public Object {
	static Physics2DServer *_singleton;

	Physics2DServer();

	struct ___method_bindings {
		godot_method_bind *mb_area_add_shape;
		godot_method_bind *mb_area_attach_canvas_instance_id;
		godot_method_bind *mb_area_attach_object_instance_id;
		godot_method_bind *mb_area_clear_shapes;
		godot_method_bind *mb_area_create;
		godot_method_bind *mb_area_get_canvas_instance_id;
		godot_method_bind *mb_area_get_object_instance_id;
		godot_method_bind *mb_area_get_param;
		godot_method_bind *mb_area_get_shape;
		godot_method_bind *mb_area_get_shape_count;
		godot_method_bind *mb_area_get_shape_transform;
		godot_method_bind *mb_area_get_space;
		godot_method_bind *mb_area_get_space_override_mode;
		godot_method_bind *mb_area_get_transform;
		godot_method_bind *mb_area_remove_shape;
		godot_method_bind *mb_area_set_area_monitor_callback;
		godot_method_bind *mb_area_set_collision_layer;
		godot_method_bind *mb_area_set_collision_mask;
		godot_method_bind *mb_area_set_monitor_callback;
		godot_method_bind *mb_area_set_monitorable;
		godot_method_bind *mb_area_set_param;
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
		godot_method_bind *mb_body_attach_canvas_instance_id;
		godot_method_bind *mb_body_attach_object_instance_id;
		godot_method_bind *mb_body_clear_shapes;
		godot_method_bind *mb_body_create;
		godot_method_bind *mb_body_get_canvas_instance_id;
		godot_method_bind *mb_body_get_collision_layer;
		godot_method_bind *mb_body_get_collision_mask;
		godot_method_bind *mb_body_get_continuous_collision_detection_mode;
		godot_method_bind *mb_body_get_direct_state;
		godot_method_bind *mb_body_get_max_contacts_reported;
		godot_method_bind *mb_body_get_mode;
		godot_method_bind *mb_body_get_object_instance_id;
		godot_method_bind *mb_body_get_param;
		godot_method_bind *mb_body_get_shape;
		godot_method_bind *mb_body_get_shape_count;
		godot_method_bind *mb_body_get_shape_metadata;
		godot_method_bind *mb_body_get_shape_transform;
		godot_method_bind *mb_body_get_space;
		godot_method_bind *mb_body_get_state;
		godot_method_bind *mb_body_is_omitting_force_integration;
		godot_method_bind *mb_body_remove_collision_exception;
		godot_method_bind *mb_body_remove_shape;
		godot_method_bind *mb_body_set_axis_velocity;
		godot_method_bind *mb_body_set_collision_layer;
		godot_method_bind *mb_body_set_collision_mask;
		godot_method_bind *mb_body_set_continuous_collision_detection_mode;
		godot_method_bind *mb_body_set_force_integration_callback;
		godot_method_bind *mb_body_set_max_contacts_reported;
		godot_method_bind *mb_body_set_mode;
		godot_method_bind *mb_body_set_omit_force_integration;
		godot_method_bind *mb_body_set_param;
		godot_method_bind *mb_body_set_shape;
		godot_method_bind *mb_body_set_shape_as_one_way_collision;
		godot_method_bind *mb_body_set_shape_disabled;
		godot_method_bind *mb_body_set_shape_metadata;
		godot_method_bind *mb_body_set_shape_transform;
		godot_method_bind *mb_body_set_space;
		godot_method_bind *mb_body_set_state;
		godot_method_bind *mb_body_test_motion;
		godot_method_bind *mb_capsule_shape_create;
		godot_method_bind *mb_circle_shape_create;
		godot_method_bind *mb_concave_polygon_shape_create;
		godot_method_bind *mb_convex_polygon_shape_create;
		godot_method_bind *mb_damped_spring_joint_create;
		godot_method_bind *mb_damped_string_joint_get_param;
		godot_method_bind *mb_damped_string_joint_set_param;
		godot_method_bind *mb_free_rid;
		godot_method_bind *mb_get_process_info;
		godot_method_bind *mb_groove_joint_create;
		godot_method_bind *mb_joint_get_param;
		godot_method_bind *mb_joint_get_type;
		godot_method_bind *mb_joint_set_param;
		godot_method_bind *mb_line_shape_create;
		godot_method_bind *mb_pin_joint_create;
		godot_method_bind *mb_ray_shape_create;
		godot_method_bind *mb_rectangle_shape_create;
		godot_method_bind *mb_segment_shape_create;
		godot_method_bind *mb_set_active;
		godot_method_bind *mb_shape_get_data;
		godot_method_bind *mb_shape_get_type;
		godot_method_bind *mb_shape_set_data;
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

	static inline Physics2DServer *get_singleton()
	{
		if (!Physics2DServer::_singleton) {
			Physics2DServer::_singleton = new Physics2DServer;
		}
		return Physics2DServer::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "Physics2DServer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ProcessInfo {
		INFO_ACTIVE_OBJECTS = 0,
		INFO_COLLISION_PAIRS = 1,
		INFO_ISLAND_COUNT = 2,
	};
	enum AreaBodyStatus {
		AREA_BODY_ADDED = 0,
		AREA_BODY_REMOVED = 1,
	};
	enum DampedStringParam {
		DAMPED_STRING_REST_LENGTH = 0,
		DAMPED_STRING_STIFFNESS = 1,
		DAMPED_STRING_DAMPING = 2,
	};
	enum BodyMode {
		BODY_MODE_STATIC = 0,
		BODY_MODE_KINEMATIC = 1,
		BODY_MODE_RIGID = 2,
		BODY_MODE_CHARACTER = 3,
	};
	enum ShapeType {
		SHAPE_LINE = 0,
		SHAPE_RAY = 1,
		SHAPE_SEGMENT = 2,
		SHAPE_CIRCLE = 3,
		SHAPE_RECTANGLE = 4,
		SHAPE_CAPSULE = 5,
		SHAPE_CONVEX_POLYGON = 6,
		SHAPE_CONCAVE_POLYGON = 7,
		SHAPE_CUSTOM = 8,
	};
	enum JointParam {
		JOINT_PARAM_BIAS = 0,
		JOINT_PARAM_MAX_BIAS = 1,
		JOINT_PARAM_MAX_FORCE = 2,
	};
	enum SpaceParameter {
		SPACE_PARAM_CONTACT_RECYCLE_RADIUS = 0,
		SPACE_PARAM_CONTACT_MAX_SEPARATION = 1,
		SPACE_PARAM_BODY_MAX_ALLOWED_PENETRATION = 2,
		SPACE_PARAM_BODY_LINEAR_VELOCITY_SLEEP_THRESHOLD = 3,
		SPACE_PARAM_BODY_ANGULAR_VELOCITY_SLEEP_THRESHOLD = 4,
		SPACE_PARAM_BODY_TIME_TO_SLEEP = 5,
		SPACE_PARAM_CONSTRAINT_DEFAULT_BIAS = 6,
		SPACE_PARAM_TEST_MOTION_MIN_CONTACT_DEPTH = 7,
	};
	enum JointType {
		JOINT_PIN = 0,
		JOINT_GROOVE = 1,
		JOINT_DAMPED_SPRING = 2,
	};
	enum CCDMode {
		CCD_MODE_DISABLED = 0,
		CCD_MODE_CAST_RAY = 1,
		CCD_MODE_CAST_SHAPE = 2,
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
		BODY_PARAM_INERTIA = 3,
		BODY_PARAM_GRAVITY_SCALE = 4,
		BODY_PARAM_LINEAR_DAMP = 5,
		BODY_PARAM_ANGULAR_DAMP = 6,
		BODY_PARAM_MAX = 7,
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
	void area_add_shape(const RID area, const RID shape, const Transform2D transform = Transform2D(), const bool disabled = false);
	void area_attach_canvas_instance_id(const RID area, const int64_t id);
	void area_attach_object_instance_id(const RID area, const int64_t id);
	void area_clear_shapes(const RID area);
	RID area_create();
	int64_t area_get_canvas_instance_id(const RID area) const;
	int64_t area_get_object_instance_id(const RID area) const;
	Variant area_get_param(const RID area, const int64_t param) const;
	RID area_get_shape(const RID area, const int64_t shape_idx) const;
	int64_t area_get_shape_count(const RID area) const;
	Transform2D area_get_shape_transform(const RID area, const int64_t shape_idx) const;
	RID area_get_space(const RID area) const;
	Physics2DServer::AreaSpaceOverrideMode area_get_space_override_mode(const RID area) const;
	Transform2D area_get_transform(const RID area) const;
	void area_remove_shape(const RID area, const int64_t shape_idx);
	void area_set_area_monitor_callback(const RID area, const Object *receiver, const String method);
	void area_set_collision_layer(const RID area, const int64_t layer);
	void area_set_collision_mask(const RID area, const int64_t mask);
	void area_set_monitor_callback(const RID area, const Object *receiver, const String method);
	void area_set_monitorable(const RID area, const bool monitorable);
	void area_set_param(const RID area, const int64_t param, const Variant value);
	void area_set_shape(const RID area, const int64_t shape_idx, const RID shape);
	void area_set_shape_disabled(const RID area, const int64_t shape_idx, const bool disabled);
	void area_set_shape_transform(const RID area, const int64_t shape_idx, const Transform2D transform);
	void area_set_space(const RID area, const RID space);
	void area_set_space_override_mode(const RID area, const int64_t mode);
	void area_set_transform(const RID area, const Transform2D transform);
	void body_add_central_force(const RID body, const Vector2 force);
	void body_add_collision_exception(const RID body, const RID excepted_body);
	void body_add_force(const RID body, const Vector2 offset, const Vector2 force);
	void body_add_shape(const RID body, const RID shape, const Transform2D transform = Transform2D(), const bool disabled = false);
	void body_add_torque(const RID body, const real_t torque);
	void body_apply_central_impulse(const RID body, const Vector2 impulse);
	void body_apply_impulse(const RID body, const Vector2 position, const Vector2 impulse);
	void body_apply_torque_impulse(const RID body, const real_t impulse);
	void body_attach_canvas_instance_id(const RID body, const int64_t id);
	void body_attach_object_instance_id(const RID body, const int64_t id);
	void body_clear_shapes(const RID body);
	RID body_create();
	int64_t body_get_canvas_instance_id(const RID body) const;
	int64_t body_get_collision_layer(const RID body) const;
	int64_t body_get_collision_mask(const RID body) const;
	Physics2DServer::CCDMode body_get_continuous_collision_detection_mode(const RID body) const;
	Physics2DDirectBodyState *body_get_direct_state(const RID body);
	int64_t body_get_max_contacts_reported(const RID body) const;
	Physics2DServer::BodyMode body_get_mode(const RID body) const;
	int64_t body_get_object_instance_id(const RID body) const;
	real_t body_get_param(const RID body, const int64_t param) const;
	RID body_get_shape(const RID body, const int64_t shape_idx) const;
	int64_t body_get_shape_count(const RID body) const;
	Variant body_get_shape_metadata(const RID body, const int64_t shape_idx) const;
	Transform2D body_get_shape_transform(const RID body, const int64_t shape_idx) const;
	RID body_get_space(const RID body) const;
	Variant body_get_state(const RID body, const int64_t state) const;
	bool body_is_omitting_force_integration(const RID body) const;
	void body_remove_collision_exception(const RID body, const RID excepted_body);
	void body_remove_shape(const RID body, const int64_t shape_idx);
	void body_set_axis_velocity(const RID body, const Vector2 axis_velocity);
	void body_set_collision_layer(const RID body, const int64_t layer);
	void body_set_collision_mask(const RID body, const int64_t mask);
	void body_set_continuous_collision_detection_mode(const RID body, const int64_t mode);
	void body_set_force_integration_callback(const RID body, const Object *receiver, const String method, const Variant userdata = Variant());
	void body_set_max_contacts_reported(const RID body, const int64_t amount);
	void body_set_mode(const RID body, const int64_t mode);
	void body_set_omit_force_integration(const RID body, const bool enable);
	void body_set_param(const RID body, const int64_t param, const real_t value);
	void body_set_shape(const RID body, const int64_t shape_idx, const RID shape);
	void body_set_shape_as_one_way_collision(const RID body, const int64_t shape_idx, const bool enable, const real_t margin);
	void body_set_shape_disabled(const RID body, const int64_t shape_idx, const bool disabled);
	void body_set_shape_metadata(const RID body, const int64_t shape_idx, const Variant metadata);
	void body_set_shape_transform(const RID body, const int64_t shape_idx, const Transform2D transform);
	void body_set_space(const RID body, const RID space);
	void body_set_state(const RID body, const int64_t state, const Variant value);
	bool body_test_motion(const RID body, const Transform2D from, const Vector2 motion, const bool infinite_inertia, const real_t margin = 0.08, const Ref<Physics2DTestMotionResult> result = nullptr);
	RID capsule_shape_create();
	RID circle_shape_create();
	RID concave_polygon_shape_create();
	RID convex_polygon_shape_create();
	RID damped_spring_joint_create(const Vector2 anchor_a, const Vector2 anchor_b, const RID body_a, const RID body_b = RID());
	real_t damped_string_joint_get_param(const RID joint, const int64_t param) const;
	void damped_string_joint_set_param(const RID joint, const int64_t param, const real_t value);
	void free_rid(const RID rid);
	int64_t get_process_info(const int64_t process_info);
	RID groove_joint_create(const Vector2 groove1_a, const Vector2 groove2_a, const Vector2 anchor_b, const RID body_a = RID(), const RID body_b = RID());
	real_t joint_get_param(const RID joint, const int64_t param) const;
	Physics2DServer::JointType joint_get_type(const RID joint) const;
	void joint_set_param(const RID joint, const int64_t param, const real_t value);
	RID line_shape_create();
	RID pin_joint_create(const Vector2 anchor, const RID body_a, const RID body_b = RID());
	RID ray_shape_create();
	RID rectangle_shape_create();
	RID segment_shape_create();
	void set_active(const bool active);
	Variant shape_get_data(const RID shape) const;
	Physics2DServer::ShapeType shape_get_type(const RID shape) const;
	void shape_set_data(const RID shape, const Variant data);
	RID space_create();
	Physics2DDirectSpaceState *space_get_direct_state(const RID space);
	real_t space_get_param(const RID space, const int64_t param) const;
	bool space_is_active(const RID space) const;
	void space_set_active(const RID space, const bool active);
	void space_set_param(const RID space, const int64_t param, const real_t value);

};

}

#endif