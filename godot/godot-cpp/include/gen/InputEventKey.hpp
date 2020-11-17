#ifndef GODOT_CPP_INPUTEVENTKEY_HPP
#define GODOT_CPP_INPUTEVENTKEY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEventWithModifiers.hpp"
namespace godot {


class InputEventKey : public InputEventWithModifiers {
	struct ___method_bindings {
		godot_method_bind *mb_get_scancode;
		godot_method_bind *mb_get_scancode_with_modifiers;
		godot_method_bind *mb_get_unicode;
		godot_method_bind *mb_set_echo;
		godot_method_bind *mb_set_pressed;
		godot_method_bind *mb_set_scancode;
		godot_method_bind *mb_set_unicode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventKey"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventKey *_new();

	// methods
	int64_t get_scancode() const;
	int64_t get_scancode_with_modifiers() const;
	int64_t get_unicode() const;
	void set_echo(const bool echo);
	void set_pressed(const bool pressed);
	void set_scancode(const int64_t scancode);
	void set_unicode(const int64_t unicode);

};

}

#endif