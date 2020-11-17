#ifndef GODOT_CPP_INPUTEVENTMOUSE_HPP
#define GODOT_CPP_INPUTEVENTMOUSE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEventWithModifiers.hpp"
namespace godot {


class InputEventMouse : public InputEventWithModifiers {
	struct ___method_bindings {
		godot_method_bind *mb_get_button_mask;
		godot_method_bind *mb_get_global_position;
		godot_method_bind *mb_get_position;
		godot_method_bind *mb_set_button_mask;
		godot_method_bind *mb_set_global_position;
		godot_method_bind *mb_set_position;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventMouse"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	int64_t get_button_mask() const;
	Vector2 get_global_position() const;
	Vector2 get_position() const;
	void set_button_mask(const int64_t button_mask);
	void set_global_position(const Vector2 global_position);
	void set_position(const Vector2 position);

};

}

#endif