#include "Physics2DServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Physics2DDirectBodyState.hpp"
#include "Physics2DTestMotionResult.hpp"
#include "Physics2DDirectSpaceState.hpp"


namespace godot {


Physics2DServer *Physics2DServer::_singleton = NULL;


Physics2DServer::Physics2DServer() {
	_owner = godot::api->godot_global_get_singleton((char *) "Physics2DServer");
}


Physics2DServer::___method_bindings Physics2DServer::___mb = {};

void Physics2DServer::___init_method_bindings() {
	___mb.mb_area_add_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "area_add_shape");
	___mb.mb_area_attach_canvas_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "area_attach_canvas_instance_id");
	___mb.mb_area_attach_object_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "area_attach_object_instance_id");
	___mb.mb_area_clear_shapes = godot::api->godot_method_bind_get_method("Physics2DServer", "area_clear_shapes");
	___mb.mb_area_create = godot::api->godot_method_bind_get_method("Physics2DServer", "area_create");
	___mb.mb_area_get_canvas_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_canvas_instance_id");
	___mb.mb_area_get_object_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_object_instance_id");
	___mb.mb_area_get_param = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_param");
	___mb.mb_area_get_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_shape");
	___mb.mb_area_get_shape_count = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_shape_count");
	___mb.mb_area_get_shape_transform = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_shape_transform");
	___mb.mb_area_get_space = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_space");
	___mb.mb_area_get_space_override_mode = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_space_override_mode");
	___mb.mb_area_get_transform = godot::api->godot_method_bind_get_method("Physics2DServer", "area_get_transform");
	___mb.mb_area_remove_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "area_remove_shape");
	___mb.mb_area_set_area_monitor_callback = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_area_monitor_callback");
	___mb.mb_area_set_collision_layer = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_collision_layer");
	___mb.mb_area_set_collision_mask = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_collision_mask");
	___mb.mb_area_set_monitor_callback = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_monitor_callback");
	___mb.mb_area_set_monitorable = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_monitorable");
	___mb.mb_area_set_param = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_param");
	___mb.mb_area_set_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_shape");
	___mb.mb_area_set_shape_disabled = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_shape_disabled");
	___mb.mb_area_set_shape_transform = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_shape_transform");
	___mb.mb_area_set_space = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_space");
	___mb.mb_area_set_space_override_mode = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_space_override_mode");
	___mb.mb_area_set_transform = godot::api->godot_method_bind_get_method("Physics2DServer", "area_set_transform");
	___mb.mb_body_add_central_force = godot::api->godot_method_bind_get_method("Physics2DServer", "body_add_central_force");
	___mb.mb_body_add_collision_exception = godot::api->godot_method_bind_get_method("Physics2DServer", "body_add_collision_exception");
	___mb.mb_body_add_force = godot::api->godot_method_bind_get_method("Physics2DServer", "body_add_force");
	___mb.mb_body_add_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "body_add_shape");
	___mb.mb_body_add_torque = godot::api->godot_method_bind_get_method("Physics2DServer", "body_add_torque");
	___mb.mb_body_apply_central_impulse = godot::api->godot_method_bind_get_method("Physics2DServer", "body_apply_central_impulse");
	___mb.mb_body_apply_impulse = godot::api->godot_method_bind_get_method("Physics2DServer", "body_apply_impulse");
	___mb.mb_body_apply_torque_impulse = godot::api->godot_method_bind_get_method("Physics2DServer", "body_apply_torque_impulse");
	___mb.mb_body_attach_canvas_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "body_attach_canvas_instance_id");
	___mb.mb_body_attach_object_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "body_attach_object_instance_id");
	___mb.mb_body_clear_shapes = godot::api->godot_method_bind_get_method("Physics2DServer", "body_clear_shapes");
	___mb.mb_body_create = godot::api->godot_method_bind_get_method("Physics2DServer", "body_create");
	___mb.mb_body_get_canvas_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_canvas_instance_id");
	___mb.mb_body_get_collision_layer = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_collision_layer");
	___mb.mb_body_get_collision_mask = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_collision_mask");
	___mb.mb_body_get_continuous_collision_detection_mode = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_continuous_collision_detection_mode");
	___mb.mb_body_get_direct_state = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_direct_state");
	___mb.mb_body_get_max_contacts_reported = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_max_contacts_reported");
	___mb.mb_body_get_mode = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_mode");
	___mb.mb_body_get_object_instance_id = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_object_instance_id");
	___mb.mb_body_get_param = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_param");
	___mb.mb_body_get_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_shape");
	___mb.mb_body_get_shape_count = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_shape_count");
	___mb.mb_body_get_shape_metadata = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_shape_metadata");
	___mb.mb_body_get_shape_transform = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_shape_transform");
	___mb.mb_body_get_space = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_space");
	___mb.mb_body_get_state = godot::api->godot_method_bind_get_method("Physics2DServer", "body_get_state");
	___mb.mb_body_is_omitting_force_integration = godot::api->godot_method_bind_get_method("Physics2DServer", "body_is_omitting_force_integration");
	___mb.mb_body_remove_collision_exception = godot::api->godot_method_bind_get_method("Physics2DServer", "body_remove_collision_exception");
	___mb.mb_body_remove_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "body_remove_shape");
	___mb.mb_body_set_axis_velocity = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_axis_velocity");
	___mb.mb_body_set_collision_layer = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_collision_layer");
	___mb.mb_body_set_collision_mask = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_collision_mask");
	___mb.mb_body_set_continuous_collision_detection_mode = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_continuous_collision_detection_mode");
	___mb.mb_body_set_force_integration_callback = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_force_integration_callback");
	___mb.mb_body_set_max_contacts_reported = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_max_contacts_reported");
	___mb.mb_body_set_mode = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_mode");
	___mb.mb_body_set_omit_force_integration = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_omit_force_integration");
	___mb.mb_body_set_param = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_param");
	___mb.mb_body_set_shape = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_shape");
	___mb.mb_body_set_shape_as_one_way_collision = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_shape_as_one_way_collision");
	___mb.mb_body_set_shape_disabled = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_shape_disabled");
	___mb.mb_body_set_shape_metadata = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_shape_metadata");
	___mb.mb_body_set_shape_transform = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_shape_transform");
	___mb.mb_body_set_space = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_space");
	___mb.mb_body_set_state = godot::api->godot_method_bind_get_method("Physics2DServer", "body_set_state");
	___mb.mb_body_test_motion = godot::api->godot_method_bind_get_method("Physics2DServer", "body_test_motion");
	___mb.mb_capsule_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "capsule_shape_create");
	___mb.mb_circle_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "circle_shape_create");
	___mb.mb_concave_polygon_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "concave_polygon_shape_create");
	___mb.mb_convex_polygon_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "convex_polygon_shape_create");
	___mb.mb_damped_spring_joint_create = godot::api->godot_method_bind_get_method("Physics2DServer", "damped_spring_joint_create");
	___mb.mb_damped_string_joint_get_param = godot::api->godot_method_bind_get_method("Physics2DServer", "damped_string_joint_get_param");
	___mb.mb_damped_string_joint_set_param = godot::api->godot_method_bind_get_method("Physics2DServer", "damped_string_joint_set_param");
	___mb.mb_free_rid = godot::api->godot_method_bind_get_method("Physics2DServer", "free_rid");
	___mb.mb_get_process_info = godot::api->godot_method_bind_get_method("Physics2DServer", "get_process_info");
	___mb.mb_groove_joint_create = godot::api->godot_method_bind_get_method("Physics2DServer", "groove_joint_create");
	___mb.mb_joint_get_param = godot::api->godot_method_bind_get_method("Physics2DServer", "joint_get_param");
	___mb.mb_joint_get_type = godot::api->godot_method_bind_get_method("Physics2DServer", "joint_get_type");
	___mb.mb_joint_set_param = godot::api->godot_method_bind_get_method("Physics2DServer", "joint_set_param");
	___mb.mb_line_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "line_shape_create");
	___mb.mb_pin_joint_create = godot::api->godot_method_bind_get_method("Physics2DServer", "pin_joint_create");
	___mb.mb_ray_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "ray_shape_create");
	___mb.mb_rectangle_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "rectangle_shape_create");
	___mb.mb_segment_shape_create = godot::api->godot_method_bind_get_method("Physics2DServer", "segment_shape_create");
	___mb.mb_set_active = godot::api->godot_method_bind_get_method("Physics2DServer", "set_active");
	___mb.mb_shape_get_data = godot::api->godot_method_bind_get_method("Physics2DServer", "shape_get_data");
	___mb.mb_shape_get_type = godot::api->godot_method_bind_get_method("Physics2DServer", "shape_get_type");
	___mb.mb_shape_set_data = godot::api->godot_method_bind_get_method("Physics2DServer", "shape_set_data");
	___mb.mb_space_create = godot::api->godot_method_bind_get_method("Physics2DServer", "space_create");
	___mb.mb_space_get_direct_state = godot::api->godot_method_bind_get_method("Physics2DServer", "space_get_direct_state");
	___mb.mb_space_get_param = godot::api->godot_method_bind_get_method("Physics2DServer", "space_get_param");
	___mb.mb_space_is_active = godot::api->godot_method_bind_get_method("Physics2DServer", "space_is_active");
	___mb.mb_space_set_active = godot::api->godot_method_bind_get_method("Physics2DServer", "space_set_active");
	___mb.mb_space_set_param = godot::api->godot_method_bind_get_method("Physics2DServer", "space_set_param");
}

