#include "PhysicsDirectBodyState.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "PhysicsDirectSpaceState.hpp"


namespace godot {


PhysicsDirectBodyState::___method_bindings PhysicsDirectBodyState::___mb = {};

void PhysicsDirectBodyState::___init_method_bindings() {
	___mb.mb_add_central_force = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "add_central_force");
	___mb.mb_add_force = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "add_force");
	___mb.mb_add_torque = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "add_torque");
	___mb.mb_apply_central_impulse = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "apply_central_impulse");
	___mb.mb_apply_impulse = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "apply_impulse");
	___mb.mb_apply_torque_impulse = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "apply_torque_impulse");
	___mb.mb_get_angular_velocity = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_angular_velocity");
	___mb.mb_get_center_of_mass = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_center_of_mass");
	___mb.mb_get_contact_collider = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_collider");
	___mb.mb_get_contact_collider_id = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_collider_id");
	___mb.mb_get_contact_collider_object = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_collider_object");
	___mb.mb_get_contact_collider_position = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_collider_position");
	___mb.mb_get_contact_collider_shape = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_collider_shape");
	___mb.mb_get_contact_collider_velocity_at_position = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_collider_velocity_at_position");
	___mb.mb_get_contact_count = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_count");
	___mb.mb_get_contact_impulse = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_impulse");
	___mb.mb_get_contact_local_normal = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_local_normal");
	___mb.mb_get_contact_local_position = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_local_position");
	___mb.mb_get_contact_local_shape = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_contact_local_shape");
	___mb.mb_get_inverse_inertia = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_inverse_inertia");
	___mb.mb_get_inverse_mass = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_inverse_mass");
	___mb.mb_get_linear_velocity = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_linear_velocity");
	___mb.mb_get_principal_inertia_axes = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_principal_inertia_axes");
	___mb.mb_get_space_state = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_space_state");
	___mb.mb_get_step = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_step");
	___mb.mb_get_total_angular_damp = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_total_angular_damp");
	___mb.mb_get_total_gravity = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_total_gravity");
	___mb.mb_get_total_linear_damp = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_total_linear_damp");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "get_transform");
	___mb.mb_integrate_forces = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "integrate_forces");
	___mb.mb_is_sleeping = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "is_sleeping");
	___mb.mb_set_angular_velocity = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "set_angular_velocity");
	___mb.mb_set_linear_velocity = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "set_linear_velocity");
	___mb.mb_set_sleep_state = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "set_sleep_state");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("PhysicsDirectBodyState", "set_transform");
}

void PhysicsDirectBodyState::add_central_force(const Vector3 force) {
	___godot_icall_void_Vector3(___mb.mb_add_central_force, (const Object *) this, force);
}

void PhysicsDirectBodyState::add_force(const Vector3 force, const Vector3 position) {
	___godot_icall_void_Vector3_Vector3(___mb.mb_add_force, (const Object *) this, force, position);
}

void PhysicsDirectBodyState::add_torque(const Vector3 torque) {
	___godot_icall_void_Vector3(___mb.mb_add_torque, (const Object *) this, torque);
}

void PhysicsDirectBodyState::apply_central_impulse(const Vector3 j) {
	___godot_icall_void_Vector3(___mb.mb_apply_central_impulse, (const Object *) this, j);
}

void PhysicsDirectBodyState::apply_impulse(const Vector3 position, const Vector3 j) {
	___godot_icall_void_Vector3_Vector3(___mb.mb_apply_impulse, (const Object *) this, position, j);
}

void PhysicsDirectBodyState::apply_torque_impulse(const Vector3 j) {
	___godot_icall_void_Vector3(___mb.mb_apply_torque_impulse, (const Object *) this, j);
}

Vector3 PhysicsDirectBodyState::get_angular_velocity() const {
	return ___godot_icall_Vector3(___mb.mb_get_angular_velocity, (const Object *) this);
}

Vector3 PhysicsDirectBodyState::get_center_of_mass() const {
	return ___godot_icall_Vector3(___mb.mb_get_center_of_mass, (const Object *) this);
}

RID PhysicsDirectBodyState::get_contact_collider(const int64_t contact_idx) const {
	return ___godot_icall_RID_int(___mb.mb_get_contact_collider, (const Object *) this, contact_idx);
}

int64_t PhysicsDirectBodyState::get_contact_collider_id(const int64_t contact_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_contact_collider_id, (const Object *) this, contact_idx);
}

