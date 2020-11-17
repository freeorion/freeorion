#include "VehicleWheel.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VehicleWheel::___method_bindings VehicleWheel::___mb = {};

void VehicleWheel::___init_method_bindings() {
	___mb.mb_get_brake = godot::api->godot_method_bind_get_method("VehicleWheel", "get_brake");
	___mb.mb_get_damping_compression = godot::api->godot_method_bind_get_method("VehicleWheel", "get_damping_compression");
	___mb.mb_get_damping_relaxation = godot::api->godot_method_bind_get_method("VehicleWheel", "get_damping_relaxation");
	___mb.mb_get_engine_force = godot::api->godot_method_bind_get_method("VehicleWheel", "get_engine_force");
	___mb.mb_get_friction_slip = godot::api->godot_method_bind_get_method("VehicleWheel", "get_friction_slip");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("VehicleWheel", "get_radius");
	___mb.mb_get_roll_influence = godot::api->godot_method_bind_get_method("VehicleWheel", "get_roll_influence");
	___mb.mb_get_rpm = godot::api->godot_method_bind_get_method("VehicleWheel", "get_rpm");
	___mb.mb_get_skidinfo = godot::api->godot_method_bind_get_method("VehicleWheel", "get_skidinfo");
	___mb.mb_get_steering = godot::api->godot_method_bind_get_method("VehicleWheel", "get_steering");
	___mb.mb_get_suspension_max_force = godot::api->godot_method_bind_get_method("VehicleWheel", "get_suspension_max_force");
	___mb.mb_get_suspension_rest_length = godot::api->godot_method_bind_get_method("VehicleWheel", "get_suspension_rest_length");
	___mb.mb_get_suspension_stiffness = godot::api->godot_method_bind_get_method("VehicleWheel", "get_suspension_stiffness");
	___mb.mb_get_suspension_travel = godot::api->godot_method_bind_get_method("VehicleWheel", "get_suspension_travel");
	___mb.mb_is_in_contact = godot::api->godot_method_bind_get_method("VehicleWheel", "is_in_contact");
	___mb.mb_is_used_as_steering = godot::api->godot_method_bind_get_method("VehicleWheel", "is_used_as_steering");
	___mb.mb_is_used_as_traction = godot::api->godot_method_bind_get_method("VehicleWheel", "is_used_as_traction");
	___mb.mb_set_brake = godot::api->godot_method_bind_get_method("VehicleWheel", "set_brake");
	___mb.mb_set_damping_compression = godot::api->godot_method_bind_get_method("VehicleWheel", "set_damping_compression");
	___mb.mb_set_damping_relaxation = godot::api->godot_method_bind_get_method("VehicleWheel", "set_damping_relaxation");
	___mb.mb_set_engine_force = godot::api->godot_method_bind_get_method("VehicleWheel", "set_engine_force");
	___mb.mb_set_friction_slip = godot::api->godot_method_bind_get_method("VehicleWheel", "set_friction_slip");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("VehicleWheel", "set_radius");
	___mb.mb_set_roll_influence = godot::api->godot_method_bind_get_method("VehicleWheel", "set_roll_influence");
	___mb.mb_set_steering = godot::api->godot_method_bind_get_method("VehicleWheel", "set_steering");
	___mb.mb_set_suspension_max_force = godot::api->godot_method_bind_get_method("VehicleWheel", "set_suspension_max_force");
	___mb.mb_set_suspension_rest_length = godot::api->godot_method_bind_get_method("VehicleWheel", "set_suspension_rest_length");
	___mb.mb_set_suspension_stiffness = godot::api->godot_method_bind_get_method("VehicleWheel", "set_suspension_stiffness");
	___mb.mb_set_suspension_travel = godot::api->godot_method_bind_get_method("VehicleWheel", "set_suspension_travel");
	___mb.mb_set_use_as_steering = godot::api->godot_method_bind_get_method("VehicleWheel", "set_use_as_steering");
	___mb.mb_set_use_as_traction = godot::api->godot_method_bind_get_method("VehicleWheel", "set_use_as_traction");
}