void Physics2DServer::area_add_shape(const RID area, const RID shape, const Transform2D transform, const bool disabled) {
	___godot_icall_void_RID_RID_Transform2D_bool(___mb.mb_area_add_shape, (const Object *) this, area, shape, transform, disabled);
}

void Physics2DServer::area_attach_canvas_instance_id(const RID area, const int64_t id) {
	___godot_icall_void_RID_int(___mb.mb_area_attach_canvas_instance_id, (const Object *) this, area, id);
}

void Physics2DServer::area_attach_object_instance_id(const RID area, const int64_t id) {
	___godot_icall_void_RID_int(___mb.mb_area_attach_object_instance_id, (const Object *) this, area, id);
}

void Physics2DServer::area_clear_shapes(const RID area) {
	___godot_icall_void_RID(___mb.mb_area_clear_shapes, (const Object *) this, area);
}

RID Physics2DServer::area_create() {
	return ___godot_icall_RID(___mb.mb_area_create, (const Object *) this);
}

int64_t Physics2DServer::area_get_canvas_instance_id(const RID area) const {
	return ___godot_icall_int_RID(___mb.mb_area_get_canvas_instance_id, (const Object *) this, area);
}

int64_t Physics2DServer::area_get_object_instance_id(const RID area) const {
	return ___godot_icall_int_RID(___mb.mb_area_get_object_instance_id, (const Object *) this, area);
}

