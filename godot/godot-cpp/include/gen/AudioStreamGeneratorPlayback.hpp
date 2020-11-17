#ifndef GODOT_CPP_AUDIOSTREAMGENERATORPLAYBACK_HPP
#define GODOT_CPP_AUDIOSTREAMGENERATORPLAYBACK_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AudioStreamPlaybackResampled.hpp"
namespace godot {


class AudioStreamGeneratorPlayback : public AudioStreamPlaybackResampled {
	struct ___method_bindings {
		godot_method_bind *mb_can_push_buffer;
		godot_method_bind *mb_clear_buffer;
		godot_method_bind *mb_get_frames_available;
		godot_method_bind *mb_get_skips;
		godot_method_bind *mb_push_buffer;
		godot_method_bind *mb_push_frame;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioStreamGeneratorPlayback"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool can_push_buffer(const int64_t amount) const;
	void clear_buffer();
	int64_t get_frames_available() const;
	int64_t get_skips() const;
	bool push_buffer(const PoolVector2Array frames);
	bool push_frame(const Vector2 frame);

};

}

#endif