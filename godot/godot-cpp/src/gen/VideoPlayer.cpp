#include "VideoPlayer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "VideoStream.hpp"
#include "Texture.hpp"


namespace godot {


VideoPlayer::___method_bindings VideoPlayer::___mb = {};

void VideoPlayer::___init_method_bindings() {
	___mb.mb_get_audio_track = godot::api->godot_method_bind_get_method("VideoPlayer", "get_audio_track");
	___mb.mb_get_buffering_msec = godot::api->godot_method_bind_get_method("VideoPlayer", "get_buffering_msec");
	___mb.mb_get_bus = godot::api->godot_method_bind_get_method("VideoPlayer", "get_bus");
	___mb.mb_get_stream = godot::api->godot_method_bind_get_method("VideoPlayer", "get_stream");
	___mb.mb_get_stream_name = godot::api->godot_method_bind_get_method("VideoPlayer", "get_stream_name");
	___mb.mb_get_stream_position = godot::api->godot_method_bind_get_method("VideoPlayer", "get_stream_position");
	___mb.mb_get_video_texture = godot::api->godot_method_bind_get_method("VideoPlayer", "get_video_texture");
	___mb.mb_get_volume = godot::api->godot_method_bind_get_method("VideoPlayer", "get_volume");
	___mb.mb_get_volume_db = godot::api->godot_method_bind_get_method("VideoPlayer", "get_volume_db");
	___mb.mb_has_autoplay = godot::api->godot_method_bind_get_method("VideoPlayer", "has_autoplay");
	___mb.mb_has_expand = godot::api->godot_method_bind_get_method("VideoPlayer", "has_expand");
	___mb.mb_is_paused = godot::api->godot_method_bind_get_method("VideoPlayer", "is_paused");
	___mb.mb_is_playing = godot::api->godot_method_bind_get_method("VideoPlayer", "is_playing");
	___mb.mb_play = godot::api->godot_method_bind_get_method("VideoPlayer", "play");
	___mb.mb_set_audio_track = godot::api->godot_method_bind_get_method("VideoPlayer", "set_audio_track");
	___mb.mb_set_autoplay = godot::api->godot_method_bind_get_method("VideoPlayer", "set_autoplay");
	___mb.mb_set_buffering_msec = godot::api->godot_method_bind_get_method("VideoPlayer", "set_buffering_msec");
	___mb.mb_set_bus = godot::api->godot_method_bind_get_method("VideoPlayer", "set_bus");
	___mb.mb_set_expand = godot::api->godot_method_bind_get_method("VideoPlayer", "set_expand");
	___mb.mb_set_paused = godot::api->godot_method_bind_get_method("VideoPlayer", "set_paused");
	___mb.mb_set_stream = godot::api->godot_method_bind_get_method("VideoPlayer", "set_stream");
	___mb.mb_set_stream_position = godot::api->godot_method_bind_get_method("VideoPlayer", "set_stream_position");
	___mb.mb_set_volume = godot::api->godot_method_bind_get_method("VideoPlayer", "set_volume");
	___mb.mb_set_volume_db = godot::api->godot_method_bind_get_method("VideoPlayer", "set_volume_db");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("VideoPlayer", "stop");
}

VideoPlayer *VideoPlayer::_new()
{
	return (VideoPlayer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VideoPlayer")());
}
int64_t VideoPlayer::get_audio_track() const {
	return ___godot_icall_int(___mb.mb_get_audio_track, (const Object *) this);
}

int64_t VideoPlayer::get_buffering_msec() const {
	return ___godot_icall_int(___mb.mb_get_buffering_msec, (const Object *) this);
}

String VideoPlayer::get_bus() const {
	return ___godot_icall_String(___mb.mb_get_bus, (const Object *) this);
}

Ref<VideoStream> VideoPlayer::get_stream() const {
	return Ref<VideoStream>::__internal_constructor(___godot_icall_Object(___mb.mb_get_stream, (const Object *) this));
}

String VideoPlayer::get_stream_name() const {
	return ___godot_icall_String(___mb.mb_get_stream_name, (const Object *) this);
}

real_t VideoPlayer::get_stream_position() const {
	return ___godot_icall_float(___mb.mb_get_stream_position, (const Object *) this);
}

Ref<Texture> VideoPlayer::get_video_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_video_texture, (const Object *) this));
}

real_t VideoPlayer::get_volume() const {
	return ___godot_icall_float(___mb.mb_get_volume, (const Object *) this);
}

real_t VideoPlayer::get_volume_db() const {
	return ___godot_icall_float(___mb.mb_get_volume_db, (const Object *) this);
}

bool VideoPlayer::has_autoplay() const {
	return ___godot_icall_bool(___mb.mb_has_autoplay, (const Object *) this);
}

bool VideoPlayer::has_expand() const {
	return ___godot_icall_bool(___mb.mb_has_expand, (const Object *) this);
}

bool VideoPlayer::is_paused() const {
	return ___godot_icall_bool(___mb.mb_is_paused, (const Object *) this);
}

bool VideoPlayer::is_playing() const {
	return ___godot_icall_bool(___mb.mb_is_playing, (const Object *) this);
}

void VideoPlayer::play() {
	___godot_icall_void(___mb.mb_play, (const Object *) this);
}

void VideoPlayer::set_audio_track(const int64_t track) {
	___godot_icall_void_int(___mb.mb_set_audio_track, (const Object *) this, track);
}

void VideoPlayer::set_autoplay(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_autoplay, (const Object *) this, enabled);
}

void VideoPlayer::set_buffering_msec(const int64_t msec) {
	___godot_icall_void_int(___mb.mb_set_buffering_msec, (const Object *) this, msec);
}

void VideoPlayer::set_bus(const String bus) {
	___godot_icall_void_String(___mb.mb_set_bus, (const Object *) this, bus);
}

void VideoPlayer::set_expand(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_expand, (const Object *) this, enable);
}

void VideoPlayer::set_paused(const bool paused) {
	___godot_icall_void_bool(___mb.mb_set_paused, (const Object *) this, paused);
}

void VideoPlayer::set_stream(const Ref<VideoStream> stream) {
	___godot_icall_void_Object(___mb.mb_set_stream, (const Object *) this, stream.ptr());
}

void VideoPlayer::set_stream_position(const real_t position) {
	___godot_icall_void_float(___mb.mb_set_stream_position, (const Object *) this, position);
}

void VideoPlayer::set_volume(const real_t volume) {
	___godot_icall_void_float(___mb.mb_set_volume, (const Object *) this, volume);
}

void VideoPlayer::set_volume_db(const real_t db) {
	___godot_icall_void_float(___mb.mb_set_volume_db, (const Object *) this, db);
}

void VideoPlayer::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}