#ifndef GODOT_CPP_AUDIOEFFECTPHASER_HPP
#define GODOT_CPP_AUDIOEFFECTPHASER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectPhaser : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_depth;
		godot_method_bind *mb_get_feedback;
		godot_method_bind *mb_get_range_max_hz;
		godot_method_bind *mb_get_range_min_hz;
		godot_method_bind *mb_get_rate_hz;
		godot_method_bind *mb_set_depth;
		godot_method_bind *mb_set_feedback;
		godot_method_bind *mb_set_range_max_hz;
		godot_method_bind *mb_set_range_min_hz;
		godot_method_bind *mb_set_rate_hz;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectPhaser"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectPhaser *_new();

	// methods
	real_t get_depth() const;
	real_t get_feedback() const;
	real_t get_range_max_hz() const;
	real_t get_range_min_hz() const;
	real_t get_rate_hz() const;
	void set_depth(const real_t depth);
	void set_feedback(const real_t fbk);
	void set_range_max_hz(const real_t hz);
	void set_range_min_hz(const real_t hz);
	void set_rate_hz(const real_t hz);

};

}

#endif