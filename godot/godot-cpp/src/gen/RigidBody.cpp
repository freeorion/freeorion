#include "RigidBody.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "PhysicsDirectBodyState.hpp"
#include "PhysicsMaterial.hpp"


namespace godot {


RigidBody::___method_bindings RigidBody::___mb = {};

void RigidBody::___init_method_bindings() {
	___mb.mb__body_enter_tree = godot::api->godot_method_bind_get_method("RigidBody", "_body_enter_tree");
	___mb.mb__body_exit_tree = godot::api->godot_method_bind_get_method("RigidBody", "_body_exit_tree");
	___mb.mb__direct_state_changed = godot::api->godot_method_bind_get_method("RigidBody", "_direct_state_changed");
	___mb.mb__integrate_forces = godot::api->godot_method_bind_get_method("RigidBody", "_integrate_forces");
	___mb.mb__reload_physics_characteristics = godot::api->godot_method_bind_get_method("RigidBody", "_reload_physics_characteristics");
	___mb.mb_add_central_force = godot::api->godot_method_bind_get_method("RigidBody", "add_central_force");
	___mb.mb_add_force = godot::api->godot_method_bind_get_method("RigidBody", "add_force");
	___mb.mb_add_torque = godot::api->godot_method_bind_get_method("RigidBody", "add_torque");
	___mb.mb_apply_central_impulse = godot::api->godot_method_bind_get_method("RigidBody", "apply_central_impulse");
	___mb.mb_apply_impulse = godot::api->godot_method_bind_get_method("RigidBody", "apply_impulse");
	___mb.mb_apply_torque_impulse = godot::api->godot_method_bind_get_method("RigidBody", "apply_torque_impulse");
	___mb.mb_get_angular_damp = godot::api->godot_method_bind_get_method("RigidBody", "get_angular_damp");
	___mb.mb_get_angular_velocity = godot::api->godot_method_bind_get_method("RigidBody", "get_angular_velocity");
	___mb.mb_get_axis_lock = godot::api->godot_method_bind_get_method("RigidBody", "get_axis_lock");
	___mb.mb_get_bounce = godot::api->godot_method_bind_get_method("RigidBody", "get_bounce");
	___mb.mb_get_colliding_bodies = godot::api->godot_method_bind_get_method("RigidBody", "get_colliding_bodies");
	___mb.mb_get_friction = godot::api->godot_method_bind_get_method("RigidBody", "get_friction");
	___mb.mb_get_gravity_scale = godot::api->godot_method_bind_get_method("RigidBody", "get_gravity_scale");
	___mb.mb_get_linear_damp = godot::api->godot_method_bind_get_method("RigidBody", "get_linear_damp");
	___mb.mb_get_linear_velocity = godot::api->godot_method_bind_get_method("RigidBody", "get_linear_velocity");
	___mb.mb_get_mass = godot::api->godot_method_bind_get_method("RigidBody", "get_mass");
	___mb.mb_get_max_contacts_reported = godot::api->godot_method_bind_get_method("RigidBody", "get_max_contacts_reported");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("RigidBody", "get_mode");
	___mb.mb_get_physics_material_override = godot::api->godot_method_bind_get_method("RigidBody", "get_physics_material_override");
	___mb.mb_get_weight = godot::api->godot_method_bind_get_method("RigidBody", "get_weight");
	___mb.mb_is_able_to_sleep = godot::api->godot_method_bind_get_method("RigidBody", "is_able_to_sleep");
	___mb.mb_is_contact_monitor_enabled = godot::api->godot_method_bind_get_method("RigidBody", "is_contact_monitor_enabled");
	___mb.mb_is_sleeping = godot::api->godot_method_bind_get_method("RigidBody", "is_sleeping");
	___mb.mb_is_using_continuous_collision_detection = godot::api->godot_method_bind_get_method("RigidBody", "is_using_continuous_collision_detection");
	___mb.mb_is_using_custom_integrator = godot::api->godot_method_bind_get_method("RigidBody", "is_using_custom_integrator");
	___mb.mb_set_angular_damp = godot::api->godot_method_bind_get_method("RigidBody", "set_angular_damp");
	___mb.mb_set_angular_velocity = godot::api->godot_method_bind_get_method("RigidBody", "set_angular_velocity");
	___mb.mb_set_axis_lock = godot::api->godot_method_bind_get_method("RigidBody", "set_axis_lock");
	___mb.mb_set_axis_velocity = godot::api->godot_method_bind_get_method("RigidBody", "set_axis_velocity");
	___mb.mb_set_bounce = godot::api->godot_method_bind_get_method("RigidBody", "set_bounce");
	___mb.mb_set_can_sleep = godot::api->godot_method_bind_get_method("RigidBody", "set_can_sleep");
	___mb.mb_set_contact_monitor = godot::api->godot_method_bind_get_method("RigidBody", "set_contact_monitor");
	___mb.mb_set_friction = godot::api->godot_method_bind_get_method("RigidBody", "set_friction");
	___mb.mb_set_gravity_scale = godot::api->godot_method_bind_get_method("RigidBody", "set_gravity_scale");
	___mb.mb_set_linear_damp = godot::api->godot_method_bind_get_method("RigidBody", "set_linear_damp");
	___mb.mb_set_linear_velocity = godot::api->godot_method_bind_get_method("RigidBody", "set_linear_velocity");
	___mb.mb_set_mass = godot::api->godot_method_bind_get_method("RigidBody", "set_mass");
	___mb.mb_set_max_contacts_reported = godot::api->godot_method_bind_get_method("RigidBody", "set_max_contacts_reported");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("RigidBody", "set_mode");
	___mb.mb_set_physics_material_override = godot::api->godot_method_bind_get_method("RigidBody", "set_physics_material_override");
	___mb.mb_set_sleeping = godot::api->godot_method_bind_get_method("RigidBody", "set_sleeping");
	___mb.mb_set_use_continuous_collision_detection = godot::api->godot_method_bind_get_method("RigidBody", "set_use_continuous_collision_detection");
	___mb.mb_set_use_custom_integrator = godot::api->godot_method_bind_get_method("RigidBody", "set_use_custom_integrator");
	___mb.mb_set_weight = godot::api->godot_method_bind_get_method("RigidBody", "set_weight");
}

RigidBody *RigidBody::_new()
{
	return (RigidBody *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RigidBody")());
}
void RigidBody::_body_enter_tree(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__body_enter_tree, (const Object *) this, arg0);
}

void RigidBody::_body_exit_tree(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__body_exit_tree, (const Object *) this, arg0);
}

