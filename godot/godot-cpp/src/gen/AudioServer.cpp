#include "AudioServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AudioEffect.hpp"
#include "AudioBusLayout.hpp"
#include "AudioEffectInstance.hpp"


namespace godot {


AudioServer *AudioServer::_singleton = NULL;


AudioServer::AudioServer() {
	_owner = godot::api->godot_global_get_singleton((char *) "AudioServer");
}


AudioServer::___method_bindings AudioServer::___mb = {};

void AudioServer::___init_method_bindings() {
	___mb.mb_add_bus = godot::api->godot_method_bind_get_method("AudioServer", "add_bus");
	___mb.mb_add_bus_effect = godot::api->godot_method_bind_get_method("AudioServer", "add_bus_effect");
	___mb.mb_capture_get_device = godot::api->godot_method_bind_get_method("AudioServer", "capture_get_device");
	___mb.mb_capture_get_device_list = godot::api->godot_method_bind_get_method("AudioServer", "capture_get_device_list");
	___mb.mb_capture_set_device = godot::api->godot_method_bind_get_method("AudioServer", "capture_set_device");
	___mb.mb_generate_bus_layout = godot::api->godot_method_bind_get_method("AudioServer", "generate_bus_layout");
	___mb.mb_get_bus_channels = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_channels");
	___mb.mb_get_bus_count = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_count");
	___mb.mb_get_bus_effect = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_effect");
	___mb.mb_get_bus_effect_count = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_effect_count");
	___mb.mb_get_bus_effect_instance = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_effect_instance");
	___mb.mb_get_bus_index = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_index");
	___mb.mb_get_bus_name = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_name");
	___mb.mb_get_bus_peak_volume_left_db = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_peak_volume_left_db");
	___mb.mb_get_bus_peak_volume_right_db = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_peak_volume_right_db");
	___mb.mb_get_bus_send = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_send");
	___mb.mb_get_bus_volume_db = godot::api->godot_method_bind_get_method("AudioServer", "get_bus_volume_db");
	___mb.mb_get_device = godot::api->godot_method_bind_get_method("AudioServer", "get_device");
	___mb.mb_get_device_list = godot::api->godot_method_bind_get_method("AudioServer", "get_device_list");
	___mb.mb_get_global_rate_scale = godot::api->godot_method_bind_get_method("AudioServer", "get_global_rate_scale");
	___mb.mb_get_mix_rate = godot::api->godot_method_bind_get_method("AudioServer", "get_mix_rate");
	___mb.mb_get_output_latency = godot::api->godot_method_bind_get_method("AudioServer", "get_output_latency");
	___mb.mb_get_speaker_mode = godot::api->godot_method_bind_get_method("AudioServer", "get_speaker_mode");
	___mb.mb_get_time_since_last_mix = godot::api->godot_method_bind_get_method("AudioServer", "get_time_since_last_mix");
	___mb.mb_get_time_to_next_mix = godot::api->godot_method_bind_get_method("AudioServer", "get_time_to_next_mix");
	___mb.mb_is_bus_bypassing_effects = godot::api->godot_method_bind_get_method("AudioServer", "is_bus_bypassing_effects");
	___mb.mb_is_bus_effect_enabled = godot::api->godot_method_bind_get_method("AudioServer", "is_bus_effect_enabled");
	___mb.mb_is_bus_mute = godot::api->godot_method_bind_get_method("AudioServer", "is_bus_mute");
	___mb.mb_is_bus_solo = godot::api->godot_method_bind_get_method("AudioServer", "is_bus_solo");
	___mb.mb_lock = godot::api->godot_method_bind_get_method("AudioServer", "lock");
	___mb.mb_move_bus = godot::api->godot_method_bind_get_method("AudioServer", "move_bus");
	___mb.mb_remove_bus = godot::api->godot_method_bind_get_method("AudioServer", "remove_bus");
	___mb.mb_remove_bus_effect = godot::api->godot_method_bind_get_method("AudioServer", "remove_bus_effect");
	___mb.mb_set_bus_bypass_effects = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_bypass_effects");
	___mb.mb_set_bus_count = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_count");
	___mb.mb_set_bus_effect_enabled = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_effect_enabled");
	___mb.mb_set_bus_layout = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_layout");
	___mb.mb_set_bus_mute = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_mute");
	___mb.mb_set_bus_name = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_name");
	___mb.mb_set_bus_send = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_send");
	___mb.mb_set_bus_solo = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_solo");
	___mb.mb_set_bus_volume_db = godot::api->godot_method_bind_get_method("AudioServer", "set_bus_volume_db");
	___mb.mb_set_device = godot::api->godot_method_bind_get_method("AudioServer", "set_device");
	___mb.mb_set_global_rate_scale = godot::api->godot_method_bind_get_method("AudioServer", "set_global_rate_scale");
	___mb.mb_swap_bus_effects = godot::api->godot_method_bind_get_method("AudioServer", "swap_bus_effects");
	___mb.mb_unlock = godot::api->godot_method_bind_get_method("AudioServer", "unlock");
}

void AudioServer::add_bus(const int64_t at_position) {
	___godot_icall_void_int(___mb.mb_add_bus, (const Object *) this, at_position);
}

void AudioServer::add_bus_effect(const int64_t bus_idx, const Ref<AudioEffect> effect, const int64_t at_position) {
	___godot_icall_void_int_Object_int(___mb.mb_add_bus_effect, (const Object *) this, bus_idx, effect.ptr(), at_position);
}

String AudioServer::capture_get_device() {
	return ___godot_icall_String(___mb.mb_capture_get_device, (const Object *) this);
}

Array AudioServer::capture_get_device_list() {
	return ___godot_icall_Array(___mb.mb_capture_get_device_list, (const Object *) this);
}

void AudioServer::capture_set_device(const String name) {
	___godot_icall_void_String(___mb.mb_capture_set_device, (const Object *) this, name);
}

Ref<AudioBusLayout> AudioServer::generate_bus_layout() const {
	return Ref<AudioBusLayout>::__internal_constructor(___godot_icall_Object(___mb.mb_generate_bus_layout, (const Object *) this));
}

int64_t AudioServer::get_bus_channels(const int64_t bus_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_bus_channels, (const Object *) this, bus_idx);
}

