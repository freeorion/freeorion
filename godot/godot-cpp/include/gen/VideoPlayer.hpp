#ifndef GODOT_CPP_VIDEOPLAYER_HPP
#define GODOT_CPP_VIDEOPLAYER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Control.hpp"
namespace godot {

class VideoStream;
class Texture;

class VideoPlayer : public Control {
	struct ___method_bindings {
		godot_method_bind *mb_get_audio_track;
		godot_method_bind *mb_get_buffering_msec;
		godot_method_bind *mb_get_bus;
		godot_method_bind *mb_get_stream;
		godot_method_bind *mb_get_stream_name;
		godot_method_bind *mb_get_stream_position;
		godot_method_bind *mb_get_video_texture;
		godot_method_bind *mb_get_volume;
		godot_method_bind *mb_get_volume_db;
		godot_method_bind *mb_has_autoplay;
		godot_method_bind *mb_has_expand;
		godot_method_bind *mb_is_paused;
		godot_method_bind *mb_is_playing;
		godot_method_bind *mb_play;
		godot_method_bind *mb_set_audio_track;
		godot_method_bind *mb_set_autoplay;
		godot_method_bind *mb_set_buffering_msec;
		godot_method_bind *mb_set_bus;
		godot_method_bind *mb_set_expand;
		godot_method_bind *mb_set_paused;
		godot_method_bind *mb_set_stream;
		godot_method_bind *mb_set_stream_position;
		godot_method_bind *mb_set_volume;
		godot_method_bind *mb_set_volume_db;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VideoPlayer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VideoPlayer *_new();

	// methods
	int64_t get_audio_track() const;
	int64_t get_buffering_msec() const;
	String get_bus() const;
	Ref<VideoStream> get_stream() const;
	String get_stream_name() const;
	real_t get_stream_position() const;
	Ref<Texture> get_video_texture() const;
	real_t get_volume() const;
	real_t get_volume_db() const;
	bool has_autoplay() const;
	bool has_expand() const;
	bool is_paused() const;
	bool is_playing() const;
	void play();
	void set_audio_track(const int64_t track);
	void set_autoplay(const bool enabled);
	void set_buffering_msec(const int64_t msec);
	void set_bus(const String bus);
	void set_expand(const bool enable);
	void set_paused(const bool paused);
	void set_stream(const Ref<VideoStream> stream);
	void set_stream_position(const real_t position);
	void set_volume(const real_t volume);
	void set_volume_db(const real_t db);
	void stop();

};

}

#endif