Object *PhysicsDirectBodyState::get_contact_collider_object(const int64_t contact_idx) const {
	return (Object *) ___godot_icall_Object_int(___mb.mb_get_contact_collider_object, (const Object *) this, contact_idx);
}

Vector3 PhysicsDirectBodyState::get_contact_collider_position(const int64_t contact_idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_contact_collider_position, (const Object *) this, contact_idx);
}

int64_t PhysicsDirectBodyState::get_contact_collider_shape(const int64_t contact_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_contact_collider_shape, (const Object *) this, contact_idx);
}

Vector3 PhysicsDirectBodyState::get_contact_collider_velocity_at_position(const int64_t contact_idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_contact_collider_velocity_at_position, (const Object *) this, contact_idx);
}

int64_t PhysicsDirectBodyState::get_contact_count() const {
	return ___godot_icall_int(___mb.mb_get_contact_count, (const Object *) this);
}

real_t PhysicsDirectBodyState::get_contact_impulse(const int64_t contact_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_contact_impulse, (const Object *) this, contact_idx);
}

Vector3 PhysicsDirectBodyState::get_contact_local_normal(const int64_t contact_idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_contact_local_normal, (const Object *) this, contact_idx);
}

Vector3 PhysicsDirectBodyState::get_contact_local_position(const int64_t contact_idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_contact_local_position, (const Object *) this, contact_idx);
}

int64_t PhysicsDirectBodyState::get_contact_local_shape(const int64_t contact_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_contact_local_shape, (const Object *) this, contact_idx);
}

Vector3 PhysicsDirectBodyState::get_inverse_inertia() const {
	return ___godot_icall_Vector3(___mb.mb_get_inverse_inertia, (const Object *) this);
}

real_t PhysicsDirectBodyState::get_inverse_mass() const {
	return ___godot_icall_float(___mb.mb_get_inverse_mass, (const Object *) this);
}

Vector3 PhysicsDirectBodyState::get_linear_velocity() const {
	return ___godot_icall_Vector3(___mb.mb_get_linear_velocity, (const Object *) this);
}

Basis PhysicsDirectBodyState::get_principal_inertia_axes() const {
	return ___godot_icall_Basis(___mb.mb_get_principal_inertia_axes, (const Object *) this);
}

PhysicsDirectSpaceState *PhysicsDirectBodyState::get_space_state() {
	return (PhysicsDirectSpaceState *) ___godot_icall_Object(___mb.mb_get_space_state, (const Object *) this);
}

real_t PhysicsDirectBodyState::get_step() const {
	return ___godot_icall_float(___mb.mb_get_step, (const Object *) this);
}

real_t PhysicsDirectBodyState::get_total_angular_damp() const {
	return ___godot_icall_float(___mb.mb_get_total_angular_damp, (const Object *) this);
}

Vector3 PhysicsDirectBodyState::get_total_gravity() const {
	return ___godot_icall_Vector3(___mb.mb_get_total_gravity, (const Object *) this);
}

real_t PhysicsDirectBodyState::get_total_linear_damp() const {
	return ___godot_icall_float(___mb.mb_get_total_linear_damp, (const Object *) this);
}

Transform PhysicsDirectBodyState::get_transform() const {
	return ___godot_icall_Transform(___mb.mb_get_transform, (const Object *) this);
}

void PhysicsDirectBodyState::integrate_forces() {
	___godot_icall_void(___mb.mb_integrate_forces, (const Object *) this);
}

bool PhysicsDirectBodyState::is_sleeping() const {
	return ___godot_icall_bool(___mb.mb_is_sleeping, (const Object *) this);
}

void PhysicsDirectBodyState::set_angular_velocity(const Vector3 velocity) {
	___godot_icall_void_Vector3(___mb.mb_set_angular_velocity, (const Object *) this, velocity);
}

void PhysicsDirectBodyState::set_linear_velocity(const Vector3 velocity) {
	___godot_icall_void_Vector3(___mb.mb_set_linear_velocity, (const Object *) this, velocity);
}

void PhysicsDirectBodyState::set_sleep_state(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_sleep_state, (const Object *) this, enabled);
}

void PhysicsDirectBodyState::set_transform(const Transform transform) {
	___godot_icall_void_Transform(___mb.mb_set_transform, (const Object *) this, transform);
}

}