int64_t AudioServer::get_bus_count() const {
	return ___godot_icall_int(___mb.mb_get_bus_count, (const Object *) this);
}

Ref<AudioEffect> AudioServer::get_bus_effect(const int64_t bus_idx, const int64_t effect_idx) {
	return Ref<AudioEffect>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_get_bus_effect, (const Object *) this, bus_idx, effect_idx));
}

int64_t AudioServer::get_bus_effect_count(const int64_t bus_idx) {
	return ___godot_icall_int_int(___mb.mb_get_bus_effect_count, (const Object *) this, bus_idx);
}

Ref<AudioEffectInstance> AudioServer::get_bus_effect_instance(const int64_t bus_idx, const int64_t effect_idx, const int64_t channel) {
	return Ref<AudioEffectInstance>::__internal_constructor(___godot_icall_Object_int_int_int(___mb.mb_get_bus_effect_instance, (const Object *) this, bus_idx, effect_idx, channel));
}

int64_t AudioServer::get_bus_index(const String bus_name) const {
	return ___godot_icall_int_String(___mb.mb_get_bus_index, (const Object *) this, bus_name);
}

String AudioServer::get_bus_name(const int64_t bus_idx) const {
	return ___godot_icall_String_int(___mb.mb_get_bus_name, (const Object *) this, bus_idx);
}

real_t AudioServer::get_bus_peak_volume_left_db(const int64_t bus_idx, const int64_t channel) const {
	return ___godot_icall_float_int_int(___mb.mb_get_bus_peak_volume_left_db, (const Object *) this, bus_idx, channel);
}

real_t AudioServer::get_bus_peak_volume_right_db(const int64_t bus_idx, const int64_t channel) const {
	return ___godot_icall_float_int_int(___mb.mb_get_bus_peak_volume_right_db, (const Object *) this, bus_idx, channel);
}

String AudioServer::get_bus_send(const int64_t bus_idx) const {
	return ___godot_icall_String_int(___mb.mb_get_bus_send, (const Object *) this, bus_idx);
}

real_t AudioServer::get_bus_volume_db(const int64_t bus_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_bus_volume_db, (const Object *) this, bus_idx);
}

String AudioServer::get_device() {
	return ___godot_icall_String(___mb.mb_get_device, (const Object *) this);
}

Array AudioServer::get_device_list() {
	return ___godot_icall_Array(___mb.mb_get_device_list, (const Object *) this);
}

real_t AudioServer::get_global_rate_scale() const {
	return ___godot_icall_float(___mb.mb_get_global_rate_scale, (const Object *) this);
}

