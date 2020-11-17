#include "InputEventWithModifiers.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventWithModifiers::___method_bindings InputEventWithModifiers::___mb = {};

void InputEventWithModifiers::___init_method_bindings() {
	___mb.mb_get_alt = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "get_alt");
	___mb.mb_get_command = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "get_command");
	___mb.mb_get_control = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "get_control");
	___mb.mb_get_metakey = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "get_metakey");
	___mb.mb_get_shift = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "get_shift");
	___mb.mb_set_alt = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "set_alt");
	___mb.mb_set_command = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "set_command");
	___mb.mb_set_control = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "set_control");
	___mb.mb_set_metakey = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "set_metakey");
	___mb.mb_set_shift = godot::api->godot_method_bind_get_method("InputEventWithModifiers", "set_shift");
}

bool InputEventWithModifiers::get_alt() const {
	return ___godot_icall_bool(___mb.mb_get_alt, (const Object *) this);
}

bool InputEventWithModifiers::get_command() const {
	return ___godot_icall_bool(___mb.mb_get_command, (const Object *) this);
}

bool InputEventWithModifiers::get_control() const {
	return ___godot_icall_bool(___mb.mb_get_control, (const Object *) this);
}

bool InputEventWithModifiers::get_metakey() const {
	return ___godot_icall_bool(___mb.mb_get_metakey, (const Object *) this);
}

bool InputEventWithModifiers::get_shift() const {
	return ___godot_icall_bool(___mb.mb_get_shift, (const Object *) this);
}

void InputEventWithModifiers::set_alt(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_alt, (const Object *) this, enable);
}

void InputEventWithModifiers::set_command(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_command, (const Object *) this, enable);
}

void InputEventWithModifiers::set_control(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_control, (const Object *) this, enable);
}

void InputEventWithModifiers::set_metakey(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_metakey, (const Object *) this, enable);
}

void InputEventWithModifiers::set_shift(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_shift, (const Object *) this, enable);
}

}