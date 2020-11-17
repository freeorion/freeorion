#include "PhysicsServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "PhysicsDirectBodyState.hpp"
#include "PhysicsDirectSpaceState.hpp"


namespace godot {


PhysicsServer *PhysicsServer::_singleton = NULL;


PhysicsServer::PhysicsServer() {
	_owner = godot::api->godot_global_get_singleton((char *) "PhysicsServer");
}


PhysicsServer::___method_bindings PhysicsServer::___mb = {};

void PhysicsServer::___init_method_bindings() {
	___mb.mb_area_add_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "area_add_shape");
	___mb.mb_area_attach_object_instance_id = godot::api->godot_method_bind_get_method("PhysicsServer", "area_attach_object_instance_id");
	___mb.mb_area_clear_shapes = godot::api->godot_method_bind_get_method("PhysicsServer", "area_clear_shapes");
	___mb.mb_area_create = godot::api->godot_method_bind_get_method("PhysicsServer", "area_create");
	___mb.mb_area_get_object_instance_id = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_object_instance_id");
	___mb.mb_area_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_param");
	___mb.mb_area_get_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_shape");
	___mb.mb_area_get_shape_count = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_shape_count");
	___mb.mb_area_get_shape_transform = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_shape_transform");
	___mb.mb_area_get_space = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_space");
	___mb.mb_area_get_space_override_mode = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_space_override_mode");
	___mb.mb_area_get_transform = godot::api->godot_method_bind_get_method("PhysicsServer", "area_get_transform");
	___mb.mb_area_is_ray_pickable = godot::api->godot_method_bind_get_method("PhysicsServer", "area_is_ray_pickable");
	___mb.mb_area_remove_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "area_remove_shape");
	___mb.mb_area_set_area_monitor_callback = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_area_monitor_callback");
	___mb.mb_area_set_collision_layer = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_collision_layer");
	___mb.mb_area_set_collision_mask = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_collision_mask");
	___mb.mb_area_set_monitor_callback = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_monitor_callback");
	___mb.mb_area_set_monitorable = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_monitorable");
	___mb.mb_area_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_param");
	___mb.mb_area_set_ray_pickable = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_ray_pickable");
	___mb.mb_area_set_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_shape");
	___mb.mb_area_set_shape_disabled = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_shape_disabled");
	___mb.mb_area_set_shape_transform = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_shape_transform");
	___mb.mb_area_set_space = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_space");
	___mb.mb_area_set_space_override_mode = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_space_override_mode");
	___mb.mb_area_set_transform = godot::api->godot_method_bind_get_method("PhysicsServer", "area_set_transform");
	___mb.mb_body_add_central_force = godot::api->godot_method_bind_get_method("PhysicsServer", "body_add_central_force");
	___mb.mb_body_add_collision_exception = godot::api->godot_method_bind_get_method("PhysicsServer", "body_add_collision_exception");
	___mb.mb_body_add_force = godot::api->godot_method_bind_get_method("PhysicsServer", "body_add_force");
	___mb.mb_body_add_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "body_add_shape");
	___mb.mb_body_add_torque = godot::api->godot_method_bind_get_method("PhysicsServer", "body_add_torque");
	___mb.mb_body_apply_central_impulse = godot::api->godot_method_bind_get_method("PhysicsServer", "body_apply_central_impulse");
	___mb.mb_body_apply_impulse = godot::api->godot_method_bind_get_method("PhysicsServer", "body_apply_impulse");
	___mb.mb_body_apply_torque_impulse = godot::api->godot_method_bind_get_method("PhysicsServer", "body_apply_torque_impulse");
	___mb.mb_body_attach_object_instance_id = godot::api->godot_method_bind_get_method("PhysicsServer", "body_attach_object_instance_id");
	___mb.mb_body_clear_shapes = godot::api->godot_method_bind_get_method("PhysicsServer", "body_clear_shapes");
	___mb.mb_body_create = godot::api->godot_method_bind_get_method("PhysicsServer", "body_create");
	___mb.mb_body_get_collision_layer = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_collision_layer");
	___mb.mb_body_get_collision_mask = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_collision_mask");
	___mb.mb_body_get_direct_state = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_direct_state");
	___mb.mb_body_get_kinematic_safe_margin = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_kinematic_safe_margin");
	___mb.mb_body_get_max_contacts_reported = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_max_contacts_reported");
	___mb.mb_body_get_mode = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_mode");
	___mb.mb_body_get_object_instance_id = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_object_instance_id");
	___mb.mb_body_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_param");
	___mb.mb_body_get_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_shape");
	___mb.mb_body_get_shape_count = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_shape_count");
	___mb.mb_body_get_shape_transform = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_shape_transform");
	___mb.mb_body_get_space = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_space");
	___mb.mb_body_get_state = godot::api->godot_method_bind_get_method("PhysicsServer", "body_get_state");
	___mb.mb_body_is_axis_locked = godot::api->godot_method_bind_get_method("PhysicsServer", "body_is_axis_locked");
	___mb.mb_body_is_continuous_collision_detection_enabled = godot::api->godot_method_bind_get_method("PhysicsServer", "body_is_continuous_collision_detection_enabled");
	___mb.mb_body_is_omitting_force_integration = godot::api->godot_method_bind_get_method("PhysicsServer", "body_is_omitting_force_integration");
	___mb.mb_body_is_ray_pickable = godot::api->godot_method_bind_get_method("PhysicsServer", "body_is_ray_pickable");
	___mb.mb_body_remove_collision_exception = godot::api->godot_method_bind_get_method("PhysicsServer", "body_remove_collision_exception");
	___mb.mb_body_remove_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "body_remove_shape");
	___mb.mb_body_set_axis_lock = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_axis_lock");
	___mb.mb_body_set_axis_velocity = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_axis_velocity");
	___mb.mb_body_set_collision_layer = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_collision_layer");
	___mb.mb_body_set_collision_mask = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_collision_mask");
	___mb.mb_body_set_enable_continuous_collision_detection = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_enable_continuous_collision_detection");
	___mb.mb_body_set_force_integration_callback = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_force_integration_callback");
	___mb.mb_body_set_kinematic_safe_margin = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_kinematic_safe_margin");
	___mb.mb_body_set_max_contacts_reported = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_max_contacts_reported");
	___mb.mb_body_set_mode = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_mode");
	___mb.mb_body_set_omit_force_integration = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_omit_force_integration");
	___mb.mb_body_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_param");
	___mb.mb_body_set_ray_pickable = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_ray_pickable");
	___mb.mb_body_set_shape = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_shape");
	___mb.mb_body_set_shape_disabled = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_shape_disabled");
	___mb.mb_body_set_shape_transform = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_shape_transform");
	___mb.mb_body_set_space = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_space");
	___mb.mb_body_set_state = godot::api->godot_method_bind_get_method("PhysicsServer", "body_set_state");
	___mb.mb_cone_twist_joint_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "cone_twist_joint_get_param");
	___mb.mb_cone_twist_joint_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "cone_twist_joint_set_param");
	___mb.mb_free_rid = godot::api->godot_method_bind_get_method("PhysicsServer", "free_rid");
	___mb.mb_generic_6dof_joint_get_flag = godot::api->godot_method_bind_get_method("PhysicsServer", "generic_6dof_joint_get_flag");
	___mb.mb_generic_6dof_joint_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "generic_6dof_joint_get_param");
	___mb.mb_generic_6dof_joint_set_flag = godot::api->godot_method_bind_get_method("PhysicsServer", "generic_6dof_joint_set_flag");
	___mb.mb_generic_6dof_joint_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "generic_6dof_joint_set_param");
	___mb.mb_get_process_info = godot::api->godot_method_bind_get_method("PhysicsServer", "get_process_info");
	___mb.mb_hinge_joint_get_flag = godot::api->godot_method_bind_get_method("PhysicsServer", "hinge_joint_get_flag");
	___mb.mb_hinge_joint_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "hinge_joint_get_param");
	___mb.mb_hinge_joint_set_flag = godot::api->godot_method_bind_get_method("PhysicsServer", "hinge_joint_set_flag");
	___mb.mb_hinge_joint_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "hinge_joint_set_param");
	___mb.mb_joint_create_cone_twist = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_create_cone_twist");
	___mb.mb_joint_create_generic_6dof = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_create_generic_6dof");
	___mb.mb_joint_create_hinge = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_create_hinge");
	___mb.mb_joint_create_pin = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_create_pin");
	___mb.mb_joint_create_slider = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_create_slider");
	___mb.mb_joint_get_solver_priority = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_get_solver_priority");
	___mb.mb_joint_get_type = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_get_type");
	___mb.mb_joint_set_solver_priority = godot::api->godot_method_bind_get_method("PhysicsServer", "joint_set_solver_priority");
	___mb.mb_pin_joint_get_local_a = godot::api->godot_method_bind_get_method("PhysicsServer", "pin_joint_get_local_a");
	___mb.mb_pin_joint_get_local_b = godot::api->godot_method_bind_get_method("PhysicsServer", "pin_joint_get_local_b");
	___mb.mb_pin_joint_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "pin_joint_get_param");
	___mb.mb_pin_joint_set_local_a = godot::api->godot_method_bind_get_method("PhysicsServer", "pin_joint_set_local_a");
	___mb.mb_pin_joint_set_local_b = godot::api->godot_method_bind_get_method("PhysicsServer", "pin_joint_set_local_b");
	___mb.mb_pin_joint_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "pin_joint_set_param");
	___mb.mb_set_active = godot::api->godot_method_bind_get_method("PhysicsServer", "set_active");
	___mb.mb_shape_create = godot::api->godot_method_bind_get_method("PhysicsServer", "shape_create");
	___mb.mb_shape_get_data = godot::api->godot_method_bind_get_method("PhysicsServer", "shape_get_data");
	___mb.mb_shape_get_type = godot::api->godot_method_bind_get_method("PhysicsServer", "shape_get_type");
	___mb.mb_shape_set_data = godot::api->godot_method_bind_get_method("PhysicsServer", "shape_set_data");
	___mb.mb_slider_joint_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "slider_joint_get_param");
	___mb.mb_slider_joint_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "slider_joint_set_param");
	___mb.mb_space_create = godot::api->godot_method_bind_get_method("PhysicsServer", "space_create");
	___mb.mb_space_get_direct_state = godot::api->godot_method_bind_get_method("PhysicsServer", "space_get_direct_state");
	___mb.mb_space_get_param = godot::api->godot_method_bind_get_method("PhysicsServer", "space_get_param");
	___mb.mb_space_is_active = godot::api->godot_method_bind_get_method("PhysicsServer", "space_is_active");
	___mb.mb_space_set_active = godot::api->godot_method_bind_get_method("PhysicsServer", "space_set_active");
	___mb.mb_space_set_param = godot::api->godot_method_bind_get_method("PhysicsServer", "space_set_param");
}

