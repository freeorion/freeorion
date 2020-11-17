#ifndef GODOT_CPP_INPUTEVENTMOUSEBUTTON_HPP
#define GODOT_CPP_INPUTEVENTMOUSEBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEventMouse.hpp"
namespace godot {


class InputEventMouseButton : public InputEventMouse {
	struct ___method_bindings {
		godot_method_bind *mb_get_button_index;
		godot_method_bind *mb_get_factor;
		godot_method_bind *mb_is_doubleclick;
		godot_method_bind *mb_set_button_index;
		godot_method_bind *mb_set_doubleclick;
		godot_method_bind *mb_set_factor;
		godot_method_bind *mb_set_pressed;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventMouseButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventMouseButton *_new();

	// methods
	int64_t get_button_index() const;
	real_t get_factor();
	bool is_doubleclick() const;
	void set_button_index(const int64_t button_index);
	void set_doubleclick(const bool doubleclick);
	void set_factor(const real_t factor);
	void set_pressed(const bool pressed);

};

}

#endif