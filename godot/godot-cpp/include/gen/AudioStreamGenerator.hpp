#ifndef GODOT_CPP_AUDIOSTREAMGENERATOR_HPP
#define GODOT_CPP_AUDIOSTREAMGENERATOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioStream.hpp"
namespace godot {


class AudioStreamGenerator : public AudioStream {
	struct ___method_bindings {
		godot_method_bind *mb_get_buffer_length;
		godot_method_bind *mb_get_mix_rate;
		godot_method_bind *mb_set_buffer_length;
		godot_method_bind *mb_set_mix_rate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioStreamGenerator"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioStreamGenerator *_new();

	// methods
	real_t get_buffer_length() const;
	real_t get_mix_rate() const;
	void set_buffer_length(const real_t seconds);
	void set_mix_rate(const real_t hz);

};

}

#endif