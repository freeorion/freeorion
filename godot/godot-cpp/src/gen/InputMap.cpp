#include "InputMap.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


InputMap *InputMap::_singleton = NULL;


InputMap::InputMap() {
	_owner = godot::api->godot_global_get_singleton((char *) "InputMap");
}


InputMap::___method_bindings InputMap::___mb = {};

void InputMap::___init_method_bindings() {
	___mb.mb_action_add_event = godot::api->godot_method_bind_get_method("InputMap", "action_add_event");
	___mb.mb_action_erase_event = godot::api->godot_method_bind_get_method("InputMap", "action_erase_event");
	___mb.mb_action_erase_events = godot::api->godot_method_bind_get_method("InputMap", "action_erase_events");
	___mb.mb_action_has_event = godot::api->godot_method_bind_get_method("InputMap", "action_has_event");
	___mb.mb_action_set_deadzone = godot::api->godot_method_bind_get_method("InputMap", "action_set_deadzone");
	___mb.mb_add_action = godot::api->godot_method_bind_get_method("InputMap", "add_action");
	___mb.mb_erase_action = godot::api->godot_method_bind_get_method("InputMap", "erase_action");
	___mb.mb_event_is_action = godot::api->godot_method_bind_get_method("InputMap", "event_is_action");
	___mb.mb_get_action_list = godot::api->godot_method_bind_get_method("InputMap", "get_action_list");
	___mb.mb_get_actions = godot::api->godot_method_bind_get_method("InputMap", "get_actions");
	___mb.mb_has_action = godot::api->godot_method_bind_get_method("InputMap", "has_action");
	___mb.mb_load_from_globals = godot::api->godot_method_bind_get_method("InputMap", "load_from_globals");
}

void InputMap::action_add_event(const String action, const Ref<InputEvent> event) {
	___godot_icall_void_String_Object(___mb.mb_action_add_event, (const Object *) this, action, event.ptr());
}

void InputMap::action_erase_event(const String action, const Ref<InputEvent> event) {
	___godot_icall_void_String_Object(___mb.mb_action_erase_event, (const Object *) this, action, event.ptr());
}

void InputMap::action_erase_events(const String action) {
	___godot_icall_void_String(___mb.mb_action_erase_events, (const Object *) this, action);
}

bool InputMap::action_has_event(const String action, const Ref<InputEvent> event) {
	return ___godot_icall_bool_String_Object(___mb.mb_action_has_event, (const Object *) this, action, event.ptr());
}

void InputMap::action_set_deadzone(const String action, const real_t deadzone) {
	___godot_icall_void_String_float(___mb.mb_action_set_deadzone, (const Object *) this, action, deadzone);
}

void InputMap::add_action(const String action, const real_t deadzone) {
	___godot_icall_void_String_float(___mb.mb_add_action, (const Object *) this, action, deadzone);
}

void InputMap::erase_action(const String action) {
	___godot_icall_void_String(___mb.mb_erase_action, (const Object *) this, action);
}

bool InputMap::event_is_action(const Ref<InputEvent> event, const String action) const {
	return ___godot_icall_bool_Object_String(___mb.mb_event_is_action, (const Object *) this, event.ptr(), action);
}

Array InputMap::get_action_list(const String action) {
	return ___godot_icall_Array_String(___mb.mb_get_action_list, (const Object *) this, action);
}

Array InputMap::get_actions() {
	return ___godot_icall_Array(___mb.mb_get_actions, (const Object *) this);
}

bool InputMap::has_action(const String action) const {
	return ___godot_icall_bool_String(___mb.mb_has_action, (const Object *) this, action);
}

void InputMap::load_from_globals() {
	___godot_icall_void(___mb.mb_load_from_globals, (const Object *) this);
}

}