void PhysicsServer::area_add_shape(const RID area, const RID shape, const Transform transform, const bool disabled) {
	___godot_icall_void_RID_RID_Transform_bool(___mb.mb_area_add_shape, (const Object *) this, area, shape, transform, disabled);
}

void PhysicsServer::area_attach_object_instance_id(const RID area, const int64_t id) {
	___godot_icall_void_RID_int(___mb.mb_area_attach_object_instance_id, (const Object *) this, area, id);
}

void PhysicsServer::area_clear_shapes(const RID area) {
	___godot_icall_void_RID(___mb.mb_area_clear_shapes, (const Object *) this, area);
}

RID PhysicsServer::area_create() {
	return ___godot_icall_RID(___mb.mb_area_create, (const Object *) this);
}

int64_t PhysicsServer::area_get_object_instance_id(const RID area) const {
	return ___godot_icall_int_RID(___mb.mb_area_get_object_instance_id, (const Object *) this, area);
}

Variant PhysicsServer::area_get_param(const RID area, const int64_t param) const {
	return ___godot_icall_Variant_RID_int(___mb.mb_area_get_param, (const Object *) this, area, param);
}

RID PhysicsServer::area_get_shape(const RID area, const int64_t shape_idx) const {
	return ___godot_icall_RID_RID_int(___mb.mb_area_get_shape, (const Object *) this, area, shape_idx);
}

