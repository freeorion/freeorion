#include "AudioStreamPlayer3D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AudioStream.hpp"
#include "AudioStreamPlayback.hpp"


namespace godot {


AudioStreamPlayer3D::___method_bindings AudioStreamPlayer3D::___mb = {};

void AudioStreamPlayer3D::___init_method_bindings() {
	___mb.mb__bus_layout_changed = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "_bus_layout_changed");
	___mb.mb__is_active = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "_is_active");
	___mb.mb__set_playing = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "_set_playing");
	___mb.mb_get_area_mask = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_area_mask");
	___mb.mb_get_attenuation_filter_cutoff_hz = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_attenuation_filter_cutoff_hz");
	___mb.mb_get_attenuation_filter_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_attenuation_filter_db");
	___mb.mb_get_attenuation_model = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_attenuation_model");
	___mb.mb_get_bus = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_bus");
	___mb.mb_get_doppler_tracking = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_doppler_tracking");
	___mb.mb_get_emission_angle = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_emission_angle");
	___mb.mb_get_emission_angle_filter_attenuation_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_emission_angle_filter_attenuation_db");
	___mb.mb_get_max_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_max_db");
	___mb.mb_get_max_distance = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_max_distance");
	___mb.mb_get_out_of_range_mode = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_out_of_range_mode");
	___mb.mb_get_pitch_scale = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_pitch_scale");
	___mb.mb_get_playback_position = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_playback_position");
	___mb.mb_get_stream = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_stream");
	___mb.mb_get_stream_paused = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_stream_paused");
	___mb.mb_get_stream_playback = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_stream_playback");
	___mb.mb_get_unit_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_unit_db");
	___mb.mb_get_unit_size = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "get_unit_size");
	___mb.mb_is_autoplay_enabled = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "is_autoplay_enabled");
	___mb.mb_is_emission_angle_enabled = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "is_emission_angle_enabled");
	___mb.mb_is_playing = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "is_playing");
	___mb.mb_play = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "play");
	___mb.mb_seek = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "seek");
	___mb.mb_set_area_mask = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_area_mask");
	___mb.mb_set_attenuation_filter_cutoff_hz = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_attenuation_filter_cutoff_hz");
	___mb.mb_set_attenuation_filter_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_attenuation_filter_db");
	___mb.mb_set_attenuation_model = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_attenuation_model");
	___mb.mb_set_autoplay = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_autoplay");
	___mb.mb_set_bus = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_bus");
	___mb.mb_set_doppler_tracking = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_doppler_tracking");
	___mb.mb_set_emission_angle = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_emission_angle");
	___mb.mb_set_emission_angle_enabled = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_emission_angle_enabled");
	___mb.mb_set_emission_angle_filter_attenuation_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_emission_angle_filter_attenuation_db");
	___mb.mb_set_max_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_max_db");
	___mb.mb_set_max_distance = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_max_distance");
	___mb.mb_set_out_of_range_mode = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_out_of_range_mode");
	___mb.mb_set_pitch_scale = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_pitch_scale");
	___mb.mb_set_stream = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_stream");
	___mb.mb_set_stream_paused = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_stream_paused");
	___mb.mb_set_unit_db = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_unit_db");
	___mb.mb_set_unit_size = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "set_unit_size");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("AudioStreamPlayer3D", "stop");
}

