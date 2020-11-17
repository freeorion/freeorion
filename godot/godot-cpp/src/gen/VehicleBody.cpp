#include "VehicleBody.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VehicleBody::___method_bindings VehicleBody::___mb = {};

void VehicleBody::___init_method_bindings() {
	___mb.mb_get_brake = godot::api->godot_method_bind_get_method("VehicleBody", "get_brake");
	___mb.mb_get_engine_force = godot::api->godot_method_bind_get_method("VehicleBody", "get_engine_force");
	___mb.mb_get_steering = godot::api->godot_method_bind_get_method("VehicleBody", "get_steering");
	___mb.mb_set_brake = godot::api->godot_method_bind_get_method("VehicleBody", "set_brake");
	___mb.mb_set_engine_force = godot::api->godot_method_bind_get_method("VehicleBody", "set_engine_force");
	___mb.mb_set_steering = godot::api->godot_method_bind_get_method("VehicleBody", "set_steering");
}

VehicleBody *VehicleBody::_new()
{
	return (VehicleBody *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VehicleBody")());
}
real_t VehicleBody::get_brake() const {
	return ___godot_icall_float(___mb.mb_get_brake, (const Object *) this);
}

real_t VehicleBody::get_engine_force() const {
	return ___godot_icall_float(___mb.mb_get_engine_force, (const Object *) this);
}

real_t VehicleBody::get_steering() const {
	return ___godot_icall_float(___mb.mb_get_steering, (const Object *) this);
}

void VehicleBody::set_brake(const real_t brake) {
	___godot_icall_void_float(___mb.mb_set_brake, (const Object *) this, brake);
}

void VehicleBody::set_engine_force(const real_t engine_force) {
	___godot_icall_void_float(___mb.mb_set_engine_force, (const Object *) this, engine_force);
}

void VehicleBody::set_steering(const real_t steering) {
	___godot_icall_void_float(___mb.mb_set_steering, (const Object *) this, steering);
}

}