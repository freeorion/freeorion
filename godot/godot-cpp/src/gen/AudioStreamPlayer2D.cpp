#include "AudioStreamPlayer2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AudioStream.hpp"
#include "AudioStreamPlayback.hpp"


namespace godot {


AudioStreamPlayer2D::___method_bindings AudioStreamPlayer2D::___mb = {};

void AudioStreamPlayer2D::___init_method_bindings() {
	___mb.mb__bus_layout_changed = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "_bus_layout_changed");
	___mb.mb__is_active = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "_is_active");
	___mb.mb__set_playing = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "_set_playing");
	___mb.mb_get_area_mask = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_area_mask");
	___mb.mb_get_attenuation = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_attenuation");
	___mb.mb_get_bus = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_bus");
	___mb.mb_get_max_distance = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_max_distance");
	___mb.mb_get_pitch_scale = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_pitch_scale");
	___mb.mb_get_playback_position = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_playback_position");
	___mb.mb_get_stream = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_stream");
	___mb.mb_get_stream_paused = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_stream_paused");
	___mb.mb_get_stream_playback = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_stream_playback");
	___mb.mb_get_volume_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "get_volume_db");
	___mb.mb_is_autoplay_enabled = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "is_autoplay_enabled");
	___mb.mb_is_playing = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "is_playing");
	___mb.mb_play = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "play");
	___mb.mb_seek = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "seek");
	___mb.mb_set_area_mask = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_area_mask");
	___mb.mb_set_attenuation = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_attenuation");
	___mb.mb_set_autoplay = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_autoplay");
	___mb.mb_set_bus = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_bus");
	___mb.mb_set_max_distance = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_max_distance");
	___mb.mb_set_pitch_scale = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_pitch_scale");
	___mb.mb_set_stream = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_stream");
	___mb.mb_set_stream_paused = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_stream_paused");
	___mb.mb_set_volume_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "set_volume_db");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("AudioStreamPlayer2D", "stop");
}

AudioStreamPlayer2D *AudioStreamPlayer2D::_new()
{
	return (AudioStreamPlayer2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioStreamPlayer2D")());
}
void AudioStreamPlayer2D::_bus_layout_changed() {
	___godot_icall_void(___mb.mb__bus_layout_changed, (const Object *) this);
}

bool AudioStreamPlayer2D::_is_active() const {
	return ___godot_icall_bool(___mb.mb__is_active, (const Object *) this);
}

void AudioStreamPlayer2D::_set_playing(const bool enable) {
	___godot_icall_void_bool(___mb.mb__set_playing, (const Object *) this, enable);
}

int64_t AudioStreamPlayer2D::get_area_mask() const {
	return ___godot_icall_int(___mb.mb_get_area_mask, (const Object *) this);
}

real_t AudioStreamPlayer2D::get_attenuation() const {
	return ___godot_icall_float(___mb.mb_get_attenuation, (const Object *) this);
}

String AudioStreamPlayer2D::get_bus() const {
	return ___godot_icall_String(___mb.mb_get_bus, (const Object *) this);
}

real_t AudioStreamPlayer2D::get_max_distance() const {
	return ___godot_icall_float(___mb.mb_get_max_distance, (const Object *) this);
}

real_t AudioStreamPlayer2D::get_pitch_scale() const {
	return ___godot_icall_float(___mb.mb_get_pitch_scale, (const Object *) this);
}

real_t AudioStreamPlayer2D::get_playback_position() {
	return ___godot_icall_float(___mb.mb_get_playback_position, (const Object *) this);
}

Ref<AudioStream> AudioStreamPlayer2D::get_stream() const {
	return Ref<AudioStream>::__internal_constructor(___godot_icall_Object(___mb.mb_get_stream, (const Object *) this));
}

bool AudioStreamPlayer2D::get_stream_paused() const {
	return ___godot_icall_bool(___mb.mb_get_stream_paused, (const Object *) this);
}

Ref<AudioStreamPlayback> AudioStreamPlayer2D::get_stream_playback() {
	return Ref<AudioStreamPlayback>::__internal_constructor(___godot_icall_Object(___mb.mb_get_stream_playback, (const Object *) this));
}

real_t AudioStreamPlayer2D::get_volume_db() const {
	return ___godot_icall_float(___mb.mb_get_volume_db, (const Object *) this);
}

bool AudioStreamPlayer2D::is_autoplay_enabled() {
	return ___godot_icall_bool(___mb.mb_is_autoplay_enabled, (const Object *) this);
}

bool AudioStreamPlayer2D::is_playing() const {
	return ___godot_icall_bool(___mb.mb_is_playing, (const Object *) this);
}

void AudioStreamPlayer2D::play(const real_t from_position) {
	___godot_icall_void_float(___mb.mb_play, (const Object *) this, from_position);
}

void AudioStreamPlayer2D::seek(const real_t to_position) {
	___godot_icall_void_float(___mb.mb_seek, (const Object *) this, to_position);
}

void AudioStreamPlayer2D::set_area_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_area_mask, (const Object *) this, mask);
}

void AudioStreamPlayer2D::set_attenuation(const real_t curve) {
	___godot_icall_void_float(___mb.mb_set_attenuation, (const Object *) this, curve);
}

void AudioStreamPlayer2D::set_autoplay(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_autoplay, (const Object *) this, enable);
}

void AudioStreamPlayer2D::set_bus(const String bus) {
	___godot_icall_void_String(___mb.mb_set_bus, (const Object *) this, bus);
}

void AudioStreamPlayer2D::set_max_distance(const real_t pixels) {
	___godot_icall_void_float(___mb.mb_set_max_distance, (const Object *) this, pixels);
}

void AudioStreamPlayer2D::set_pitch_scale(const real_t pitch_scale) {
	___godot_icall_void_float(___mb.mb_set_pitch_scale, (const Object *) this, pitch_scale);
}

void AudioStreamPlayer2D::set_stream(const Ref<AudioStream> stream) {
	___godot_icall_void_Object(___mb.mb_set_stream, (const Object *) this, stream.ptr());
}

void AudioStreamPlayer2D::set_stream_paused(const bool pause) {
	___godot_icall_void_bool(___mb.mb_set_stream_paused, (const Object *) this, pause);
}

void AudioStreamPlayer2D::set_volume_db(const real_t volume_db) {
	___godot_icall_void_float(___mb.mb_set_volume_db, (const Object *) this, volume_db);
}

void AudioStreamPlayer2D::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}