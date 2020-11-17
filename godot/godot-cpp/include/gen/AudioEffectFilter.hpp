#ifndef GODOT_CPP_AUDIOEFFECTFILTER_HPP
#define GODOT_CPP_AUDIOEFFECTFILTER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AudioEffectFilter.hpp"

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectFilter : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_cutoff;
		godot_method_bind *mb_get_db;
		godot_method_bind *mb_get_gain;
		godot_method_bind *mb_get_resonance;
		godot_method_bind *mb_set_cutoff;
		godot_method_bind *mb_set_db;
		godot_method_bind *mb_set_gain;
		godot_method_bind *mb_set_resonance;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectFilter"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum FilterDB {
		FILTER_6DB = 0,
		FILTER_12DB = 1,
		FILTER_18DB = 2,
		FILTER_24DB = 3,
	};

	// constants


	static AudioEffectFilter *_new();

	// methods
	real_t get_cutoff() const;
	AudioEffectFilter::FilterDB get_db() const;
	real_t get_gain() const;
	real_t get_resonance() const;
	void set_cutoff(const real_t freq);
	void set_db(const int64_t amount);
	void set_gain(const real_t amount);
	void set_resonance(const real_t amount);

};

}

#endif