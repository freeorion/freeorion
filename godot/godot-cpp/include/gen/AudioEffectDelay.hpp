#ifndef GODOT_CPP_AUDIOEFFECTDELAY_HPP
#define GODOT_CPP_AUDIOEFFECTDELAY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectDelay : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_dry;
		godot_method_bind *mb_get_feedback_delay_ms;
		godot_method_bind *mb_get_feedback_level_db;
		godot_method_bind *mb_get_feedback_lowpass;
		godot_method_bind *mb_get_tap1_delay_ms;
		godot_method_bind *mb_get_tap1_level_db;
		godot_method_bind *mb_get_tap1_pan;
		godot_method_bind *mb_get_tap2_delay_ms;
		godot_method_bind *mb_get_tap2_level_db;
		godot_method_bind *mb_get_tap2_pan;
		godot_method_bind *mb_is_feedback_active;
		godot_method_bind *mb_is_tap1_active;
		godot_method_bind *mb_is_tap2_active;
		godot_method_bind *mb_set_dry;
		godot_method_bind *mb_set_feedback_active;
		godot_method_bind *mb_set_feedback_delay_ms;
		godot_method_bind *mb_set_feedback_level_db;
		godot_method_bind *mb_set_feedback_lowpass;
		godot_method_bind *mb_set_tap1_active;
		godot_method_bind *mb_set_tap1_delay_ms;
		godot_method_bind *mb_set_tap1_level_db;
		godot_method_bind *mb_set_tap1_pan;
		godot_method_bind *mb_set_tap2_active;
		godot_method_bind *mb_set_tap2_delay_ms;
		godot_method_bind *mb_set_tap2_level_db;
		godot_method_bind *mb_set_tap2_pan;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectDelay"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectDelay *_new();

	// methods
	real_t get_dry();
	real_t get_feedback_delay_ms() const;
	real_t get_feedback_level_db() const;
	real_t get_feedback_lowpass() const;
	real_t get_tap1_delay_ms() const;
	real_t get_tap1_level_db() const;
	real_t get_tap1_pan() const;
	real_t get_tap2_delay_ms() const;
	real_t get_tap2_level_db() const;
	real_t get_tap2_pan() const;
	bool is_feedback_active() const;
	bool is_tap1_active() const;
	bool is_tap2_active() const;
	void set_dry(const real_t amount);
	void set_feedback_active(const bool amount);
	void set_feedback_delay_ms(const real_t amount);
	void set_feedback_level_db(const real_t amount);
	void set_feedback_lowpass(const real_t amount);
	void set_tap1_active(const bool amount);
	void set_tap1_delay_ms(const real_t amount);
	void set_tap1_level_db(const real_t amount);
	void set_tap1_pan(const real_t amount);
	void set_tap2_active(const bool amount);
	void set_tap2_delay_ms(const real_t amount);
	void set_tap2_level_db(const real_t amount);
	void set_tap2_pan(const real_t amount);

};

}

#endif