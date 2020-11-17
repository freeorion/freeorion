#include "RigidBody2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Physics2DDirectBodyState.hpp"
#include "PhysicsMaterial.hpp"
#include "Physics2DTestMotionResult.hpp"


namespace godot {


RigidBody2D::___method_bindings RigidBody2D::___mb = {};

void RigidBody2D::___init_method_bindings() {
	___mb.mb__body_enter_tree = godot::api->godot_method_bind_get_method("RigidBody2D", "_body_enter_tree");
	___mb.mb__body_exit_tree = godot::api->godot_method_bind_get_method("RigidBody2D", "_body_exit_tree");
	___mb.mb__direct_state_changed = godot::api->godot_method_bind_get_method("RigidBody2D", "_direct_state_changed");
	___mb.mb__integrate_forces = godot::api->godot_method_bind_get_method("RigidBody2D", "_integrate_forces");
	___mb.mb__reload_physics_characteristics = godot::api->godot_method_bind_get_method("RigidBody2D", "_reload_physics_characteristics");
	___mb.mb_add_central_force = godot::api->godot_method_bind_get_method("RigidBody2D", "add_central_force");
	___mb.mb_add_force = godot::api->godot_method_bind_get_method("RigidBody2D", "add_force");
	___mb.mb_add_torque = godot::api->godot_method_bind_get_method("RigidBody2D", "add_torque");
	___mb.mb_apply_central_impulse = godot::api->godot_method_bind_get_method("RigidBody2D", "apply_central_impulse");
	___mb.mb_apply_impulse = godot::api->godot_method_bind_get_method("RigidBody2D", "apply_impulse");
	___mb.mb_apply_torque_impulse = godot::api->godot_method_bind_get_method("RigidBody2D", "apply_torque_impulse");
	___mb.mb_get_angular_damp = godot::api->godot_method_bind_get_method("RigidBody2D", "get_angular_damp");
	___mb.mb_get_angular_velocity = godot::api->godot_method_bind_get_method("RigidBody2D", "get_angular_velocity");
	___mb.mb_get_applied_force = godot::api->godot_method_bind_get_method("RigidBody2D", "get_applied_force");
	___mb.mb_get_applied_torque = godot::api->godot_method_bind_get_method("RigidBody2D", "get_applied_torque");
	___mb.mb_get_bounce = godot::api->godot_method_bind_get_method("RigidBody2D", "get_bounce");
	___mb.mb_get_colliding_bodies = godot::api->godot_method_bind_get_method("RigidBody2D", "get_colliding_bodies");
	___mb.mb_get_continuous_collision_detection_mode = godot::api->godot_method_bind_get_method("RigidBody2D", "get_continuous_collision_detection_mode");
	___mb.mb_get_friction = godot::api->godot_method_bind_get_method("RigidBody2D", "get_friction");
	___mb.mb_get_gravity_scale = godot::api->godot_method_bind_get_method("RigidBody2D", "get_gravity_scale");
	___mb.mb_get_inertia = godot::api->godot_method_bind_get_method("RigidBody2D", "get_inertia");
	___mb.mb_get_linear_damp = godot::api->godot_method_bind_get_method("RigidBody2D", "get_linear_damp");
	___mb.mb_get_linear_velocity = godot::api->godot_method_bind_get_method("RigidBody2D", "get_linear_velocity");
	___mb.mb_get_mass = godot::api->godot_method_bind_get_method("RigidBody2D", "get_mass");
	___mb.mb_get_max_contacts_reported = godot::api->godot_method_bind_get_method("RigidBody2D", "get_max_contacts_reported");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("RigidBody2D", "get_mode");
	___mb.mb_get_physics_material_override = godot::api->godot_method_bind_get_method("RigidBody2D", "get_physics_material_override");
	___mb.mb_get_weight = godot::api->godot_method_bind_get_method("RigidBody2D", "get_weight");
	___mb.mb_is_able_to_sleep = godot::api->godot_method_bind_get_method("RigidBody2D", "is_able_to_sleep");
	___mb.mb_is_contact_monitor_enabled = godot::api->godot_method_bind_get_method("RigidBody2D", "is_contact_monitor_enabled");
	___mb.mb_is_sleeping = godot::api->godot_method_bind_get_method("RigidBody2D", "is_sleeping");
	___mb.mb_is_using_custom_integrator = godot::api->godot_method_bind_get_method("RigidBody2D", "is_using_custom_integrator");
	___mb.mb_set_angular_damp = godot::api->godot_method_bind_get_method("RigidBody2D", "set_angular_damp");
	___mb.mb_set_angular_velocity = godot::api->godot_method_bind_get_method("RigidBody2D", "set_angular_velocity");
	___mb.mb_set_applied_force = godot::api->godot_method_bind_get_method("RigidBody2D", "set_applied_force");
	___mb.mb_set_applied_torque = godot::api->godot_method_bind_get_method("RigidBody2D", "set_applied_torque");
	___mb.mb_set_axis_velocity = godot::api->godot_method_bind_get_method("RigidBody2D", "set_axis_velocity");
	___mb.mb_set_bounce = godot::api->godot_method_bind_get_method("RigidBody2D", "set_bounce");
	___mb.mb_set_can_sleep = godot::api->godot_method_bind_get_method("RigidBody2D", "set_can_sleep");
	___mb.mb_set_contact_monitor = godot::api->godot_method_bind_get_method("RigidBody2D", "set_contact_monitor");
	___mb.mb_set_continuous_collision_detection_mode = godot::api->godot_method_bind_get_method("RigidBody2D", "set_continuous_collision_detection_mode");
	___mb.mb_set_friction = godot::api->godot_method_bind_get_method("RigidBody2D", "set_friction");
	___mb.mb_set_gravity_scale = godot::api->godot_method_bind_get_method("RigidBody2D", "set_gravity_scale");
	___mb.mb_set_inertia = godot::api->godot_method_bind_get_method("RigidBody2D", "set_inertia");
	___mb.mb_set_linear_damp = godot::api->godot_method_bind_get_method("RigidBody2D", "set_linear_damp");
	___mb.mb_set_linear_velocity = godot::api->godot_method_bind_get_method("RigidBody2D", "set_linear_velocity");
	___mb.mb_set_mass = godot::api->godot_method_bind_get_method("RigidBody2D", "set_mass");
	___mb.mb_set_max_contacts_reported = godot::api->godot_method_bind_get_method("RigidBody2D", "set_max_contacts_reported");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("RigidBody2D", "set_mode");
	___mb.mb_set_physics_material_override = godot::api->godot_method_bind_get_method("RigidBody2D", "set_physics_material_override");
	___mb.mb_set_sleeping = godot::api->godot_method_bind_get_method("RigidBody2D", "set_sleeping");
	___mb.mb_set_use_custom_integrator = godot::api->godot_method_bind_get_method("RigidBody2D", "set_use_custom_integrator");
	___mb.mb_set_weight = godot::api->godot_method_bind_get_method("RigidBody2D", "set_weight");
	___mb.mb_test_motion = godot::api->godot_method_bind_get_method("RigidBody2D", "test_motion");
}

RigidBody2D *RigidBody2D::_new()
{
	return (RigidBody2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RigidBody2D")());
}
void RigidBody2D::_body_enter_tree(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__body_enter_tree, (const Object *) this, arg0);
}

void RigidBody2D::_body_exit_tree(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__body_exit_tree, (const Object *) this, arg0);
}

