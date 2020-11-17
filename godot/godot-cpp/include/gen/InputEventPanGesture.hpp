#ifndef GODOT_CPP_INPUTEVENTPANGESTURE_HPP
#define GODOT_CPP_INPUTEVENTPANGESTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEventGesture.hpp"
namespace godot {


class InputEventPanGesture : public InputEventGesture {
	struct ___method_bindings {
		godot_method_bind *mb_get_delta;
		godot_method_bind *mb_set_delta;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventPanGesture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventPanGesture *_new();

	// methods
	Vector2 get_delta() const;
	void set_delta(const Vector2 delta);

};

}

#endif