int64_t PhysicsServer::area_get_shape_count(const RID area) const {
	return ___godot_icall_int_RID(___mb.mb_area_get_shape_count, (const Object *) this, area);
}

Transform PhysicsServer::area_get_shape_transform(const RID area, const int64_t shape_idx) const {
	return ___godot_icall_Transform_RID_int(___mb.mb_area_get_shape_transform, (const Object *) this, area, shape_idx);
}

RID PhysicsServer::area_get_space(const RID area) const {
	return ___godot_icall_RID_RID(___mb.mb_area_get_space, (const Object *) this, area);
}

PhysicsServer::AreaSpaceOverrideMode PhysicsServer::area_get_space_override_mode(const RID area) const {
	return (PhysicsServer::AreaSpaceOverrideMode) ___godot_icall_int_RID(___mb.mb_area_get_space_override_mode, (const Object *) this, area);
}

Transform PhysicsServer::area_get_transform(const RID area) const {
	return ___godot_icall_Transform_RID(___mb.mb_area_get_transform, (const Object *) this, area);
}

bool PhysicsServer::area_is_ray_pickable(const RID area) const {
	return ___godot_icall_bool_RID(___mb.mb_area_is_ray_pickable, (const Object *) this, area);
}

void PhysicsServer::area_remove_shape(const RID area, const int64_t shape_idx) {
	___godot_icall_void_RID_int(___mb.mb_area_remove_shape, (const Object *) this, area, shape_idx);
}