Variant Physics2DServer::area_get_param(const RID area, const int64_t param) const {
	return ___godot_icall_Variant_RID_int(___mb.mb_area_get_param, (const Object *) this, area, param);
}

RID Physics2DServer::area_get_shape(const RID area, const int64_t shape_idx) const {
	return ___godot_icall_RID_RID_int(___mb.mb_area_get_shape, (const Object *) this, area, shape_idx);
}

int64_t Physics2DServer::area_get_shape_count(const RID area) const {
	return ___godot_icall_int_RID(___mb.mb_area_get_shape_count, (const Object *) this, area);
}

Transform2D Physics2DServer::area_get_shape_transform(const RID area, const int64_t shape_idx) const {
	return ___godot_icall_Transform2D_RID_int(___mb.mb_area_get_shape_transform, (const Object *) this, area, shape_idx);
}

RID Physics2DServer::area_get_space(const RID area) const {
	return ___godot_icall_RID_RID(___mb.mb_area_get_space, (const Object *) this, area);
}

Physics2DServer::AreaSpaceOverrideMode Physics2DServer::area_get_space_override_mode(const RID area) const {
	return (Physics2DServer::AreaSpaceOverrideMode) ___godot_icall_int_RID(___mb.mb_area_get_space_override_mode, (const Object *) this, area);
}

Transform2D Physics2DServer::area_get_transform(const RID area) const {
	return ___godot_icall_Transform2D_RID(___mb.mb_area_get_transform, (const Object *) this, area);
}

void Physics2DServer::area_remove_shape(const RID area, const int64_t shape_idx) {
	___godot_icall_void_RID_int(___mb.mb_area_remove_shape, (const Object *) this, area, shape_idx);
}

