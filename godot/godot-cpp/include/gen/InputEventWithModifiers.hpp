#ifndef GODOT_CPP_INPUTEVENTWITHMODIFIERS_HPP
#define GODOT_CPP_INPUTEVENTWITHMODIFIERS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEvent.hpp"
namespace godot {


class InputEventWithModifiers : public InputEvent {
	struct ___method_bindings {
		godot_method_bind *mb_get_alt;
		godot_method_bind *mb_get_command;
		godot_method_bind *mb_get_control;
		godot_method_bind *mb_get_metakey;
		godot_method_bind *mb_get_shift;
		godot_method_bind *mb_set_alt;
		godot_method_bind *mb_set_command;
		godot_method_bind *mb_set_control;
		godot_method_bind *mb_set_metakey;
		godot_method_bind *mb_set_shift;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventWithModifiers"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool get_alt() const;
	bool get_command() const;
	bool get_control() const;
	bool get_metakey() const;
	bool get_shift() const;
	void set_alt(const bool enable);
	void set_command(const bool enable);
	void set_control(const bool enable);
	void set_metakey(const bool enable);
	void set_shift(const bool enable);

};

}

#endif