void RigidBody2D::_direct_state_changed(const Object *arg0) {
	___godot_icall_void_Object(___mb.mb__direct_state_changed, (const Object *) this, arg0);
}

void RigidBody2D::_integrate_forces(const Physics2DDirectBodyState *state) {
	___godot_icall_void_Object(___mb.mb__integrate_forces, (const Object *) this, state);
}

void RigidBody2D::_reload_physics_characteristics() {
	___godot_icall_void(___mb.mb__reload_physics_characteristics, (const Object *) this);
}

void RigidBody2D::add_central_force(const Vector2 force) {
	___godot_icall_void_Vector2(___mb.mb_add_central_force, (const Object *) this, force);
}

void RigidBody2D::add_force(const Vector2 offset, const Vector2 force) {
	___godot_icall_void_Vector2_Vector2(___mb.mb_add_force, (const Object *) this, offset, force);
}

void RigidBody2D::add_torque(const real_t torque) {
	___godot_icall_void_float(___mb.mb_add_torque, (const Object *) this, torque);
}

void RigidBody2D::apply_central_impulse(const Vector2 impulse) {
	___godot_icall_void_Vector2(___mb.mb_apply_central_impulse, (const Object *) this, impulse);
}

void RigidBody2D::apply_impulse(const Vector2 offset, const Vector2 impulse) {
	___godot_icall_void_Vector2_Vector2(___mb.mb_apply_impulse, (const Object *) this, offset, impulse);
}

