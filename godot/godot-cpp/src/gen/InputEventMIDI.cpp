#include "InputEventMIDI.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventMIDI::___method_bindings InputEventMIDI::___mb = {};

void InputEventMIDI::___init_method_bindings() {
	___mb.mb_get_channel = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_channel");
	___mb.mb_get_controller_number = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_controller_number");
	___mb.mb_get_controller_value = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_controller_value");
	___mb.mb_get_instrument = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_instrument");
	___mb.mb_get_message = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_message");
	___mb.mb_get_pitch = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_pitch");
	___mb.mb_get_pressure = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_pressure");
	___mb.mb_get_velocity = godot::api->godot_method_bind_get_method("InputEventMIDI", "get_velocity");
	___mb.mb_set_channel = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_channel");
	___mb.mb_set_controller_number = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_controller_number");
	___mb.mb_set_controller_value = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_controller_value");
	___mb.mb_set_instrument = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_instrument");
	___mb.mb_set_message = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_message");
	___mb.mb_set_pitch = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_pitch");
	___mb.mb_set_pressure = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_pressure");
	___mb.mb_set_velocity = godot::api->godot_method_bind_get_method("InputEventMIDI", "set_velocity");
}

InputEventMIDI *InputEventMIDI::_new()
{
	return (InputEventMIDI *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventMIDI")());
}
int64_t InputEventMIDI::get_channel() const {
	return ___godot_icall_int(___mb.mb_get_channel, (const Object *) this);
}

int64_t InputEventMIDI::get_controller_number() const {
	return ___godot_icall_int(___mb.mb_get_controller_number, (const Object *) this);
}

int64_t InputEventMIDI::get_controller_value() const {
	return ___godot_icall_int(___mb.mb_get_controller_value, (const Object *) this);
}

int64_t InputEventMIDI::get_instrument() const {
	return ___godot_icall_int(___mb.mb_get_instrument, (const Object *) this);
}

int64_t InputEventMIDI::get_message() const {
	return ___godot_icall_int(___mb.mb_get_message, (const Object *) this);
}

int64_t InputEventMIDI::get_pitch() const {
	return ___godot_icall_int(___mb.mb_get_pitch, (const Object *) this);
}

int64_t InputEventMIDI::get_pressure() const {
	return ___godot_icall_int(___mb.mb_get_pressure, (const Object *) this);
}

int64_t InputEventMIDI::get_velocity() const {
	return ___godot_icall_int(___mb.mb_get_velocity, (const Object *) this);
}

void InputEventMIDI::set_channel(const int64_t channel) {
	___godot_icall_void_int(___mb.mb_set_channel, (const Object *) this, channel);
}

void InputEventMIDI::set_controller_number(const int64_t controller_number) {
	___godot_icall_void_int(___mb.mb_set_controller_number, (const Object *) this, controller_number);
}

void InputEventMIDI::set_controller_value(const int64_t controller_value) {
	___godot_icall_void_int(___mb.mb_set_controller_value, (const Object *) this, controller_value);
}

void InputEventMIDI::set_instrument(const int64_t instrument) {
	___godot_icall_void_int(___mb.mb_set_instrument, (const Object *) this, instrument);
}

void InputEventMIDI::set_message(const int64_t message) {
	___godot_icall_void_int(___mb.mb_set_message, (const Object *) this, message);
}

void InputEventMIDI::set_pitch(const int64_t pitch) {
	___godot_icall_void_int(___mb.mb_set_pitch, (const Object *) this, pitch);
}

void InputEventMIDI::set_pressure(const int64_t pressure) {
	___godot_icall_void_int(___mb.mb_set_pressure, (const Object *) this, pressure);
}

void InputEventMIDI::set_velocity(const int64_t velocity) {
	___godot_icall_void_int(___mb.mb_set_velocity, (const Object *) this, velocity);
}

}