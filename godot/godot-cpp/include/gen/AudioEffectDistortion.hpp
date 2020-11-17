#ifndef GODOT_CPP_AUDIOEFFECTDISTORTION_HPP
#define GODOT_CPP_AUDIOEFFECTDISTORTION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AudioEffectDistortion.hpp"

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectDistortion : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_drive;
		godot_method_bind *mb_get_keep_hf_hz;
		godot_method_bind *mb_get_mode;
		godot_method_bind *mb_get_post_gain;
		godot_method_bind *mb_get_pre_gain;
		godot_method_bind *mb_set_drive;
		godot_method_bind *mb_set_keep_hf_hz;
		godot_method_bind *mb_set_mode;
		godot_method_bind *mb_set_post_gain;
		godot_method_bind *mb_set_pre_gain;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectDistortion"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Mode {
		MODE_CLIP = 0,
		MODE_ATAN = 1,
		MODE_LOFI = 2,
		MODE_OVERDRIVE = 3,
		MODE_WAVESHAPE = 4,
	};

	// constants


	static AudioEffectDistortion *_new();

	// methods
	real_t get_drive() const;
	real_t get_keep_hf_hz() const;
	AudioEffectDistortion::Mode get_mode() const;
	real_t get_post_gain() const;
	real_t get_pre_gain() const;
	void set_drive(const real_t drive);
	void set_keep_hf_hz(const real_t keep_hf_hz);
	void set_mode(const int64_t mode);
	void set_post_gain(const real_t post_gain);
	void set_pre_gain(const real_t pre_gain);

};

}

#endif