void Physics2DServer::area_set_area_monitor_callback(const RID area, const Object *receiver, const String method) {
	___godot_icall_void_RID_Object_String(___mb.mb_area_set_area_monitor_callback, (const Object *) this, area, receiver, method);
}

void Physics2DServer::area_set_collision_layer(const RID area, const int64_t layer) {
	___godot_icall_void_RID_int(___mb.mb_area_set_collision_layer, (const Object *) this, area, layer);
}

void Physics2DServer::area_set_collision_mask(const RID area, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_area_set_collision_mask, (const Object *) this, area, mask);
}

void Physics2DServer::area_set_monitor_callback(const RID area, const Object *receiver, const String method) {
	___godot_icall_void_RID_Object_String(___mb.mb_area_set_monitor_callback, (const Object *) this, area, receiver, method);
}

void Physics2DServer::area_set_monitorable(const RID area, const bool monitorable) {
	___godot_icall_void_RID_bool(___mb.mb_area_set_monitorable, (const Object *) this, area, monitorable);
}

void Physics2DServer::area_set_param(const RID area, const int64_t param, const Variant value) {
	___godot_icall_void_RID_int_Variant(___mb.mb_area_set_param, (const Object *) this, area, param, value);
}

void Physics2DServer::area_set_shape(const RID area, const int64_t shape_idx, const RID shape) {
	___godot_icall_void_RID_int_RID(___mb.mb_area_set_shape, (const Object *) this, area, shape_idx, shape);
}

void Physics2DServer::area_set_shape_disabled(const RID area, const int64_t shape_idx, const bool disabled) {
	___godot_icall_void_RID_int_bool(___mb.mb_area_set_shape_disabled, (const Object *) this, area, shape_idx, disabled);
}

void Physics2DServer::area_set_shape_transform(const RID area, const int64_t shape_idx, const Transform2D transform) {
	___godot_icall_void_RID_int_Transform2D(___mb.mb_area_set_shape_transform, (const Object *) this, area, shape_idx, transform);
}

void Physics2DServer::area_set_space(const RID area, const RID space) {
	___godot_icall_void_RID_RID(___mb.mb_area_set_space, (const Object *) this, area, space);
}

void Physics2DServer::area_set_space_override_mode(const RID area, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_area_set_space_override_mode, (const Object *) this, area, mode);
}

void Physics2DServer::area_set_transform(const RID area, const Transform2D transform) {
	___godot_icall_void_RID_Transform2D(___mb.mb_area_set_transform, (const Object *) this, area, transform);
}

void Physics2DServer::body_add_central_force(const RID body, const Vector2 force) {
	___godot_icall_void_RID_Vector2(___mb.mb_body_add_central_force, (const Object *) this, body, force);
}

void Physics2DServer::body_add_collision_exception(const RID body, const RID excepted_body) {
	___godot_icall_void_RID_RID(___mb.mb_body_add_collision_exception, (const Object *) this, body, excepted_body);
}

void Physics2DServer::body_add_force(const RID body, const Vector2 offset, const Vector2 force) {
	___godot_icall_void_RID_Vector2_Vector2(___mb.mb_body_add_force, (const Object *) this, body, offset, force);
}

void Physics2DServer::body_add_shape(const RID body, const RID shape, const Transform2D transform, const bool disabled) {
	___godot_icall_void_RID_RID_Transform2D_bool(___mb.mb_body_add_shape, (const Object *) this, body, shape, transform, disabled);
}

void Physics2DServer::body_add_torque(const RID body, const real_t torque) {
	___godot_icall_void_RID_float(___mb.mb_body_add_torque, (const Object *) this, body, torque);
}

void Physics2DServer::body_apply_central_impulse(const RID body, const Vector2 impulse) {
	___godot_icall_void_RID_Vector2(___mb.mb_body_apply_central_impulse, (const Object *) this, body, impulse);
}

void Physics2DServer::body_apply_impulse(const RID body, const Vector2 position, const Vector2 impulse) {
	___godot_icall_void_RID_Vector2_Vector2(___mb.mb_body_apply_impulse, (const Object *) this, body, position, impulse);
}

void Physics2DServer::body_apply_torque_impulse(const RID body, const real_t impulse) {
	___godot_icall_void_RID_float(___mb.mb_body_apply_torque_impulse, (const Object *) this, body, impulse);
}