void PhysicsServer::area_set_area_monitor_callback(const RID area, const Object *receiver, const String method) {
	___godot_icall_void_RID_Object_String(___mb.mb_area_set_area_monitor_callback, (const Object *) this, area, receiver, method);
}

void PhysicsServer::area_set_collision_layer(const RID area, const int64_t layer) {
	___godot_icall_void_RID_int(___mb.mb_area_set_collision_layer, (const Object *) this, area, layer);
}

void PhysicsServer::area_set_collision_mask(const RID area, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_area_set_collision_mask, (const Object *) this, area, mask);
}

void PhysicsServer::area_set_monitor_callback(const RID area, const Object *receiver, const String method) {
	___godot_icall_void_RID_Object_String(___mb.mb_area_set_monitor_callback, (const Object *) this, area, receiver, method);
}

void PhysicsServer::area_set_monitorable(const RID area, const bool monitorable) {
	___godot_icall_void_RID_bool(___mb.mb_area_set_monitorable, (const Object *) this, area, monitorable);
}

void PhysicsServer::area_set_param(const RID area, const int64_t param, const Variant value) {
	___godot_icall_void_RID_int_Variant(___mb.mb_area_set_param, (const Object *) this, area, param, value);
}

void PhysicsServer::area_set_ray_pickable(const RID area, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_area_set_ray_pickable, (const Object *) this, area, enable);
}

void PhysicsServer::area_set_shape(const RID area, const int64_t shape_idx, const RID shape) {
	___godot_icall_void_RID_int_RID(___mb.mb_area_set_shape, (const Object *) this, area, shape_idx, shape);
}

void PhysicsServer::area_set_shape_disabled(const RID area, const int64_t shape_idx, const bool disabled) {
	___godot_icall_void_RID_int_bool(___mb.mb_area_set_shape_disabled, (const Object *) this, area, shape_idx, disabled);
}

void PhysicsServer::area_set_shape_transform(const RID area, const int64_t shape_idx, const Transform transform) {
	___godot_icall_void_RID_int_Transform(___mb.mb_area_set_shape_transform, (const Object *) this, area, shape_idx, transform);
}

void PhysicsServer::area_set_space(const RID area, const RID space) {
	___godot_icall_void_RID_RID(___mb.mb_area_set_space, (const Object *) this, area, space);
}

void PhysicsServer::area_set_space_override_mode(const RID area, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_area_set_space_override_mode, (const Object *) this, area, mode);
}

void PhysicsServer::area_set_transform(const RID area, const Transform transform) {
	___godot_icall_void_RID_Transform(___mb.mb_area_set_transform, (const Object *) this, area, transform);
}

void PhysicsServer::body_add_central_force(const RID body, const Vector3 force) {
	___godot_icall_void_RID_Vector3(___mb.mb_body_add_central_force, (const Object *) this, body, force);
}

void PhysicsServer::body_add_collision_exception(const RID body, const RID excepted_body) {
	___godot_icall_void_RID_RID(___mb.mb_body_add_collision_exception, (const Object *) this, body, excepted_body);
}

