#include "InputEventKey.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventKey::___method_bindings InputEventKey::___mb = {};

void InputEventKey::___init_method_bindings() {
	___mb.mb_get_scancode = godot::api->godot_method_bind_get_method("InputEventKey", "get_scancode");
	___mb.mb_get_scancode_with_modifiers = godot::api->godot_method_bind_get_method("InputEventKey", "get_scancode_with_modifiers");
	___mb.mb_get_unicode = godot::api->godot_method_bind_get_method("InputEventKey", "get_unicode");
	___mb.mb_set_echo = godot::api->godot_method_bind_get_method("InputEventKey", "set_echo");
	___mb.mb_set_pressed = godot::api->godot_method_bind_get_method("InputEventKey", "set_pressed");
	___mb.mb_set_scancode = godot::api->godot_method_bind_get_method("InputEventKey", "set_scancode");
	___mb.mb_set_unicode = godot::api->godot_method_bind_get_method("InputEventKey", "set_unicode");
}

InputEventKey *InputEventKey::_new()
{
	return (InputEventKey *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventKey")());
}
int64_t InputEventKey::get_scancode() const {
	return ___godot_icall_int(___mb.mb_get_scancode, (const Object *) this);
}

int64_t InputEventKey::get_scancode_with_modifiers() const {
	return ___godot_icall_int(___mb.mb_get_scancode_with_modifiers, (const Object *) this);
}

int64_t InputEventKey::get_unicode() const {
	return ___godot_icall_int(___mb.mb_get_unicode, (const Object *) this);
}

void InputEventKey::set_echo(const bool echo) {
	___godot_icall_void_bool(___mb.mb_set_echo, (const Object *) this, echo);
}

void InputEventKey::set_pressed(const bool pressed) {
	___godot_icall_void_bool(___mb.mb_set_pressed, (const Object *) this, pressed);
}

void InputEventKey::set_scancode(const int64_t scancode) {
	___godot_icall_void_int(___mb.mb_set_scancode, (const Object *) this, scancode);
}

void InputEventKey::set_unicode(const int64_t unicode) {
	___godot_icall_void_int(___mb.mb_set_unicode, (const Object *) this, unicode);
}

}