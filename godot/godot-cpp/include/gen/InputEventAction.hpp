#ifndef GODOT_CPP_INPUTEVENTACTION_HPP
#define GODOT_CPP_INPUTEVENTACTION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEvent.hpp"
namespace godot {


class InputEventAction : public InputEvent {
	struct ___method_bindings {
		godot_method_bind *mb_get_action;
		godot_method_bind *mb_get_strength;
		godot_method_bind *mb_set_action;
		godot_method_bind *mb_set_pressed;
		godot_method_bind *mb_set_strength;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventAction"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventAction *_new();

	// methods
	String get_action() const;
	real_t get_strength() const;
	void set_action(const String action);
	void set_pressed(const bool pressed);
	void set_strength(const real_t strength);

};

}

#endif