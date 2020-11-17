#include "InputEvent.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


InputEvent::___method_bindings InputEvent::___mb = {};

void InputEvent::___init_method_bindings() {
	___mb.mb_accumulate = godot::api->godot_method_bind_get_method("InputEvent", "accumulate");
	___mb.mb_as_text = godot::api->godot_method_bind_get_method("InputEvent", "as_text");
	___mb.mb_get_action_strength = godot::api->godot_method_bind_get_method("InputEvent", "get_action_strength");
	___mb.mb_get_device = godot::api->godot_method_bind_get_method("InputEvent", "get_device");
	___mb.mb_is_action = godot::api->godot_method_bind_get_method("InputEvent", "is_action");
	___mb.mb_is_action_pressed = godot::api->godot_method_bind_get_method("InputEvent", "is_action_pressed");
	___mb.mb_is_action_released = godot::api->godot_method_bind_get_method("InputEvent", "is_action_released");
	___mb.mb_is_action_type = godot::api->godot_method_bind_get_method("InputEvent", "is_action_type");
	___mb.mb_is_echo = godot::api->godot_method_bind_get_method("InputEvent", "is_echo");
	___mb.mb_is_pressed = godot::api->godot_method_bind_get_method("InputEvent", "is_pressed");
	___mb.mb_set_device = godot::api->godot_method_bind_get_method("InputEvent", "set_device");
	___mb.mb_shortcut_match = godot::api->godot_method_bind_get_method("InputEvent", "shortcut_match");
	___mb.mb_xformed_by = godot::api->godot_method_bind_get_method("InputEvent", "xformed_by");
}

bool InputEvent::accumulate(const Ref<InputEvent> with_event) {
	return ___godot_icall_bool_Object(___mb.mb_accumulate, (const Object *) this, with_event.ptr());
}

String InputEvent::as_text() const {
	return ___godot_icall_String(___mb.mb_as_text, (const Object *) this);
}

real_t InputEvent::get_action_strength(const String action) const {
	return ___godot_icall_float_String(___mb.mb_get_action_strength, (const Object *) this, action);
}

int64_t InputEvent::get_device() const {
	return ___godot_icall_int(___mb.mb_get_device, (const Object *) this);
}

bool InputEvent::is_action(const String action) const {
	return ___godot_icall_bool_String(___mb.mb_is_action, (const Object *) this, action);
}

bool InputEvent::is_action_pressed(const String action, const bool allow_echo) const {
	return ___godot_icall_bool_String_bool(___mb.mb_is_action_pressed, (const Object *) this, action, allow_echo);
}

bool InputEvent::is_action_released(const String action) const {
	return ___godot_icall_bool_String(___mb.mb_is_action_released, (const Object *) this, action);
}

bool InputEvent::is_action_type() const {
	return ___godot_icall_bool(___mb.mb_is_action_type, (const Object *) this);
}

bool InputEvent::is_echo() const {
	return ___godot_icall_bool(___mb.mb_is_echo, (const Object *) this);
}

bool InputEvent::is_pressed() const {
	return ___godot_icall_bool(___mb.mb_is_pressed, (const Object *) this);
}

void InputEvent::set_device(const int64_t device) {
	___godot_icall_void_int(___mb.mb_set_device, (const Object *) this, device);
}

bool InputEvent::shortcut_match(const Ref<InputEvent> event) const {
	return ___godot_icall_bool_Object(___mb.mb_shortcut_match, (const Object *) this, event.ptr());
}

Ref<InputEvent> InputEvent::xformed_by(const Transform2D xform, const Vector2 local_ofs) const {
	return Ref<InputEvent>::__internal_constructor(___godot_icall_Object_Transform2D_Vector2(___mb.mb_xformed_by, (const Object *) this, xform, local_ofs));
}

}