#ifndef GODOT_CPP_AUDIOSTREAMOGGVORBIS_HPP
#define GODOT_CPP_AUDIOSTREAMOGGVORBIS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioStream.hpp"
namespace godot {


class AudioStreamOGGVorbis : public AudioStream {
	struct ___method_bindings {
		godot_method_bind *mb_get_data;
		godot_method_bind *mb_get_loop_offset;
		godot_method_bind *mb_has_loop;
		godot_method_bind *mb_set_data;
		godot_method_bind *mb_set_loop;
		godot_method_bind *mb_set_loop_offset;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioStreamOGGVorbis"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AudioStreamOGGVorbis *_new();

	// methods
	PoolByteArray get_data() const;
	real_t get_loop_offset() const;
	bool has_loop() const;
	void set_data(const PoolByteArray data);
	void set_loop(const bool enable);
	void set_loop_offset(const real_t seconds);

};

}

#endif