void RigidBody2D::apply_torque_impulse(const real_t torque) {
	___godot_icall_void_float(___mb.mb_apply_torque_impulse, (const Object *) this, torque);
}

real_t RigidBody2D::get_angular_damp() const {
	return ___godot_icall_float(___mb.mb_get_angular_damp, (const Object *) this);
}

real_t RigidBody2D::get_angular_velocity() const {
	return ___godot_icall_float(___mb.mb_get_angular_velocity, (const Object *) this);
}

Vector2 RigidBody2D::get_applied_force() const {
	return ___godot_icall_Vector2(___mb.mb_get_applied_force, (const Object *) this);
}

real_t RigidBody2D::get_applied_torque() const {
	return ___godot_icall_float(___mb.mb_get_applied_torque, (const Object *) this);
}

real_t RigidBody2D::get_bounce() const {
	return ___godot_icall_float(___mb.mb_get_bounce, (const Object *) this);
}

Array RigidBody2D::get_colliding_bodies() const {
	return ___godot_icall_Array(___mb.mb_get_colliding_bodies, (const Object *) this);
}

RigidBody2D::CCDMode RigidBody2D::get_continuous_collision_detection_mode() const {
	return (RigidBody2D::CCDMode) ___godot_icall_int(___mb.mb_get_continuous_collision_detection_mode, (const Object *) this);
}

real_t RigidBody2D::get_friction() const {
	return ___godot_icall_float(___mb.mb_get_friction, (const Object *) this);
}

real_t RigidBody2D::get_gravity_scale() const {
	return ___godot_icall_float(___mb.mb_get_gravity_scale, (const Object *) this);
}

real_t RigidBody2D::get_inertia() const {
	return ___godot_icall_float(___mb.mb_get_inertia, (const Object *) this);
}

real_t RigidBody2D::get_linear_damp() const {
	return ___godot_icall_float(___mb.mb_get_linear_damp, (const Object *) this);
}

Vector2 RigidBody2D::get_linear_velocity() const {
	return ___godot_icall_Vector2(___mb.mb_get_linear_velocity, (const Object *) this);
}

real_t RigidBody2D::get_mass() const {
	return ___godot_icall_float(___mb.mb_get_mass, (const Object *) this);
}

int64_t RigidBody2D::get_max_contacts_reported() const {
	return ___godot_icall_int(___mb.mb_get_max_contacts_reported, (const Object *) this);
}

RigidBody2D::Mode RigidBody2D::get_mode() const {
	return (RigidBody2D::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

Ref<PhysicsMaterial> RigidBody2D::get_physics_material_override() const {
	return Ref<PhysicsMaterial>::__internal_constructor(___godot_icall_Object(___mb.mb_get_physics_material_override, (const Object *) this));
}

real_t RigidBody2D::get_weight() const {
	return ___godot_icall_float(___mb.mb_get_weight, (const Object *) this);
}

bool RigidBody2D::is_able_to_sleep() const {
	return ___godot_icall_bool(___mb.mb_is_able_to_sleep, (const Object *) this);
}

bool RigidBody2D::is_contact_monitor_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_contact_monitor_enabled, (const Object *) this);
}

