#ifndef GODOT_CPP_INPUTEVENTMOUSEMOTION_HPP
#define GODOT_CPP_INPUTEVENTMOUSEMOTION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEventMouse.hpp"
namespace godot {


class InputEventMouseMotion : public InputEventMouse {
	struct ___method_bindings {
		godot_method_bind *mb_get_pressure;
		godot_method_bind *mb_get_relative;
		godot_method_bind *mb_get_speed;
		godot_method_bind *mb_get_tilt;
		godot_method_bind *mb_set_pressure;
		godot_method_bind *mb_set_relative;
		godot_method_bind *mb_set_speed;
		godot_method_bind *mb_set_tilt;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventMouseMotion"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventMouseMotion *_new();

	// methods
	real_t get_pressure() const;
	Vector2 get_relative() const;
	Vector2 get_speed() const;
	Vector2 get_tilt() const;
	void set_pressure(const real_t pressure);
	void set_relative(const Vector2 relative);
	void set_speed(const Vector2 speed);
	void set_tilt(const Vector2 tilt);

};

}

#endif