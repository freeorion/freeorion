#include "PhysicalBone.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


PhysicalBone::___method_bindings PhysicalBone::___mb = {};

void PhysicalBone::___init_method_bindings() {
	___mb.mb__direct_state_changed = godot::api->godot_method_bind_get_method("PhysicalBone", "_direct_state_changed");
	___mb.mb_apply_central_impulse = godot::api->godot_method_bind_get_method("PhysicalBone", "apply_central_impulse");
	___mb.mb_apply_impulse = godot::api->godot_method_bind_get_method("PhysicalBone", "apply_impulse");
	___mb.mb_get_body_offset = godot::api->godot_method_bind_get_method("PhysicalBone", "get_body_offset");
	___mb.mb_get_bone_id = godot::api->godot_method_bind_get_method("PhysicalBone", "get_bone_id");
	___mb.mb_get_bounce = godot::api->godot_method_bind_get_method("PhysicalBone", "get_bounce");
	___mb.mb_get_friction = godot::api->godot_method_bind_get_method("PhysicalBone", "get_friction");
	___mb.mb_get_gravity_scale = godot::api->godot_method_bind_get_method("PhysicalBone", "get_gravity_scale");
	___mb.mb_get_joint_offset = godot::api->godot_method_bind_get_method("PhysicalBone", "get_joint_offset");
	___mb.mb_get_joint_type = godot::api->godot_method_bind_get_method("PhysicalBone", "get_joint_type");
	___mb.mb_get_mass = godot::api->godot_method_bind_get_method("PhysicalBone", "get_mass");
	___mb.mb_get_simulate_physics = godot::api->godot_method_bind_get_method("PhysicalBone", "get_simulate_physics");
	___mb.mb_get_weight = godot::api->godot_method_bind_get_method("PhysicalBone", "get_weight");
	___mb.mb_is_simulating_physics = godot::api->godot_method_bind_get_method("PhysicalBone", "is_simulating_physics");
	___mb.mb_is_static_body = godot::api->godot_method_bind_get_method("PhysicalBone", "is_static_body");
	___mb.mb_set_body_offset = godot::api->godot_method_bind_get_method("PhysicalBone", "set_body_offset");
	___mb.mb_set_bounce = godot::api->godot_method_bind_get_method("PhysicalBone", "set_bounce");
	___mb.mb_set_friction = godot::api->godot_method_bind_get_method("PhysicalBone", "set_friction");
	___mb.mb_set_gravity_scale = godot::api->godot_method_bind_get_method("PhysicalBone", "set_gravity_scale");
	___mb.mb_set_joint_offset = godot::api->godot_method_bind_get_method("PhysicalBone", "set_joint_offset");
	___mb.mb_set_joint_type = godot::api->godot_method_bind_get_method("PhysicalBone", "set_joint_type");
	___mb.mb_set_mass = godot::api->godot_method_bind_get_method("PhysicalBone", "set_mass");
	___mb.mb_set_weight = godot::api->godot_method_bind_get_method("PhysicalBone", "set_weight");
}

PhysicalBone *PhysicalBone::_new()
{
	return (PhysicalBone *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PhysicalBone")());
}
void PhysicalBone::_direct_state_changed(const Object *arg0) {
	___godot_icall_void_Object(___mb.mb__direct_state_changed, (const Object *) this, arg0);
}

void PhysicalBone::apply_central_impulse(const Vector3 impulse) {
	___godot_icall_void_Vector3(___mb.mb_apply_central_impulse, (const Object *) this, impulse);
}

void PhysicalBone::apply_impulse(const Vector3 position, const Vector3 impulse) {
	___godot_icall_void_Vector3_Vector3(___mb.mb_apply_impulse, (const Object *) this, position, impulse);
}

Transform PhysicalBone::get_body_offset() const {
	return ___godot_icall_Transform(___mb.mb_get_body_offset, (const Object *) this);
}

int64_t PhysicalBone::get_bone_id() const {
	return ___godot_icall_int(___mb.mb_get_bone_id, (const Object *) this);
}

real_t PhysicalBone::get_bounce() const {
	return ___godot_icall_float(___mb.mb_get_bounce, (const Object *) this);
}

real_t PhysicalBone::get_friction() const {
	return ___godot_icall_float(___mb.mb_get_friction, (const Object *) this);
}

real_t PhysicalBone::get_gravity_scale() const {
	return ___godot_icall_float(___mb.mb_get_gravity_scale, (const Object *) this);
}

Transform PhysicalBone::get_joint_offset() const {
	return ___godot_icall_Transform(___mb.mb_get_joint_offset, (const Object *) this);
}

PhysicalBone::JointType PhysicalBone::get_joint_type() const {
	return (PhysicalBone::JointType) ___godot_icall_int(___mb.mb_get_joint_type, (const Object *) this);
}

real_t PhysicalBone::get_mass() const {
	return ___godot_icall_float(___mb.mb_get_mass, (const Object *) this);
}

bool PhysicalBone::get_simulate_physics() {
	return ___godot_icall_bool(___mb.mb_get_simulate_physics, (const Object *) this);
}

real_t PhysicalBone::get_weight() const {
	return ___godot_icall_float(___mb.mb_get_weight, (const Object *) this);
}

bool PhysicalBone::is_simulating_physics() {
	return ___godot_icall_bool(___mb.mb_is_simulating_physics, (const Object *) this);
}

bool PhysicalBone::is_static_body() {
	return ___godot_icall_bool(___mb.mb_is_static_body, (const Object *) this);
}

void PhysicalBone::set_body_offset(const Transform offset) {
	___godot_icall_void_Transform(___mb.mb_set_body_offset, (const Object *) this, offset);
}

void PhysicalBone::set_bounce(const real_t bounce) {
	___godot_icall_void_float(___mb.mb_set_bounce, (const Object *) this, bounce);
}

void PhysicalBone::set_friction(const real_t friction) {
	___godot_icall_void_float(___mb.mb_set_friction, (const Object *) this, friction);
}

void PhysicalBone::set_gravity_scale(const real_t gravity_scale) {
	___godot_icall_void_float(___mb.mb_set_gravity_scale, (const Object *) this, gravity_scale);
}

void PhysicalBone::set_joint_offset(const Transform offset) {
	___godot_icall_void_Transform(___mb.mb_set_joint_offset, (const Object *) this, offset);
}

void PhysicalBone::set_joint_type(const int64_t joint_type) {
	___godot_icall_void_int(___mb.mb_set_joint_type, (const Object *) this, joint_type);
}

void PhysicalBone::set_mass(const real_t mass) {
	___godot_icall_void_float(___mb.mb_set_mass, (const Object *) this, mass);
}

void PhysicalBone::set_weight(const real_t weight) {
	___godot_icall_void_float(___mb.mb_set_weight, (const Object *) this, weight);
}

}