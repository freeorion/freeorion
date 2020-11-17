#ifndef GODOT_CPP_AUDIOEFFECTLIMITER_HPP
#define GODOT_CPP_AUDIOEFFECTLIMITER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectLimiter : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_ceiling_db;
		godot_method_bind *mb_get_soft_clip_db;
		godot_method_bind *mb_get_soft_clip_ratio;
		godot_method_bind *mb_get_threshold_db;
		godot_method_bind *mb_set_ceiling_db;
		godot_method_bind *mb_set_soft_clip_db;
		godot_method_bind *mb_set_soft_clip_ratio;
		godot_method_bind *mb_set_threshold_db;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectLimiter"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectLimiter *_new();

	// methods
	real_t get_ceiling_db() const;
	real_t get_soft_clip_db() const;
	real_t get_soft_clip_ratio() const;
	real_t get_threshold_db() const;
	void set_ceiling_db(const real_t ceiling);
	void set_soft_clip_db(const real_t soft_clip);
	void set_soft_clip_ratio(const real_t soft_clip);
	void set_threshold_db(const real_t threshold);

};

}

#endif