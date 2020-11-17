#ifndef GODOT_CPP_INPUTEVENTJOYPADBUTTON_HPP
#define GODOT_CPP_INPUTEVENTJOYPADBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEvent.hpp"
namespace godot {


class InputEventJoypadButton : public InputEvent {
	struct ___method_bindings {
		godot_method_bind *mb_get_button_index;
		godot_method_bind *mb_get_pressure;
		godot_method_bind *mb_set_button_index;
		godot_method_bind *mb_set_pressed;
		godot_method_bind *mb_set_pressure;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventJoypadButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventJoypadButton *_new();

	// methods
	int64_t get_button_index() const;
	real_t get_pressure() const;
	void set_button_index(const int64_t button_index);
	void set_pressed(const bool pressed);
	void set_pressure(const real_t pressure);

};

}

#endif