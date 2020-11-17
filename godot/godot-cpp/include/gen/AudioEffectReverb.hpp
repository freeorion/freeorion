#ifndef GODOT_CPP_AUDIOEFFECTREVERB_HPP
#define GODOT_CPP_AUDIOEFFECTREVERB_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectReverb : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_damping;
		godot_method_bind *mb_get_dry;
		godot_method_bind *mb_get_hpf;
		godot_method_bind *mb_get_predelay_feedback;
		godot_method_bind *mb_get_predelay_msec;
		godot_method_bind *mb_get_room_size;
		godot_method_bind *mb_get_spread;
		godot_method_bind *mb_get_wet;
		godot_method_bind *mb_set_damping;
		godot_method_bind *mb_set_dry;
		godot_method_bind *mb_set_hpf;
		godot_method_bind *mb_set_predelay_feedback;
		godot_method_bind *mb_set_predelay_msec;
		godot_method_bind *mb_set_room_size;
		godot_method_bind *mb_set_spread;
		godot_method_bind *mb_set_wet;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectReverb"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectReverb *_new();

	// methods
	real_t get_damping() const;
	real_t get_dry() const;
	real_t get_hpf() const;
	real_t get_predelay_feedback() const;
	real_t get_predelay_msec() const;
	real_t get_room_size() const;
	real_t get_spread() const;
	real_t get_wet() const;
	void set_damping(const real_t amount);
	void set_dry(const real_t amount);
	void set_hpf(const real_t amount);
	void set_predelay_feedback(const real_t feedback);
	void set_predelay_msec(const real_t msec);
	void set_room_size(const real_t size);
	void set_spread(const real_t amount);
	void set_wet(const real_t amount);

};

}

#endif