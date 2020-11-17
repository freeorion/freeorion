#ifndef GODOT_CPP_AUDIOEFFECTEQ_HPP
#define GODOT_CPP_AUDIOEFFECTEQ_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioEffect.hpp"
namespace godot {


class AudioEffectEQ : public AudioEffect {
	struct ___method_bindings {
		godot_method_bind *mb_get_band_count;
		godot_method_bind *mb_get_band_gain_db;
		godot_method_bind *mb_set_band_gain_db;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioEffectEQ"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioEffectEQ *_new();

	// methods
	int64_t get_band_count() const;
	real_t get_band_gain_db(const int64_t band_idx) const;
	void set_band_gain_db(const int64_t band_idx, const real_t volume_db);

};

}

#endif