AudioStreamPlayer3D *AudioStreamPlayer3D::_new()
{
	return (AudioStreamPlayer3D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioStreamPlayer3D")());
}
void AudioStreamPlayer3D::_bus_layout_changed() {
	___godot_icall_void(___mb.mb__bus_layout_changed, (const Object *) this);
}

bool AudioStreamPlayer3D::_is_active() const {
	return ___godot_icall_bool(___mb.mb__is_active, (const Object *) this);
}

void AudioStreamPlayer3D::_set_playing(const bool enable) {
	___godot_icall_void_bool(___mb.mb__set_playing, (const Object *) this, enable);
}

int64_t AudioStreamPlayer3D::get_area_mask() const {
	return ___godot_icall_int(___mb.mb_get_area_mask, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_attenuation_filter_cutoff_hz() const {
	return ___godot_icall_float(___mb.mb_get_attenuation_filter_cutoff_hz, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_attenuation_filter_db() const {
	return ___godot_icall_float(___mb.mb_get_attenuation_filter_db, (const Object *) this);
}

AudioStreamPlayer3D::AttenuationModel AudioStreamPlayer3D::get_attenuation_model() const {
	return (AudioStreamPlayer3D::AttenuationModel) ___godot_icall_int(___mb.mb_get_attenuation_model, (const Object *) this);
}

String AudioStreamPlayer3D::get_bus() const {
	return ___godot_icall_String(___mb.mb_get_bus, (const Object *) this);
}

AudioStreamPlayer3D::DopplerTracking AudioStreamPlayer3D::get_doppler_tracking() const {
	return (AudioStreamPlayer3D::DopplerTracking) ___godot_icall_int(___mb.mb_get_doppler_tracking, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_emission_angle() const {
	return ___godot_icall_float(___mb.mb_get_emission_angle, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_emission_angle_filter_attenuation_db() const {
	return ___godot_icall_float(___mb.mb_get_emission_angle_filter_attenuation_db, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_max_db() const {
	return ___godot_icall_float(___mb.mb_get_max_db, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_max_distance() const {
	return ___godot_icall_float(___mb.mb_get_max_distance, (const Object *) this);
}

AudioStreamPlayer3D::OutOfRangeMode AudioStreamPlayer3D::get_out_of_range_mode() const {
	return (AudioStreamPlayer3D::OutOfRangeMode) ___godot_icall_int(___mb.mb_get_out_of_range_mode, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_pitch_scale() const {
	return ___godot_icall_float(___mb.mb_get_pitch_scale, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_playback_position() {
	return ___godot_icall_float(___mb.mb_get_playback_position, (const Object *) this);
}

Ref<AudioStream> AudioStreamPlayer3D::get_stream() const {
	return Ref<AudioStream>::__internal_constructor(___godot_icall_Object(___mb.mb_get_stream, (const Object *) this));
}

bool AudioStreamPlayer3D::get_stream_paused() const {
	return ___godot_icall_bool(___mb.mb_get_stream_paused, (const Object *) this);
}

Ref<AudioStreamPlayback> AudioStreamPlayer3D::get_stream_playback() {
	return Ref<AudioStreamPlayback>::__internal_constructor(___godot_icall_Object(___mb.mb_get_stream_playback, (const Object *) this));
}

real_t AudioStreamPlayer3D::get_unit_db() const {
	return ___godot_icall_float(___mb.mb_get_unit_db, (const Object *) this);
}

real_t AudioStreamPlayer3D::get_unit_size() const {
	return ___godot_icall_float(___mb.mb_get_unit_size, (const Object *) this);
}

bool AudioStreamPlayer3D::is_autoplay_enabled() {
	return ___godot_icall_bool(___mb.mb_is_autoplay_enabled, (const Object *) this);
}

bool AudioStreamPlayer3D::is_emission_angle_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_emission_angle_enabled, (const Object *) this);
}

bool AudioStreamPlayer3D::is_playing() const {
	return ___godot_icall_bool(___mb.mb_is_playing, (const Object *) this);
}

void AudioStreamPlayer3D::play(const real_t from_position) {
	___godot_icall_void_float(___mb.mb_play, (const Object *) this, from_position);
}

void AudioStreamPlayer3D::seek(const real_t to_position) {
	___godot_icall_void_float(___mb.mb_seek, (const Object *) this, to_position);
}

void AudioStreamPlayer3D::set_area_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_area_mask, (const Object *) this, mask);
}

void AudioStreamPlayer3D::set_attenuation_filter_cutoff_hz(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_attenuation_filter_cutoff_hz, (const Object *) this, degrees);
}

void AudioStreamPlayer3D::set_attenuation_filter_db(const real_t db) {
	___godot_icall_void_float(___mb.mb_set_attenuation_filter_db, (const Object *) this, db);
}

void AudioStreamPlayer3D::set_attenuation_model(const int64_t model) {
	___godot_icall_void_int(___mb.mb_set_attenuation_model, (const Object *) this, model);
}

void AudioStreamPlayer3D::set_autoplay(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_autoplay, (const Object *) this, enable);
}

void AudioStreamPlayer3D::set_bus(const String bus) {
	___godot_icall_void_String(___mb.mb_set_bus, (const Object *) this, bus);
}

void AudioStreamPlayer3D::set_doppler_tracking(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_doppler_tracking, (const Object *) this, mode);
}

void AudioStreamPlayer3D::set_emission_angle(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_emission_angle, (const Object *) this, degrees);
}

void AudioStreamPlayer3D::set_emission_angle_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_emission_angle_enabled, (const Object *) this, enabled);
}

void AudioStreamPlayer3D::set_emission_angle_filter_attenuation_db(const real_t db) {
	___godot_icall_void_float(___mb.mb_set_emission_angle_filter_attenuation_db, (const Object *) this, db);
}

void AudioStreamPlayer3D::set_max_db(const real_t max_db) {
	___godot_icall_void_float(___mb.mb_set_max_db, (const Object *) this, max_db);
}

void AudioStreamPlayer3D::set_max_distance(const real_t metres) {
	___godot_icall_void_float(___mb.mb_set_max_distance, (const Object *) this, metres);
}

void AudioStreamPlayer3D::set_out_of_range_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_out_of_range_mode, (const Object *) this, mode);
}

void AudioStreamPlayer3D::set_pitch_scale(const real_t pitch_scale) {
	___godot_icall_void_float(___mb.mb_set_pitch_scale, (const Object *) this, pitch_scale);
}

void AudioStreamPlayer3D::set_stream(const Ref<AudioStream> stream) {
	___godot_icall_void_Object(___mb.mb_set_stream, (const Object *) this, stream.ptr());
}

void AudioStreamPlayer3D::set_stream_paused(const bool pause) {
	___godot_icall_void_bool(___mb.mb_set_stream_paused, (const Object *) this, pause);
}

void AudioStreamPlayer3D::set_unit_db(const real_t unit_db) {
	___godot_icall_void_float(___mb.mb_set_unit_db, (const Object *) this, unit_db);
}

void AudioStreamPlayer3D::set_unit_size(const real_t unit_size) {
	___godot_icall_void_float(___mb.mb_set_unit_size, (const Object *) this, unit_size);
}

void AudioStreamPlayer3D::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}