void Physics2DServer::body_attach_canvas_instance_id(const RID body, const int64_t id) {
	___godot_icall_void_RID_int(___mb.mb_body_attach_canvas_instance_id, (const Object *) this, body, id);
}

void Physics2DServer::body_attach_object_instance_id(const RID body, const int64_t id) {
	___godot_icall_void_RID_int(___mb.mb_body_attach_object_instance_id, (const Object *) this, body, id);
}

void Physics2DServer::body_clear_shapes(const RID body) {
	___godot_icall_void_RID(___mb.mb_body_clear_shapes, (const Object *) this, body);
}

RID Physics2DServer::body_create() {
	return ___godot_icall_RID(___mb.mb_body_create, (const Object *) this);
}

int64_t Physics2DServer::body_get_canvas_instance_id(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_canvas_instance_id, (const Object *) this, body);
}

int64_t Physics2DServer::body_get_collision_layer(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_collision_layer, (const Object *) this, body);
}

int64_t Physics2DServer::body_get_collision_mask(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_collision_mask, (const Object *) this, body);
}

Physics2DServer::CCDMode Physics2DServer::body_get_continuous_collision_detection_mode(const RID body) const {
	return (Physics2DServer::CCDMode) ___godot_icall_int_RID(___mb.mb_body_get_continuous_collision_detection_mode, (const Object *) this, body);
}

Physics2DDirectBodyState *Physics2DServer::body_get_direct_state(const RID body) {
	return (Physics2DDirectBodyState *) ___godot_icall_Object_RID(___mb.mb_body_get_direct_state, (const Object *) this, body);
}

int64_t Physics2DServer::body_get_max_contacts_reported(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_max_contacts_reported, (const Object *) this, body);
}

Physics2DServer::BodyMode Physics2DServer::body_get_mode(const RID body) const {
	return (Physics2DServer::BodyMode) ___godot_icall_int_RID(___mb.mb_body_get_mode, (const Object *) this, body);
}

int64_t Physics2DServer::body_get_object_instance_id(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_object_instance_id, (const Object *) this, body);
}

real_t Physics2DServer::body_get_param(const RID body, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_body_get_param, (const Object *) this, body, param);
}

RID Physics2DServer::body_get_shape(const RID body, const int64_t shape_idx) const {
	return ___godot_icall_RID_RID_int(___mb.mb_body_get_shape, (const Object *) this, body, shape_idx);
}

int64_t Physics2DServer::body_get_shape_count(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_shape_count, (const Object *) this, body);
}

Variant Physics2DServer::body_get_shape_metadata(const RID body, const int64_t shape_idx) const {
	return ___godot_icall_Variant_RID_int(___mb.mb_body_get_shape_metadata, (const Object *) this, body, shape_idx);
}

Transform2D Physics2DServer::body_get_shape_transform(const RID body, const int64_t shape_idx) const {
	return ___godot_icall_Transform2D_RID_int(___mb.mb_body_get_shape_transform, (const Object *) this, body, shape_idx);
}

RID Physics2DServer::body_get_space(const RID body) const {
	return ___godot_icall_RID_RID(___mb.mb_body_get_space, (const Object *) this, body);
}

Variant Physics2DServer::body_get_state(const RID body, const int64_t state) const {
	return ___godot_icall_Variant_RID_int(___mb.mb_body_get_state, (const Object *) this, body, state);
}

bool Physics2DServer::body_is_omitting_force_integration(const RID body) const {
	return ___godot_icall_bool_RID(___mb.mb_body_is_omitting_force_integration, (const Object *) this, body);
}

void Physics2DServer::body_remove_collision_exception(const RID body, const RID excepted_body) {
	___godot_icall_void_RID_RID(___mb.mb_body_remove_collision_exception, (const Object *) this, body, excepted_body);
}

void Physics2DServer::body_remove_shape(const RID body, const int64_t shape_idx) {
	___godot_icall_void_RID_int(___mb.mb_body_remove_shape, (const Object *) this, body, shape_idx);
}

void Physics2DServer::body_set_axis_velocity(const RID body, const Vector2 axis_velocity) {
	___godot_icall_void_RID_Vector2(___mb.mb_body_set_axis_velocity, (const Object *) this, body, axis_velocity);
}

