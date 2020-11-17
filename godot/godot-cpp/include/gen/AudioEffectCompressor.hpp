#ifndef GODOT_CPP_AUDIOEFFECTCOMPRESSOR_HPP
#define GODOT_CPP_AUDIOEFFECTCOMPRESSOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectCompressor : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_attack_us;
		godot_method_bind *mb_get_gain;
		godot_method_bind *mb_get_mix;
		godot_method_bind *mb_get_ratio;
		godot_method_bind *mb_get_release_ms;
		godot_method_bind *mb_get_sidechain;
		godot_method_bind *mb_get_threshold;
		godot_method_bind *mb_set_attack_us;
		godot_method_bind *mb_set_gain;
		godot_method_bind *mb_set_mix;
		godot_method_bind *mb_set_ratio;
		godot_method_bind *mb_set_release_ms;
		godot_method_bind *mb_set_sidechain;
		godot_method_bind *mb_set_threshold;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectCompressor"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectCompressor *_new();

	// methods
	real_t get_attack_us() const;
	real_t get_gain() const;
	real_t get_mix() const;
	real_t get_ratio() const;
	real_t get_release_ms() const;
	String get_sidechain() const;
	real_t get_threshold() const;
	void set_attack_us(const real_t attack_us);
	void set_gain(const real_t gain);
	void set_mix(const real_t mix);
	void set_ratio(const real_t ratio);
	void set_release_ms(const real_t release_ms);
	void set_sidechain(const String sidechain);
	void set_threshold(const real_t threshold);

};

}

#endif