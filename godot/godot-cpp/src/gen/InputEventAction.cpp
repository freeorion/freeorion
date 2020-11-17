#include "InputEventAction.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventAction::___method_bindings InputEventAction::___mb = {};

void InputEventAction::___init_method_bindings() {
	___mb.mb_get_action = godot::api->godot_method_bind_get_method("InputEventAction", "get_action");
	___mb.mb_get_strength = godot::api->godot_method_bind_get_method("InputEventAction", "get_strength");
	___mb.mb_set_action = godot::api->godot_method_bind_get_method("InputEventAction", "set_action");
	___mb.mb_set_pressed = godot::api->godot_method_bind_get_method("InputEventAction", "set_pressed");
	___mb.mb_set_strength = godot::api->godot_method_bind_get_method("InputEventAction", "set_strength");
}

InputEventAction *InputEventAction::_new()
{
	return (InputEventAction *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventAction")());
}
String InputEventAction::get_action() const {
	return ___godot_icall_String(___mb.mb_get_action, (const Object *) this);
}

real_t InputEventAction::get_strength() const {
	return ___godot_icall_float(___mb.mb_get_strength, (const Object *) this);
}

void InputEventAction::set_action(const String action) {
	___godot_icall_void_String(___mb.mb_set_action, (const Object *) this, action);
}

void InputEventAction::set_pressed(const bool pressed) {
	___godot_icall_void_bool(___mb.mb_set_pressed, (const Object *) this, pressed);
}

void InputEventAction::set_strength(const real_t strength) {
	___godot_icall_void_float(___mb.mb_set_strength, (const Object *) this, strength);
}

}