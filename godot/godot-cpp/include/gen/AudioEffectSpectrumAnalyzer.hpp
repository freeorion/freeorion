#ifndef GODOT_CPP_AUDIOEFFECTSPECTRUMANALYZER_HPP
#define GODOT_CPP_AUDIOEFFECTSPECTRUMANALYZER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AudioEffectSpectrumAnalyzer.hpp"

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectSpectrumAnalyzer : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_buffer_length;
		godot_method_bind *mb_get_fft_size;
		godot_method_bind *mb_get_tap_back_pos;
		godot_method_bind *mb_set_buffer_length;
		godot_method_bind *mb_set_fft_size;
		godot_method_bind *mb_set_tap_back_pos;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectSpectrumAnalyzer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum FFT_Size {
		FFT_SIZE_256 = 0,
		FFT_SIZE_512 = 1,
		FFT_SIZE_1024 = 2,
		FFT_SIZE_2048 = 3,
		FFT_SIZE_4096 = 4,
		FFT_SIZE_MAX = 5,
	};

	// constants


	static AudioEffectSpectrumAnalyzer *_new();

	// methods
	real_t get_buffer_length() const;
	AudioEffectSpectrumAnalyzer::FFT_Size get_fft_size() const;
	real_t get_tap_back_pos() const;
	void set_buffer_length(const real_t seconds);
	void set_fft_size(const int64_t size);
	void set_tap_back_pos(const real_t seconds);

};

}

#endif