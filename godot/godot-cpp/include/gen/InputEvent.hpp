#ifndef GODOT_CPP_INPUTEVENT_HPP
#define GODOT_CPP_INPUTEVENT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class InputEvent;

class InputEvent : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_accumulate;
		godot_method_bind *mb_as_text;
		godot_method_bind *mb_get_action_strength;
		godot_method_bind *mb_get_device;
		godot_method_bind *mb_is_action;
		godot_method_bind *mb_is_action_pressed;
		godot_method_bind *mb_is_action_released;
		godot_method_bind *mb_is_action_type;
		godot_method_bind *mb_is_echo;
		godot_method_bind *mb_is_pressed;
		godot_method_bind *mb_set_device;
		godot_method_bind *mb_shortcut_match;
		godot_method_bind *mb_xformed_by;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEvent"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool accumulate(const Ref<InputEvent> with_event);
	String as_text() const;
	real_t get_action_strength(const String action) const;
	int64_t get_device() const;
	bool is_action(const String action) const;
	bool is_action_pressed(const String action, const bool allow_echo = false) const;
	bool is_action_released(const String action) const;
	bool is_action_type() const;
	bool is_echo() const;
	bool is_pressed() const;
	void set_device(const int64_t device);
	bool shortcut_match(const Ref<InputEvent> event) const;
	Ref<InputEvent> xformed_by(const Transform2D xform, const Vector2 local_ofs = Vector2(0, 0)) const;

};

}

#endif