void Physics2DServer::body_set_collision_layer(const RID body, const int64_t layer) {
	___godot_icall_void_RID_int(___mb.mb_body_set_collision_layer, (const Object *) this, body, layer);
}

void Physics2DServer::body_set_collision_mask(const RID body, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_body_set_collision_mask, (const Object *) this, body, mask);
}

void Physics2DServer::body_set_continuous_collision_detection_mode(const RID body, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_body_set_continuous_collision_detection_mode, (const Object *) this, body, mode);
}

void Physics2DServer::body_set_force_integration_callback(const RID body, const Object *receiver, const String method, const Variant userdata) {
	___godot_icall_void_RID_Object_String_Variant(___mb.mb_body_set_force_integration_callback, (const Object *) this, body, receiver, method, userdata);
}

void Physics2DServer::body_set_max_contacts_reported(const RID body, const int64_t amount) {
	___godot_icall_void_RID_int(___mb.mb_body_set_max_contacts_reported, (const Object *) this, body, amount);
}

void Physics2DServer::body_set_mode(const RID body, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_body_set_mode, (const Object *) this, body, mode);
}

void Physics2DServer::body_set_omit_force_integration(const RID body, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_body_set_omit_force_integration, (const Object *) this, body, enable);
}

void Physics2DServer::body_set_param(const RID body, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_body_set_param, (const Object *) this, body, param, value);
}

void Physics2DServer::body_set_shape(const RID body, const int64_t shape_idx, const RID shape) {
	___godot_icall_void_RID_int_RID(___mb.mb_body_set_shape, (const Object *) this, body, shape_idx, shape);
}

void Physics2DServer::body_set_shape_as_one_way_collision(const RID body, const int64_t shape_idx, const bool enable, const real_t margin) {
	___godot_icall_void_RID_int_bool_float(___mb.mb_body_set_shape_as_one_way_collision, (const Object *) this, body, shape_idx, enable, margin);
}

void Physics2DServer::body_set_shape_disabled(const RID body, const int64_t shape_idx, const bool disabled) {
	___godot_icall_void_RID_int_bool(___mb.mb_body_set_shape_disabled, (const Object *) this, body, shape_idx, disabled);
}

void Physics2DServer::body_set_shape_metadata(const RID body, const int64_t shape_idx, const Variant metadata) {
	___godot_icall_void_RID_int_Variant(___mb.mb_body_set_shape_metadata, (const Object *) this, body, shape_idx, metadata);
}

void Physics2DServer::body_set_shape_transform(const RID body, const int64_t shape_idx, const Transform2D transform) {
	___godot_icall_void_RID_int_Transform2D(___mb.mb_body_set_shape_transform, (const Object *) this, body, shape_idx, transform);
}

void Physics2DServer::body_set_space(const RID body, const RID space) {
	___godot_icall_void_RID_RID(___mb.mb_body_set_space, (const Object *) this, body, space);
}

void Physics2DServer::body_set_state(const RID body, const int64_t state, const Variant value) {
	___godot_icall_void_RID_int_Variant(___mb.mb_body_set_state, (const Object *) this, body, state, value);
}

bool Physics2DServer::body_test_motion(const RID body, const Transform2D from, const Vector2 motion, const bool infinite_inertia, const real_t margin, const Ref<Physics2DTestMotionResult> result) {
	return ___godot_icall_bool_RID_Transform2D_Vector2_bool_float_Object(___mb.mb_body_test_motion, (const Object *) this, body, from, motion, infinite_inertia, margin, result.ptr());
}

RID Physics2DServer::capsule_shape_create() {
	return ___godot_icall_RID(___mb.mb_capsule_shape_create, (const Object *) this);
}

RID Physics2DServer::circle_shape_create() {
	return ___godot_icall_RID(___mb.mb_circle_shape_create, (const Object *) this);
}

RID Physics2DServer::concave_polygon_shape_create() {
	return ___godot_icall_RID(___mb.mb_concave_polygon_shape_create, (const Object *) this);
}

RID Physics2DServer::convex_polygon_shape_create() {
	return ___godot_icall_RID(___mb.mb_convex_polygon_shape_create, (const Object *) this);
}