void RigidBody::_direct_state_changed(const Object *arg0) {
	___godot_icall_void_Object(___mb.mb__direct_state_changed, (const Object *) this, arg0);
}

void RigidBody::_integrate_forces(const PhysicsDirectBodyState *state) {
	___godot_icall_void_Object(___mb.mb__integrate_forces, (const Object *) this, state);
}

void RigidBody::_reload_physics_characteristics() {
	___godot_icall_void(___mb.mb__reload_physics_characteristics, (const Object *) this);
}

void RigidBody::add_central_force(const Vector3 force) {
	___godot_icall_void_Vector3(___mb.mb_add_central_force, (const Object *) this, force);
}

void RigidBody::add_force(const Vector3 force, const Vector3 position) {
	___godot_icall_void_Vector3_Vector3(___mb.mb_add_force, (const Object *) this, force, position);
}

void RigidBody::add_torque(const Vector3 torque) {
	___godot_icall_void_Vector3(___mb.mb_add_torque, (const Object *) this, torque);
}

void RigidBody::apply_central_impulse(const Vector3 impulse) {
	___godot_icall_void_Vector3(___mb.mb_apply_central_impulse, (const Object *) this, impulse);
}

void RigidBody::apply_impulse(const Vector3 position, const Vector3 impulse) {
	___godot_icall_void_Vector3_Vector3(___mb.mb_apply_impulse, (const Object *) this, position, impulse);
}

void RigidBody::apply_torque_impulse(const Vector3 impulse) {
	___godot_icall_void_Vector3(___mb.mb_apply_torque_impulse, (const Object *) this, impulse);
}

real_t RigidBody::get_angular_damp() const {
	return ___godot_icall_float(___mb.mb_get_angular_damp, (const Object *) this);
}

Vector3 RigidBody::get_angular_velocity() const {
	return ___godot_icall_Vector3(___mb.mb_get_angular_velocity, (const Object *) this);
}

bool RigidBody::get_axis_lock(const int64_t axis) const {
	return ___godot_icall_bool_int(___mb.mb_get_axis_lock, (const Object *) this, axis);
}

real_t RigidBody::get_bounce() const {
	return ___godot_icall_float(___mb.mb_get_bounce, (const Object *) this);
}

Array RigidBody::get_colliding_bodies() const {
	return ___godot_icall_Array(___mb.mb_get_colliding_bodies, (const Object *) this);
}

real_t RigidBody::get_friction() const {
	return ___godot_icall_float(___mb.mb_get_friction, (const Object *) this);
}

real_t RigidBody::get_gravity_scale() const {
	return ___godot_icall_float(___mb.mb_get_gravity_scale, (const Object *) this);
}

real_t RigidBody::get_linear_damp() const {
	return ___godot_icall_float(___mb.mb_get_linear_damp, (const Object *) this);
}

