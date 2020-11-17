#ifndef GODOT_CPP_AUDIOEFFECTCHORUS_HPP
#define GODOT_CPP_AUDIOEFFECTCHORUS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectChorus : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_dry;
		godot_method_bind *mb_get_voice_count;
		godot_method_bind *mb_get_voice_cutoff_hz;
		godot_method_bind *mb_get_voice_delay_ms;
		godot_method_bind *mb_get_voice_depth_ms;
		godot_method_bind *mb_get_voice_level_db;
		godot_method_bind *mb_get_voice_pan;
		godot_method_bind *mb_get_voice_rate_hz;
		godot_method_bind *mb_get_wet;
		godot_method_bind *mb_set_dry;
		godot_method_bind *mb_set_voice_count;
		godot_method_bind *mb_set_voice_cutoff_hz;
		godot_method_bind *mb_set_voice_delay_ms;
		godot_method_bind *mb_set_voice_depth_ms;
		godot_method_bind *mb_set_voice_level_db;
		godot_method_bind *mb_set_voice_pan;
		godot_method_bind *mb_set_voice_rate_hz;
		godot_method_bind *mb_set_wet;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectChorus"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectChorus *_new();

	// methods
	real_t get_dry() const;
	int64_t get_voice_count() const;
	real_t get_voice_cutoff_hz(const int64_t voice_idx) const;
	real_t get_voice_delay_ms(const int64_t voice_idx) const;
	real_t get_voice_depth_ms(const int64_t voice_idx) const;
	real_t get_voice_level_db(const int64_t voice_idx) const;
	real_t get_voice_pan(const int64_t voice_idx) const;
	real_t get_voice_rate_hz(const int64_t voice_idx) const;
	real_t get_wet() const;
	void set_dry(const real_t amount);
	void set_voice_count(const int64_t voices);
	void set_voice_cutoff_hz(const int64_t voice_idx, const real_t cutoff_hz);
	void set_voice_delay_ms(const int64_t voice_idx, const real_t delay_ms);
	void set_voice_depth_ms(const int64_t voice_idx, const real_t depth_ms);
	void set_voice_level_db(const int64_t voice_idx, const real_t level_db);
	void set_voice_pan(const int64_t voice_idx, const real_t pan);
	void set_voice_rate_hz(const int64_t voice_idx, const real_t rate_hz);
	void set_wet(const real_t amount);

};

}

#endif