void PhysicsServer::body_add_force(const RID body, const Vector3 force, const Vector3 position) {
	___godot_icall_void_RID_Vector3_Vector3(___mb.mb_body_add_force, (const Object *) this, body, force, position);
}

void PhysicsServer::body_add_shape(const RID body, const RID shape, const Transform transform, const bool disabled) {
	___godot_icall_void_RID_RID_Transform_bool(___mb.mb_body_add_shape, (const Object *) this, body, shape, transform, disabled);
}

void PhysicsServer::body_add_torque(const RID body, const Vector3 torque) {
	___godot_icall_void_RID_Vector3(___mb.mb_body_add_torque, (const Object *) this, body, torque);
}

void PhysicsServer::body_apply_central_impulse(const RID body, const Vector3 impulse) {
	___godot_icall_void_RID_Vector3(___mb.mb_body_apply_central_impulse, (const Object *) this, body, impulse);
}

void PhysicsServer::body_apply_impulse(const RID body, const Vector3 position, const Vector3 impulse) {
	___godot_icall_void_RID_Vector3_Vector3(___mb.mb_body_apply_impulse, (const Object *) this, body, position, impulse);
}

void PhysicsServer::body_apply_torque_impulse(const RID body, const Vector3 impulse) {
	___godot_icall_void_RID_Vector3(___mb.mb_body_apply_torque_impulse, (const Object *) this, body, impulse);
}

void PhysicsServer::body_attach_object_instance_id(const RID body, const int64_t id) {
	___godot_icall_void_RID_int(___mb.mb_body_attach_object_instance_id, (const Object *) this, body, id);
}

void PhysicsServer::body_clear_shapes(const RID body) {
	___godot_icall_void_RID(___mb.mb_body_clear_shapes, (const Object *) this, body);
}

RID PhysicsServer::body_create(const int64_t mode, const bool init_sleeping) {
	return ___godot_icall_RID_int_bool(___mb.mb_body_create, (const Object *) this, mode, init_sleeping);
}

int64_t PhysicsServer::body_get_collision_layer(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_collision_layer, (const Object *) this, body);
}

int64_t PhysicsServer::body_get_collision_mask(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_collision_mask, (const Object *) this, body);
}

PhysicsDirectBodyState *PhysicsServer::body_get_direct_state(const RID body) {
	return (PhysicsDirectBodyState *) ___godot_icall_Object_RID(___mb.mb_body_get_direct_state, (const Object *) this, body);
}

real_t PhysicsServer::body_get_kinematic_safe_margin(const RID body) const {
	return ___godot_icall_float_RID(___mb.mb_body_get_kinematic_safe_margin, (const Object *) this, body);
}

int64_t PhysicsServer::body_get_max_contacts_reported(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_max_contacts_reported, (const Object *) this, body);
}

PhysicsServer::BodyMode PhysicsServer::body_get_mode(const RID body) const {
	return (PhysicsServer::BodyMode) ___godot_icall_int_RID(___mb.mb_body_get_mode, (const Object *) this, body);
}

int64_t PhysicsServer::body_get_object_instance_id(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_object_instance_id, (const Object *) this, body);
}

real_t PhysicsServer::body_get_param(const RID body, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_body_get_param, (const Object *) this, body, param);
}

RID PhysicsServer::body_get_shape(const RID body, const int64_t shape_idx) const {
	return ___godot_icall_RID_RID_int(___mb.mb_body_get_shape, (const Object *) this, body, shape_idx);
}

int64_t PhysicsServer::body_get_shape_count(const RID body) const {
	return ___godot_icall_int_RID(___mb.mb_body_get_shape_count, (const Object *) this, body);
}

Transform PhysicsServer::body_get_shape_transform(const RID body, const int64_t shape_idx) const {
	return ___godot_icall_Transform_RID_int(___mb.mb_body_get_shape_transform, (const Object *) this, body, shape_idx);
}

RID PhysicsServer::body_get_space(const RID body) const {
	return ___godot_icall_RID_RID(___mb.mb_body_get_space, (const Object *) this, body);
}

Variant PhysicsServer::body_get_state(const RID body, const int64_t state) const {
	return ___godot_icall_Variant_RID_int(___mb.mb_body_get_state, (const Object *) this, body, state);
}