Vector3 RigidBody::get_linear_velocity() const {
	return ___godot_icall_Vector3(___mb.mb_get_linear_velocity, (const Object *) this);
}

real_t RigidBody::get_mass() const {
	return ___godot_icall_float(___mb.mb_get_mass, (const Object *) this);
}

int64_t RigidBody::get_max_contacts_reported() const {
	return ___godot_icall_int(___mb.mb_get_max_contacts_reported, (const Object *) this);
}

RigidBody::Mode RigidBody::get_mode() const {
	return (RigidBody::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

Ref<PhysicsMaterial> RigidBody::get_physics_material_override() const {
	return Ref<PhysicsMaterial>::__internal_constructor(___godot_icall_Object(___mb.mb_get_physics_material_override, (const Object *) this));
}

real_t RigidBody::get_weight() const {
	return ___godot_icall_float(___mb.mb_get_weight, (const Object *) this);
}

bool RigidBody::is_able_to_sleep() const {
	return ___godot_icall_bool(___mb.mb_is_able_to_sleep, (const Object *) this);
}

bool RigidBody::is_contact_monitor_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_contact_monitor_enabled, (const Object *) this);
}

bool RigidBody::is_sleeping() const {
	return ___godot_icall_bool(___mb.mb_is_sleeping, (const Object *) this);
}

bool RigidBody::is_using_continuous_collision_detection() const {
	return ___godot_icall_bool(___mb.mb_is_using_continuous_collision_detection, (const Object *) this);
}

bool RigidBody::is_using_custom_integrator() {
	return ___godot_icall_bool(___mb.mb_is_using_custom_integrator, (const Object *) this);
}

void RigidBody::set_angular_damp(const real_t angular_damp) {
	___godot_icall_void_float(___mb.mb_set_angular_damp, (const Object *) this, angular_damp);
}

void RigidBody::set_angular_velocity(const Vector3 angular_velocity) {
	___godot_icall_void_Vector3(___mb.mb_set_angular_velocity, (const Object *) this, angular_velocity);
}

void RigidBody::set_axis_lock(const int64_t axis, const bool lock) {
	___godot_icall_void_int_bool(___mb.mb_set_axis_lock, (const Object *) this, axis, lock);
}

void RigidBody::set_axis_velocity(const Vector3 axis_velocity) {
	___godot_icall_void_Vector3(___mb.mb_set_axis_velocity, (const Object *) this, axis_velocity);
}

void RigidBody::set_bounce(const real_t bounce) {
	___godot_icall_void_float(___mb.mb_set_bounce, (const Object *) this, bounce);
}

void RigidBody::set_can_sleep(const bool able_to_sleep) {
	___godot_icall_void_bool(___mb.mb_set_can_sleep, (const Object *) this, able_to_sleep);
}

void RigidBody::set_contact_monitor(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_contact_monitor, (const Object *) this, enabled);
}

void RigidBody::set_friction(const real_t friction) {
	___godot_icall_void_float(___mb.mb_set_friction, (const Object *) this, friction);
}

void RigidBody::set_gravity_scale(const real_t gravity_scale) {
	___godot_icall_void_float(___mb.mb_set_gravity_scale, (const Object *) this, gravity_scale);
}

void RigidBody::set_linear_damp(const real_t linear_damp) {
	___godot_icall_void_float(___mb.mb_set_linear_damp, (const Object *) this, linear_damp);
}

void RigidBody::set_linear_velocity(const Vector3 linear_velocity) {
	___godot_icall_void_Vector3(___mb.mb_set_linear_velocity, (const Object *) this, linear_velocity);
}

void RigidBody::set_mass(const real_t mass) {
	___godot_icall_void_float(___mb.mb_set_mass, (const Object *) this, mass);
}

void RigidBody::set_max_contacts_reported(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_max_contacts_reported, (const Object *) this, amount);
}

void RigidBody::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void RigidBody::set_physics_material_override(const Ref<PhysicsMaterial> physics_material_override) {
	___godot_icall_void_Object(___mb.mb_set_physics_material_override, (const Object *) this, physics_material_override.ptr());
}

void RigidBody::set_sleeping(const bool sleeping) {
	___godot_icall_void_bool(___mb.mb_set_sleeping, (const Object *) this, sleeping);
}

void RigidBody::set_use_continuous_collision_detection(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_continuous_collision_detection, (const Object *) this, enable);
}

void RigidBody::set_use_custom_integrator(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_custom_integrator, (const Object *) this, enable);
}

void RigidBody::set_weight(const real_t weight) {
	___godot_icall_void_float(___mb.mb_set_weight, (const Object *) this, weight);
}

}