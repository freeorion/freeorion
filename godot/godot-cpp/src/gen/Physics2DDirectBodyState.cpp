#include "Physics2DDirectBodyState.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Physics2DDirectSpaceState.hpp"


namespace godot {


Physics2DDirectBodyState::___method_bindings Physics2DDirectBodyState::___mb = {};

void Physics2DDirectBodyState::___init_method_bindings() {
	___mb.mb_add_central_force = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "add_central_force");
	___mb.mb_add_force = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "add_force");
	___mb.mb_add_torque = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "add_torque");
	___mb.mb_apply_central_impulse = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "apply_central_impulse");
	___mb.mb_apply_impulse = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "apply_impulse");
	___mb.mb_apply_torque_impulse = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "apply_torque_impulse");
	___mb.mb_get_angular_velocity = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_angular_velocity");
	___mb.mb_get_contact_collider = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_collider");
	___mb.mb_get_contact_collider_id = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_collider_id");
	___mb.mb_get_contact_collider_object = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_collider_object");
	___mb.mb_get_contact_collider_position = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_collider_position");
	___mb.mb_get_contact_collider_shape = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_collider_shape");
	___mb.mb_get_contact_collider_shape_metadata = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_collider_shape_metadata");
	___mb.mb_get_contact_collider_velocity_at_position = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_collider_velocity_at_position");
	___mb.mb_get_contact_count = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_count");
	___mb.mb_get_contact_local_normal = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_local_normal");
	___mb.mb_get_contact_local_position = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_local_position");
	___mb.mb_get_contact_local_shape = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_contact_local_shape");
	___mb.mb_get_inverse_inertia = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_inverse_inertia");
	___mb.mb_get_inverse_mass = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_inverse_mass");
	___mb.mb_get_linear_velocity = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_linear_velocity");
	___mb.mb_get_space_state = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_space_state");
	___mb.mb_get_step = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_step");
	___mb.mb_get_total_angular_damp = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_total_angular_damp");
	___mb.mb_get_total_gravity = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_total_gravity");
	___mb.mb_get_total_linear_damp = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_total_linear_damp");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "get_transform");
	___mb.mb_integrate_forces = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "integrate_forces");
	___mb.mb_is_sleeping = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "is_sleeping");
	___mb.mb_set_angular_velocity = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "set_angular_velocity");
	___mb.mb_set_linear_velocity = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "set_linear_velocity");
	___mb.mb_set_sleep_state = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "set_sleep_state");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("Physics2DDirectBodyState", "set_transform");
}

void Physics2DDirectBodyState::add_central_force(const Vector2 force) {
	___godot_icall_void_Vector2(___mb.mb_add_central_force, (const Object *) this, force);
}

void Physics2DDirectBodyState::add_force(const Vector2 offset, const Vector2 force) {
	___godot_icall_void_Vector2_Vector2(___mb.mb_add_force, (const Object *) this, offset, force);
}

void Physics2DDirectBodyState::add_torque(const real_t torque) {
	___godot_icall_void_float(___mb.mb_add_torque, (const Object *) this, torque);
}

void Physics2DDirectBodyState::apply_central_impulse(const Vector2 impulse) {
	___godot_icall_void_Vector2(___mb.mb_apply_central_impulse, (const Object *) this, impulse);
}

void Physics2DDirectBodyState::apply_impulse(const Vector2 offset, const Vector2 impulse) {
	___godot_icall_void_Vector2_Vector2(___mb.mb_apply_impulse, (const Object *) this, offset, impulse);
}

void Physics2DDirectBodyState::apply_torque_impulse(const real_t impulse) {
	___godot_icall_void_float(___mb.mb_apply_torque_impulse, (const Object *) this, impulse);
}

real_t Physics2DDirectBodyState::get_angular_velocity() const {
	return ___godot_icall_float(___mb.mb_get_angular_velocity, (const Object *) this);
}

RID Physics2DDirectBodyState::get_contact_collider(const int64_t contact_idx) const {
	return ___godot_icall_RID_int(___mb.mb_get_contact_collider, (const Object *) this, contact_idx);
}