VehicleWheel *VehicleWheel::_new()
{
	return (VehicleWheel *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VehicleWheel")());
}
real_t VehicleWheel::get_brake() const {
	return ___godot_icall_float(___mb.mb_get_brake, (const Object *) this);
}

real_t VehicleWheel::get_damping_compression() const {
	return ___godot_icall_float(___mb.mb_get_damping_compression, (const Object *) this);
}

real_t VehicleWheel::get_damping_relaxation() const {
	return ___godot_icall_float(___mb.mb_get_damping_relaxation, (const Object *) this);
}

real_t VehicleWheel::get_engine_force() const {
	return ___godot_icall_float(___mb.mb_get_engine_force, (const Object *) this);
}

real_t VehicleWheel::get_friction_slip() const {
	return ___godot_icall_float(___mb.mb_get_friction_slip, (const Object *) this);
}

real_t VehicleWheel::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

real_t VehicleWheel::get_roll_influence() const {
	return ___godot_icall_float(___mb.mb_get_roll_influence, (const Object *) this);
}

real_t VehicleWheel::get_rpm() const {
	return ___godot_icall_float(___mb.mb_get_rpm, (const Object *) this);
}

real_t VehicleWheel::get_skidinfo() const {
	return ___godot_icall_float(___mb.mb_get_skidinfo, (const Object *) this);
}

real_t VehicleWheel::get_steering() const {
	return ___godot_icall_float(___mb.mb_get_steering, (const Object *) this);
}

real_t VehicleWheel::get_suspension_max_force() const {
	return ___godot_icall_float(___mb.mb_get_suspension_max_force, (const Object *) this);
}

real_t VehicleWheel::get_suspension_rest_length() const {
	return ___godot_icall_float(___mb.mb_get_suspension_rest_length, (const Object *) this);
}

real_t VehicleWheel::get_suspension_stiffness() const {
	return ___godot_icall_float(___mb.mb_get_suspension_stiffness, (const Object *) this);
}

real_t VehicleWheel::get_suspension_travel() const {
	return ___godot_icall_float(___mb.mb_get_suspension_travel, (const Object *) this);
}

bool VehicleWheel::is_in_contact() const {
	return ___godot_icall_bool(___mb.mb_is_in_contact, (const Object *) this);
}

bool VehicleWheel::is_used_as_steering() const {
	return ___godot_icall_bool(___mb.mb_is_used_as_steering, (const Object *) this);
}

bool VehicleWheel::is_used_as_traction() const {
	return ___godot_icall_bool(___mb.mb_is_used_as_traction, (const Object *) this);
}

void VehicleWheel::set_brake(const real_t brake) {
	___godot_icall_void_float(___mb.mb_set_brake, (const Object *) this, brake);
}

void VehicleWheel::set_damping_compression(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_damping_compression, (const Object *) this, length);
}

void VehicleWheel::set_damping_relaxation(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_damping_relaxation, (const Object *) this, length);
}

void VehicleWheel::set_engine_force(const real_t engine_force) {
	___godot_icall_void_float(___mb.mb_set_engine_force, (const Object *) this, engine_force);
}

void VehicleWheel::set_friction_slip(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_friction_slip, (const Object *) this, length);
}

void VehicleWheel::set_radius(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, length);
}

void VehicleWheel::set_roll_influence(const real_t roll_influence) {
	___godot_icall_void_float(___mb.mb_set_roll_influence, (const Object *) this, roll_influence);
}

void VehicleWheel::set_steering(const real_t steering) {
	___godot_icall_void_float(___mb.mb_set_steering, (const Object *) this, steering);
}

void VehicleWheel::set_suspension_max_force(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_suspension_max_force, (const Object *) this, length);
}

void VehicleWheel::set_suspension_rest_length(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_suspension_rest_length, (const Object *) this, length);
}

void VehicleWheel::set_suspension_stiffness(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_suspension_stiffness, (const Object *) this, length);
}

void VehicleWheel::set_suspension_travel(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_suspension_travel, (const Object *) this, length);
}

void VehicleWheel::set_use_as_steering(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_as_steering, (const Object *) this, enable);
}

void VehicleWheel::set_use_as_traction(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_as_traction, (const Object *) this, enable);
}

}