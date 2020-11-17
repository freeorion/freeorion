#ifndef GODOT_CPP_INPUTEVENTJOYPADMOTION_HPP
#define GODOT_CPP_INPUTEVENTJOYPADMOTION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEvent.hpp"
namespace godot {


class InputEventJoypadMotion : public InputEvent {
	struct ___method_bindings {
		godot_method_bind *mb_get_axis;
		godot_method_bind *mb_get_axis_value;
		godot_method_bind *mb_set_axis;
		godot_method_bind *mb_set_axis_value;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventJoypadMotion"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventJoypadMotion *_new();

	// methods
	int64_t get_axis() const;
	real_t get_axis_value() const;
	void set_axis(const int64_t axis);
	void set_axis_value(const real_t axis_value);

};

}

#endif