int64_t Physics2DDirectBodyState::get_contact_collider_id(const int64_t contact_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_contact_collider_id, (const Object *) this, contact_idx);
}

Object *Physics2DDirectBodyState::get_contact_collider_object(const int64_t contact_idx) const {
	return (Object *) ___godot_icall_Object_int(___mb.mb_get_contact_collider_object, (const Object *) this, contact_idx);
}

Vector2 Physics2DDirectBodyState::get_contact_collider_position(const int64_t contact_idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_contact_collider_position, (const Object *) this, contact_idx);
}

int64_t Physics2DDirectBodyState::get_contact_collider_shape(const int64_t contact_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_contact_collider_shape, (const Object *) this, contact_idx);
}

Variant Physics2DDirectBodyState::get_contact_collider_shape_metadata(const int64_t contact_idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_contact_collider_shape_metadata, (const Object *) this, contact_idx);
}

Vector2 Physics2DDirectBodyState::get_contact_collider_velocity_at_position(const int64_t contact_idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_contact_collider_velocity_at_position, (const Object *) this, contact_idx);
}

int64_t Physics2DDirectBodyState::get_contact_count() const {
	return ___godot_icall_int(___mb.mb_get_contact_count, (const Object *) this);
}

Vector2 Physics2DDirectBodyState::get_contact_local_normal(const int64_t contact_idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_contact_local_normal, (const Object *) this, contact_idx);
}

Vector2 Physics2DDirectBodyState::get_contact_local_position(const int64_t contact_idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_contact_local_position, (const Object *) this, contact_idx);
}

int64_t Physics2DDirectBodyState::get_contact_local_shape(const int64_t contact_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_contact_local_shape, (const Object *) this, contact_idx);
}

real_t Physics2DDirectBodyState::get_inverse_inertia() const {
	return ___godot_icall_float(___mb.mb_get_inverse_inertia, (const Object *) this);
}

real_t Physics2DDirectBodyState::get_inverse_mass() const {
	return ___godot_icall_float(___mb.mb_get_inverse_mass, (const Object *) this);
}

Vector2 Physics2DDirectBodyState::get_linear_velocity() const {
	return ___godot_icall_Vector2(___mb.mb_get_linear_velocity, (const Object *) this);
}

Physics2DDirectSpaceState *Physics2DDirectBodyState::get_space_state() {
	return (Physics2DDirectSpaceState *) ___godot_icall_Object(___mb.mb_get_space_state, (const Object *) this);
}

real_t Physics2DDirectBodyState::get_step() const {
	return ___godot_icall_float(___mb.mb_get_step, (const Object *) this);
}

real_t Physics2DDirectBodyState::get_total_angular_damp() const {
	return ___godot_icall_float(___mb.mb_get_total_angular_damp, (const Object *) this);
}

Vector2 Physics2DDirectBodyState::get_total_gravity() const {
	return ___godot_icall_Vector2(___mb.mb_get_total_gravity, (const Object *) this);
}

real_t Physics2DDirectBodyState::get_total_linear_damp() const {
	return ___godot_icall_float(___mb.mb_get_total_linear_damp, (const Object *) this);
}

Transform2D Physics2DDirectBodyState::get_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_transform, (const Object *) this);
}

void Physics2DDirectBodyState::integrate_forces() {
	___godot_icall_void(___mb.mb_integrate_forces, (const Object *) this);
}

bool Physics2DDirectBodyState::is_sleeping() const {
	return ___godot_icall_bool(___mb.mb_is_sleeping, (const Object *) this);
}

void Physics2DDirectBodyState::set_angular_velocity(const real_t velocity) {
	___godot_icall_void_float(___mb.mb_set_angular_velocity, (const Object *) this, velocity);
}

void Physics2DDirectBodyState::set_linear_velocity(const Vector2 velocity) {
	___godot_icall_void_Vector2(___mb.mb_set_linear_velocity, (const Object *) this, velocity);
}

void Physics2DDirectBodyState::set_sleep_state(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_sleep_state, (const Object *) this, enabled);
}

void Physics2DDirectBodyState::set_transform(const Transform2D transform) {
	___godot_icall_void_Transform2D(___mb.mb_set_transform, (const Object *) this, transform);
}

}