bool PhysicsServer::body_is_axis_locked(const RID body, const int64_t axis) const {
	return ___godot_icall_bool_RID_int(___mb.mb_body_is_axis_locked, (const Object *) this, body, axis);
}

bool PhysicsServer::body_is_continuous_collision_detection_enabled(const RID body) const {
	return ___godot_icall_bool_RID(___mb.mb_body_is_continuous_collision_detection_enabled, (const Object *) this, body);
}

bool PhysicsServer::body_is_omitting_force_integration(const RID body) const {
	return ___godot_icall_bool_RID(___mb.mb_body_is_omitting_force_integration, (const Object *) this, body);
}

bool PhysicsServer::body_is_ray_pickable(const RID body) const {
	return ___godot_icall_bool_RID(___mb.mb_body_is_ray_pickable, (const Object *) this, body);
}

void PhysicsServer::body_remove_collision_exception(const RID body, const RID excepted_body) {
	___godot_icall_void_RID_RID(___mb.mb_body_remove_collision_exception, (const Object *) this, body, excepted_body);
}

void PhysicsServer::body_remove_shape(const RID body, const int64_t shape_idx) {
	___godot_icall_void_RID_int(___mb.mb_body_remove_shape, (const Object *) this, body, shape_idx);
}

void PhysicsServer::body_set_axis_lock(const RID body, const int64_t axis, const bool lock) {
	___godot_icall_void_RID_int_bool(___mb.mb_body_set_axis_lock, (const Object *) this, body, axis, lock);
}

void PhysicsServer::body_set_axis_velocity(const RID body, const Vector3 axis_velocity) {
	___godot_icall_void_RID_Vector3(___mb.mb_body_set_axis_velocity, (const Object *) this, body, axis_velocity);
}

void PhysicsServer::body_set_collision_layer(const RID body, const int64_t layer) {
	___godot_icall_void_RID_int(___mb.mb_body_set_collision_layer, (const Object *) this, body, layer);
}

void PhysicsServer::body_set_collision_mask(const RID body, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_body_set_collision_mask, (const Object *) this, body, mask);
}

void PhysicsServer::body_set_enable_continuous_collision_detection(const RID body, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_body_set_enable_continuous_collision_detection, (const Object *) this, body, enable);
}

void PhysicsServer::body_set_force_integration_callback(const RID body, const Object *receiver, const String method, const Variant userdata) {
	___godot_icall_void_RID_Object_String_Variant(___mb.mb_body_set_force_integration_callback, (const Object *) this, body, receiver, method, userdata);
}

void PhysicsServer::body_set_kinematic_safe_margin(const RID body, const real_t margin) {
	___godot_icall_void_RID_float(___mb.mb_body_set_kinematic_safe_margin, (const Object *) this, body, margin);
}

void PhysicsServer::body_set_max_contacts_reported(const RID body, const int64_t amount) {
	___godot_icall_void_RID_int(___mb.mb_body_set_max_contacts_reported, (const Object *) this, body, amount);
}

void PhysicsServer::body_set_mode(const RID body, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_body_set_mode, (const Object *) this, body, mode);
}

void PhysicsServer::body_set_omit_force_integration(const RID body, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_body_set_omit_force_integration, (const Object *) this, body, enable);
}

void PhysicsServer::body_set_param(const RID body, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_body_set_param, (const Object *) this, body, param, value);
}

void PhysicsServer::body_set_ray_pickable(const RID body, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_body_set_ray_pickable, (const Object *) this, body, enable);
}

void PhysicsServer::body_set_shape(const RID body, const int64_t shape_idx, const RID shape) {
	___godot_icall_void_RID_int_RID(___mb.mb_body_set_shape, (const Object *) this, body, shape_idx, shape);
}

void PhysicsServer::body_set_shape_disabled(const RID body, const int64_t shape_idx, const bool disabled) {
	___godot_icall_void_RID_int_bool(___mb.mb_body_set_shape_disabled, (const Object *) this, body, shape_idx, disabled);
}

void PhysicsServer::body_set_shape_transform(const RID body, const int64_t shape_idx, const Transform transform) {
	___godot_icall_void_RID_int_Transform(___mb.mb_body_set_shape_transform, (const Object *) this, body, shape_idx, transform);
}

