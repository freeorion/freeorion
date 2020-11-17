#ifndef GODOT_CPP_INPUTMAP_HPP
#define GODOT_CPP_INPUTMAP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class InputEvent;

class InputMap : public Object {
	static InputMap *_singleton;

	InputMap();

	struct ___method_bindings {
		godot_method_bind *mb_action_add_event;
		godot_method_bind *mb_action_erase_event;
		godot_method_bind *mb_action_erase_events;
		godot_method_bind *mb_action_has_event;
		godot_method_bind *mb_action_set_deadzone;
		godot_method_bind *mb_add_action;
		godot_method_bind *mb_erase_action;
		godot_method_bind *mb_event_is_action;
		godot_method_bind *mb_get_action_list;
		godot_method_bind *mb_get_actions;
		godot_method_bind *mb_has_action;
		godot_method_bind *mb_load_from_globals;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline InputMap *get_singleton()
	{
		if (!InputMap::_singleton) {
			InputMap::_singleton = new InputMap;
		}
		return InputMap::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "InputMap"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void action_add_event(const String action, const Ref<InputEvent> event);
	void action_erase_event(const String action, const Ref<InputEvent> event);
	void action_erase_events(const String action);
	bool action_has_event(const String action, const Ref<InputEvent> event);
	void action_set_deadzone(const String action, const real_t deadzone);
	void add_action(const String action, const real_t deadzone = 0.5);
	void erase_action(const String action);
	bool event_is_action(const Ref<InputEvent> event, const String action) const;
	Array get_action_list(const String action);
	Array get_actions();
	bool has_action(const String action) const;
	void load_from_globals();

};

}

#endif