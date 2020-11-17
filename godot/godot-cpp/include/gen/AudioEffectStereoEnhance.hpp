#ifndef GODOT_CPP_AUDIOEFFECTSTEREOENHANCE_HPP
#define GODOT_CPP_AUDIOEFFECTSTEREOENHANCE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectStereoEnhance : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_pan_pullout;
		godot_method_bind *mb_get_surround;
		godot_method_bind *mb_get_time_pullout;
		godot_method_bind *mb_set_pan_pullout;
		godot_method_bind *mb_set_surround;
		godot_method_bind *mb_set_time_pullout;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectStereoEnhance"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectStereoEnhance *_new();

	// methods
	real_t get_pan_pullout() const;
	real_t get_surround() const;
	real_t get_time_pullout() const;
	void set_pan_pullout(const real_t amount);
	void set_surround(const real_t amount);
	void set_time_pullout(const real_t amount);

};

}

#endif