RID Physics2DServer::damped_spring_joint_create(const Vector2 anchor_a, const Vector2 anchor_b, const RID body_a, const RID body_b) {
	return ___godot_icall_RID_Vector2_Vector2_RID_RID(___mb.mb_damped_spring_joint_create, (const Object *) this, anchor_a, anchor_b, body_a, body_b);
}

real_t Physics2DServer::damped_string_joint_get_param(const RID joint, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_damped_string_joint_get_param, (const Object *) this, joint, param);
}

void Physics2DServer::damped_string_joint_set_param(const RID joint, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_damped_string_joint_set_param, (const Object *) this, joint, param, value);
}

void Physics2DServer::free_rid(const RID rid) {
	___godot_icall_void_RID(___mb.mb_free_rid, (const Object *) this, rid);
}

int64_t Physics2DServer::get_process_info(const int64_t process_info) {
	return ___godot_icall_int_int(___mb.mb_get_process_info, (const Object *) this, process_info);
}

RID Physics2DServer::groove_joint_create(const Vector2 groove1_a, const Vector2 groove2_a, const Vector2 anchor_b, const RID body_a, const RID body_b) {
	return ___godot_icall_RID_Vector2_Vector2_Vector2_RID_RID(___mb.mb_groove_joint_create, (const Object *) this, groove1_a, groove2_a, anchor_b, body_a, body_b);
}

real_t Physics2DServer::joint_get_param(const RID joint, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_joint_get_param, (const Object *) this, joint, param);
}

Physics2DServer::JointType Physics2DServer::joint_get_type(const RID joint) const {
	return (Physics2DServer::JointType) ___godot_icall_int_RID(___mb.mb_joint_get_type, (const Object *) this, joint);
}

void Physics2DServer::joint_set_param(const RID joint, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_joint_set_param, (const Object *) this, joint, param, value);
}

RID Physics2DServer::line_shape_create() {
	return ___godot_icall_RID(___mb.mb_line_shape_create, (const Object *) this);
}

RID Physics2DServer::pin_joint_create(const Vector2 anchor, const RID body_a, const RID body_b) {
	return ___godot_icall_RID_Vector2_RID_RID(___mb.mb_pin_joint_create, (const Object *) this, anchor, body_a, body_b);
}

RID Physics2DServer::ray_shape_create() {
	return ___godot_icall_RID(___mb.mb_ray_shape_create, (const Object *) this);
}

RID Physics2DServer::rectangle_shape_create() {
	return ___godot_icall_RID(___mb.mb_rectangle_shape_create, (const Object *) this);
}

RID Physics2DServer::segment_shape_create() {
	return ___godot_icall_RID(___mb.mb_segment_shape_create, (const Object *) this);
}

void Physics2DServer::set_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_active, (const Object *) this, active);
}

Variant Physics2DServer::shape_get_data(const RID shape) const {
	return ___godot_icall_Variant_RID(___mb.mb_shape_get_data, (const Object *) this, shape);
}

Physics2DServer::ShapeType Physics2DServer::shape_get_type(const RID shape) const {
	return (Physics2DServer::ShapeType) ___godot_icall_int_RID(___mb.mb_shape_get_type, (const Object *) this, shape);
}

void Physics2DServer::shape_set_data(const RID shape, const Variant data) {
	___godot_icall_void_RID_Variant(___mb.mb_shape_set_data, (const Object *) this, shape, data);
}

RID Physics2DServer::space_create() {
	return ___godot_icall_RID(___mb.mb_space_create, (const Object *) this);
}

Physics2DDirectSpaceState *Physics2DServer::space_get_direct_state(const RID space) {
	return (Physics2DDirectSpaceState *) ___godot_icall_Object_RID(___mb.mb_space_get_direct_state, (const Object *) this, space);
}

real_t Physics2DServer::space_get_param(const RID space, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_space_get_param, (const Object *) this, space, param);
}

bool Physics2DServer::space_is_active(const RID space) const {
	return ___godot_icall_bool_RID(___mb.mb_space_is_active, (const Object *) this, space);
}

void Physics2DServer::space_set_active(const RID space, const bool active) {
	___godot_icall_void_RID_bool(___mb.mb_space_set_active, (const Object *) this, space, active);
}

void Physics2DServer::space_set_param(const RID space, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_space_set_param, (const Object *) this, space, param, value);
}

}