void PhysicsServer::body_set_space(const RID body, const RID space) {
	___godot_icall_void_RID_RID(___mb.mb_body_set_space, (const Object *) this, body, space);
}

void PhysicsServer::body_set_state(const RID body, const int64_t state, const Variant value) {
	___godot_icall_void_RID_int_Variant(___mb.mb_body_set_state, (const Object *) this, body, state, value);
}

real_t PhysicsServer::cone_twist_joint_get_param(const RID joint, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_cone_twist_joint_get_param, (const Object *) this, joint, param);
}

void PhysicsServer::cone_twist_joint_set_param(const RID joint, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_cone_twist_joint_set_param, (const Object *) this, joint, param, value);
}

void PhysicsServer::free_rid(const RID rid) {
	___godot_icall_void_RID(___mb.mb_free_rid, (const Object *) this, rid);
}

bool PhysicsServer::generic_6dof_joint_get_flag(const RID joint, const int64_t axis, const int64_t flag) {
	return ___godot_icall_bool_RID_int_int(___mb.mb_generic_6dof_joint_get_flag, (const Object *) this, joint, axis, flag);
}

real_t PhysicsServer::generic_6dof_joint_get_param(const RID joint, const int64_t axis, const int64_t param) {
	return ___godot_icall_float_RID_int_int(___mb.mb_generic_6dof_joint_get_param, (const Object *) this, joint, axis, param);
}

void PhysicsServer::generic_6dof_joint_set_flag(const RID joint, const int64_t axis, const int64_t flag, const bool enable) {
	___godot_icall_void_RID_int_int_bool(___mb.mb_generic_6dof_joint_set_flag, (const Object *) this, joint, axis, flag, enable);
}

void PhysicsServer::generic_6dof_joint_set_param(const RID joint, const int64_t axis, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_int_float(___mb.mb_generic_6dof_joint_set_param, (const Object *) this, joint, axis, param, value);
}

int64_t PhysicsServer::get_process_info(const int64_t process_info) {
	return ___godot_icall_int_int(___mb.mb_get_process_info, (const Object *) this, process_info);
}

bool PhysicsServer::hinge_joint_get_flag(const RID joint, const int64_t flag) const {
	return ___godot_icall_bool_RID_int(___mb.mb_hinge_joint_get_flag, (const Object *) this, joint, flag);
}

real_t PhysicsServer::hinge_joint_get_param(const RID joint, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_hinge_joint_get_param, (const Object *) this, joint, param);
}

void PhysicsServer::hinge_joint_set_flag(const RID joint, const int64_t flag, const bool enabled) {
	___godot_icall_void_RID_int_bool(___mb.mb_hinge_joint_set_flag, (const Object *) this, joint, flag, enabled);
}

void PhysicsServer::hinge_joint_set_param(const RID joint, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_hinge_joint_set_param, (const Object *) this, joint, param, value);
}

RID PhysicsServer::joint_create_cone_twist(const RID body_A, const Transform local_ref_A, const RID body_B, const Transform local_ref_B) {
	return ___godot_icall_RID_RID_Transform_RID_Transform(___mb.mb_joint_create_cone_twist, (const Object *) this, body_A, local_ref_A, body_B, local_ref_B);
}

RID PhysicsServer::joint_create_generic_6dof(const RID body_A, const Transform local_ref_A, const RID body_B, const Transform local_ref_B) {
	return ___godot_icall_RID_RID_Transform_RID_Transform(___mb.mb_joint_create_generic_6dof, (const Object *) this, body_A, local_ref_A, body_B, local_ref_B);
}

RID PhysicsServer::joint_create_hinge(const RID body_A, const Transform hinge_A, const RID body_B, const Transform hinge_B) {
	return ___godot_icall_RID_RID_Transform_RID_Transform(___mb.mb_joint_create_hinge, (const Object *) this, body_A, hinge_A, body_B, hinge_B);
}

RID PhysicsServer::joint_create_pin(const RID body_A, const Vector3 local_A, const RID body_B, const Vector3 local_B) {
	return ___godot_icall_RID_RID_Vector3_RID_Vector3(___mb.mb_joint_create_pin, (const Object *) this, body_A, local_A, body_B, local_B);
}

