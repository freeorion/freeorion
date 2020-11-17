#ifndef GODOT_CPP_INPUTEVENTMIDI_HPP
#define GODOT_CPP_INPUTEVENTMIDI_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "InputEvent.hpp"
namespace godot {


class InputEventMIDI : public InputEvent {
	struct ___method_bindings {
		godot_method_bind *mb_get_channel;
		godot_method_bind *mb_get_controller_number;
		godot_method_bind *mb_get_controller_value;
		godot_method_bind *mb_get_instrument;
		godot_method_bind *mb_get_message;
		godot_method_bind *mb_get_pitch;
		godot_method_bind *mb_get_pressure;
		godot_method_bind *mb_get_velocity;
		godot_method_bind *mb_set_channel;
		godot_method_bind *mb_set_controller_number;
		godot_method_bind *mb_set_controller_value;
		godot_method_bind *mb_set_instrument;
		godot_method_bind *mb_set_message;
		godot_method_bind *mb_set_pitch;
		godot_method_bind *mb_set_pressure;
		godot_method_bind *mb_set_velocity;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InputEventMIDI"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static InputEventMIDI *_new();

	// methods
	int64_t get_channel() const;
	int64_t get_controller_number() const;
	int64_t get_controller_value() const;
	int64_t get_instrument() const;
	int64_t get_message() const;
	int64_t get_pitch() const;
	int64_t get_pressure() const;
	int64_t get_velocity() const;
	void set_channel(const int64_t channel);
	void set_controller_number(const int64_t controller_number);
	void set_controller_value(const int64_t controller_value);
	void set_instrument(const int64_t instrument);
	void set_message(const int64_t message);
	void set_pitch(const int64_t pitch);
	void set_pressure(const int64_t pressure);
	void set_velocity(const int64_t velocity);

};

}

#endif