real_t AudioServer::get_mix_rate() const {
	return ___godot_icall_float(___mb.mb_get_mix_rate, (const Object *) this);
}

real_t AudioServer::get_output_latency() const {
	return ___godot_icall_float(___mb.mb_get_output_latency, (const Object *) this);
}

AudioServer::SpeakerMode AudioServer::get_speaker_mode() const {
	return (AudioServer::SpeakerMode) ___godot_icall_int(___mb.mb_get_speaker_mode, (const Object *) this);
}

real_t AudioServer::get_time_since_last_mix() const {
	return ___godot_icall_float(___mb.mb_get_time_since_last_mix, (const Object *) this);
}

real_t AudioServer::get_time_to_next_mix() const {
	return ___godot_icall_float(___mb.mb_get_time_to_next_mix, (const Object *) this);
}

bool AudioServer::is_bus_bypassing_effects(const int64_t bus_idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_bus_bypassing_effects, (const Object *) this, bus_idx);
}

bool AudioServer::is_bus_effect_enabled(const int64_t bus_idx, const int64_t effect_idx) const {
	return ___godot_icall_bool_int_int(___mb.mb_is_bus_effect_enabled, (const Object *) this, bus_idx, effect_idx);
}

bool AudioServer::is_bus_mute(const int64_t bus_idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_bus_mute, (const Object *) this, bus_idx);
}

bool AudioServer::is_bus_solo(const int64_t bus_idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_bus_solo, (const Object *) this, bus_idx);
}

void AudioServer::lock() {
	___godot_icall_void(___mb.mb_lock, (const Object *) this);
}

void AudioServer::move_bus(const int64_t index, const int64_t to_index) {
	___godot_icall_void_int_int(___mb.mb_move_bus, (const Object *) this, index, to_index);
}

void AudioServer::remove_bus(const int64_t index) {
	___godot_icall_void_int(___mb.mb_remove_bus, (const Object *) this, index);
}

void AudioServer::remove_bus_effect(const int64_t bus_idx, const int64_t effect_idx) {
	___godot_icall_void_int_int(___mb.mb_remove_bus_effect, (const Object *) this, bus_idx, effect_idx);
}

void AudioServer::set_bus_bypass_effects(const int64_t bus_idx, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_bus_bypass_effects, (const Object *) this, bus_idx, enable);
}

void AudioServer::set_bus_count(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_bus_count, (const Object *) this, amount);
}

void AudioServer::set_bus_effect_enabled(const int64_t bus_idx, const int64_t effect_idx, const bool enabled) {
	___godot_icall_void_int_int_bool(___mb.mb_set_bus_effect_enabled, (const Object *) this, bus_idx, effect_idx, enabled);
}

void AudioServer::set_bus_layout(const Ref<AudioBusLayout> bus_layout) {
	___godot_icall_void_Object(___mb.mb_set_bus_layout, (const Object *) this, bus_layout.ptr());
}

void AudioServer::set_bus_mute(const int64_t bus_idx, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_bus_mute, (const Object *) this, bus_idx, enable);
}

void AudioServer::set_bus_name(const int64_t bus_idx, const String name) {
	___godot_icall_void_int_String(___mb.mb_set_bus_name, (const Object *) this, bus_idx, name);
}

void AudioServer::set_bus_send(const int64_t bus_idx, const String send) {
	___godot_icall_void_int_String(___mb.mb_set_bus_send, (const Object *) this, bus_idx, send);
}

void AudioServer::set_bus_solo(const int64_t bus_idx, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_bus_solo, (const Object *) this, bus_idx, enable);
}

void AudioServer::set_bus_volume_db(const int64_t bus_idx, const real_t volume_db) {
	___godot_icall_void_int_float(___mb.mb_set_bus_volume_db, (const Object *) this, bus_idx, volume_db);
}

void AudioServer::set_device(const String device) {
	___godot_icall_void_String(___mb.mb_set_device, (const Object *) this, device);
}

void AudioServer::set_global_rate_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_global_rate_scale, (const Object *) this, scale);
}

void AudioServer::swap_bus_effects(const int64_t bus_idx, const int64_t effect_idx, const int64_t by_effect_idx) {
	___godot_icall_void_int_int_int(___mb.mb_swap_bus_effects, (const Object *) this, bus_idx, effect_idx, by_effect_idx);
}

void AudioServer::unlock() {
	___godot_icall_void(___mb.mb_unlock, (const Object *) this);
}

}