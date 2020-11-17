#include "AudioEffectCompressor.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectCompressor::___method_bindings AudioEffectCompressor::___mb = {};

void AudioEffectCompressor::___init_method_bindings() {
	___mb.mb_get_attack_us = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "get_attack_us");
	___mb.mb_get_gain = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "get_gain");
	___mb.mb_get_mix = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "get_mix");
	___mb.mb_get_ratio = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "get_ratio");
	___mb.mb_get_release_ms = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "get_release_ms");
	___mb.mb_get_sidechain = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "get_sidechain");
	___mb.mb_get_threshold = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "get_threshold");
	___mb.mb_set_attack_us = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "set_attack_us");
	___mb.mb_set_gain = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "set_gain");
	___mb.mb_set_mix = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "set_mix");
	___mb.mb_set_ratio = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "set_ratio");
	___mb.mb_set_release_ms = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "set_release_ms");
	___mb.mb_set_sidechain = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "set_sidechain");
	___mb.mb_set_threshold = godot::api->godot_method_bind_get_method("AudioEffectCompressor", "set_threshold");
}

AudioEffectCompressor *AudioEffectCompressor::_new()
{
	return (AudioEffectCompressor *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectCompressor")());
}
real_t AudioEffectCompressor::get_attack_us() const {
	return ___godot_icall_float(___mb.mb_get_attack_us, (const Object *) this);
}

real_t AudioEffectCompressor::get_gain() const {
	return ___godot_icall_float(___mb.mb_get_gain, (const Object *) this);
}

real_t AudioEffectCompressor::get_mix() const {
	return ___godot_icall_float(___mb.mb_get_mix, (const Object *) this);
}

real_t AudioEffectCompressor::get_ratio() const {
	return ___godot_icall_float(___mb.mb_get_ratio, (const Object *) this);
}

real_t AudioEffectCompressor::get_release_ms() const {
	return ___godot_icall_float(___mb.mb_get_release_ms, (const Object *) this);
}

String AudioEffectCompressor::get_sidechain() const {
	return ___godot_icall_String(___mb.mb_get_sidechain, (const Object *) this);
}

real_t AudioEffectCompressor::get_threshold() const {
	return ___godot_icall_float(___mb.mb_get_threshold, (const Object *) this);
}

void AudioEffectCompressor::set_attack_us(const real_t attack_us) {
	___godot_icall_void_float(___mb.mb_set_attack_us, (const Object *) this, attack_us);
}

void AudioEffectCompressor::set_gain(const real_t gain) {
	___godot_icall_void_float(___mb.mb_set_gain, (const Object *) this, gain);
}

void AudioEffectCompressor::set_mix(const real_t mix) {
	___godot_icall_void_float(___mb.mb_set_mix, (const Object *) this, mix);
}

void AudioEffectCompressor::set_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_ratio, (const Object *) this, ratio);
}

void AudioEffectCompressor::set_release_ms(const real_t release_ms) {
	___godot_icall_void_float(___mb.mb_set_release_ms, (const Object *) this, release_ms);
}

void AudioEffectCompressor::set_sidechain(const String sidechain) {
	___godot_icall_void_String(___mb.mb_set_sidechain, (const Object *) this, sidechain);
}

void AudioEffectCompressor::set_threshold(const real_t threshold) {
	___godot_icall_void_float(___mb.mb_set_threshold, (const Object *) this, threshold);
}

}