RID PhysicsServer::joint_create_slider(const RID body_A, const Transform local_ref_A, const RID body_B, const Transform local_ref_B) {
	return ___godot_icall_RID_RID_Transform_RID_Transform(___mb.mb_joint_create_slider, (const Object *) this, body_A, local_ref_A, body_B, local_ref_B);
}

int64_t PhysicsServer::joint_get_solver_priority(const RID joint) const {
	return ___godot_icall_int_RID(___mb.mb_joint_get_solver_priority, (const Object *) this, joint);
}

PhysicsServer::JointType PhysicsServer::joint_get_type(const RID joint) const {
	return (PhysicsServer::JointType) ___godot_icall_int_RID(___mb.mb_joint_get_type, (const Object *) this, joint);
}

void PhysicsServer::joint_set_solver_priority(const RID joint, const int64_t priority) {
	___godot_icall_void_RID_int(___mb.mb_joint_set_solver_priority, (const Object *) this, joint, priority);
}

Vector3 PhysicsServer::pin_joint_get_local_a(const RID joint) const {
	return ___godot_icall_Vector3_RID(___mb.mb_pin_joint_get_local_a, (const Object *) this, joint);
}

Vector3 PhysicsServer::pin_joint_get_local_b(const RID joint) const {
	return ___godot_icall_Vector3_RID(___mb.mb_pin_joint_get_local_b, (const Object *) this, joint);
}

real_t PhysicsServer::pin_joint_get_param(const RID joint, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_pin_joint_get_param, (const Object *) this, joint, param);
}

void PhysicsServer::pin_joint_set_local_a(const RID joint, const Vector3 local_A) {
	___godot_icall_void_RID_Vector3(___mb.mb_pin_joint_set_local_a, (const Object *) this, joint, local_A);
}

void PhysicsServer::pin_joint_set_local_b(const RID joint, const Vector3 local_B) {
	___godot_icall_void_RID_Vector3(___mb.mb_pin_joint_set_local_b, (const Object *) this, joint, local_B);
}

void PhysicsServer::pin_joint_set_param(const RID joint, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_pin_joint_set_param, (const Object *) this, joint, param, value);
}

void PhysicsServer::set_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_active, (const Object *) this, active);
}

RID PhysicsServer::shape_create(const int64_t type) {
	return ___godot_icall_RID_int(___mb.mb_shape_create, (const Object *) this, type);
}

Variant PhysicsServer::shape_get_data(const RID shape) const {
	return ___godot_icall_Variant_RID(___mb.mb_shape_get_data, (const Object *) this, shape);
}

PhysicsServer::ShapeType PhysicsServer::shape_get_type(const RID shape) const {
	return (PhysicsServer::ShapeType) ___godot_icall_int_RID(___mb.mb_shape_get_type, (const Object *) this, shape);
}

void PhysicsServer::shape_set_data(const RID shape, const Variant data) {
	___godot_icall_void_RID_Variant(___mb.mb_shape_set_data, (const Object *) this, shape, data);
}

real_t PhysicsServer::slider_joint_get_param(const RID joint, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_slider_joint_get_param, (const Object *) this, joint, param);
}

void PhysicsServer::slider_joint_set_param(const RID joint, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_slider_joint_set_param, (const Object *) this, joint, param, value);
}

RID PhysicsServer::space_create() {
	return ___godot_icall_RID(___mb.mb_space_create, (const Object *) this);
}

PhysicsDirectSpaceState *PhysicsServer::space_get_direct_state(const RID space) {
	return (PhysicsDirectSpaceState *) ___godot_icall_Object_RID(___mb.mb_space_get_direct_state, (const Object *) this, space);
}

real_t PhysicsServer::space_get_param(const RID space, const int64_t param) const {
	return ___godot_icall_float_RID_int(___mb.mb_space_get_param, (const Object *) this, space, param);
}

bool PhysicsServer::space_is_active(const RID space) const {
	return ___godot_icall_bool_RID(___mb.mb_space_is_active, (const Object *) this, space);
}

void PhysicsServer::space_set_active(const RID space, const bool active) {
	___godot_icall_void_RID_bool(___mb.mb_space_set_active, (const Object *) this, space, active);
}

void PhysicsServer::space_set_param(const RID space, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_space_set_param, (const Object *) this, space, param, value);
}

}