bool RigidBody2D::is_sleeping() const {
	return ___godot_icall_bool(___mb.mb_is_sleeping, (const Object *) this);
}

bool RigidBody2D::is_using_custom_integrator() {
	return ___godot_icall_bool(___mb.mb_is_using_custom_integrator, (const Object *) this);
}

void RigidBody2D::set_angular_damp(const real_t angular_damp) {
	___godot_icall_void_float(___mb.mb_set_angular_damp, (const Object *) this, angular_damp);
}

void RigidBody2D::set_angular_velocity(const real_t angular_velocity) {
	___godot_icall_void_float(___mb.mb_set_angular_velocity, (const Object *) this, angular_velocity);
}

void RigidBody2D::set_applied_force(const Vector2 force) {
	___godot_icall_void_Vector2(___mb.mb_set_applied_force, (const Object *) this, force);
}

void RigidBody2D::set_applied_torque(const real_t torque) {
	___godot_icall_void_float(___mb.mb_set_applied_torque, (const Object *) this, torque);
}

void RigidBody2D::set_axis_velocity(const Vector2 axis_velocity) {
	___godot_icall_void_Vector2(___mb.mb_set_axis_velocity, (const Object *) this, axis_velocity);
}

void RigidBody2D::set_bounce(const real_t bounce) {
	___godot_icall_void_float(___mb.mb_set_bounce, (const Object *) this, bounce);
}

void RigidBody2D::set_can_sleep(const bool able_to_sleep) {
	___godot_icall_void_bool(___mb.mb_set_can_sleep, (const Object *) this, able_to_sleep);
}

void RigidBody2D::set_contact_monitor(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_contact_monitor, (const Object *) this, enabled);
}

void RigidBody2D::set_continuous_collision_detection_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_continuous_collision_detection_mode, (const Object *) this, mode);
}

void RigidBody2D::set_friction(const real_t friction) {
	___godot_icall_void_float(___mb.mb_set_friction, (const Object *) this, friction);
}

void RigidBody2D::set_gravity_scale(const real_t gravity_scale) {
	___godot_icall_void_float(___mb.mb_set_gravity_scale, (const Object *) this, gravity_scale);
}

void RigidBody2D::set_inertia(const real_t inertia) {
	___godot_icall_void_float(___mb.mb_set_inertia, (const Object *) this, inertia);
}

void RigidBody2D::set_linear_damp(const real_t linear_damp) {
	___godot_icall_void_float(___mb.mb_set_linear_damp, (const Object *) this, linear_damp);
}

void RigidBody2D::set_linear_velocity(const Vector2 linear_velocity) {
	___godot_icall_void_Vector2(___mb.mb_set_linear_velocity, (const Object *) this, linear_velocity);
}

void RigidBody2D::set_mass(const real_t mass) {
	___godot_icall_void_float(___mb.mb_set_mass, (const Object *) this, mass);
}

void RigidBody2D::set_max_contacts_reported(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_max_contacts_reported, (const Object *) this, amount);
}

void RigidBody2D::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void RigidBody2D::set_physics_material_override(const Ref<PhysicsMaterial> physics_material_override) {
	___godot_icall_void_Object(___mb.mb_set_physics_material_override, (const Object *) this, physics_material_override.ptr());
}

void RigidBody2D::set_sleeping(const bool sleeping) {
	___godot_icall_void_bool(___mb.mb_set_sleeping, (const Object *) this, sleeping);
}

void RigidBody2D::set_use_custom_integrator(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_custom_integrator, (const Object *) this, enable);
}

void RigidBody2D::set_weight(const real_t weight) {
	___godot_icall_void_float(___mb.mb_set_weight, (const Object *) this, weight);
}

bool RigidBody2D::test_motion(const Vector2 motion, const bool infinite_inertia, const real_t margin, const Ref<Physics2DTestMotionResult> result) {
	return ___godot_icall_bool_Vector2_bool_float_Object(___mb.mb_test_motion, (const Object *) this, motion, infinite_inertia, margin, result.ptr());
}

}