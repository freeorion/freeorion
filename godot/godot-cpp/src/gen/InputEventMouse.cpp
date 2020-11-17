#include "InputEventMouse.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventMouse::___method_bindings InputEventMouse::___mb = {};

void InputEventMouse::___init_method_bindings() {
	___mb.mb_get_button_mask = godot::api->godot_method_bind_get_method("InputEventMouse", "get_button_mask");
	___mb.mb_get_global_position = godot::api->godot_method_bind_get_method("InputEventMouse", "get_global_position");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("InputEventMouse", "get_position");
	___mb.mb_set_button_mask = godot::api->godot_method_bind_get_method("InputEventMouse", "set_button_mask");
	___mb.mb_set_global_position = godot::api->godot_method_bind_get_method("InputEventMouse", "set_global_position");
	___mb.mb_set_position = godot::api->godot_method_bind_get_method("InputEventMouse", "set_position");
}

int64_t InputEventMouse::get_button_mask() const {
	return ___godot_icall_int(___mb.mb_get_button_mask, (const Object *) this);
}

Vector2 InputEventMouse::get_global_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_global_position, (const Object *) this);
}

Vector2 InputEventMouse::get_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_position, (const Object *) this);
}

void InputEventMouse::set_button_mask(const int64_t button_mask) {
	___godot_icall_void_int(___mb.mb_set_button_mask, (const Object *) this, button_mask);
}

void InputEventMouse::set_global_position(const Vector2 global_position) {
	___godot_icall_void_Vector2(___mb.mb_set_global_position, (const Object *) this, global_position);
}

void InputEventMouse::set_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_position, (const Object *) this, position);
}

}