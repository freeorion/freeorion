#include "AudioStreamSample.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioStreamSample::___method_bindings AudioStreamSample::___mb = {};

void AudioStreamSample::___init_method_bindings() {
	___mb.mb_get_data = godot::api->godot_method_bind_get_method("AudioStreamSample", "get_data");
	___mb.mb_get_format = godot::api->godot_method_bind_get_method("AudioStreamSample", "get_format");
	___mb.mb_get_loop_begin = godot::api->godot_method_bind_get_method("AudioStreamSample", "get_loop_begin");
	___mb.mb_get_loop_end = godot::api->godot_method_bind_get_method("AudioStreamSample", "get_loop_end");
	___mb.mb_get_loop_mode = godot::api->godot_method_bind_get_method("AudioStreamSample", "get_loop_mode");
	___mb.mb_get_mix_rate = godot::api->godot_method_bind_get_method("AudioStreamSample", "get_mix_rate");
	___mb.mb_is_stereo = godot::api->godot_method_bind_get_method("AudioStreamSample", "is_stereo");
	___mb.mb_save_to_wav = godot::api->godot_method_bind_get_method("AudioStreamSample", "save_to_wav");
	___mb.mb_set_data = godot::api->godot_method_bind_get_method("AudioStreamSample", "set_data");
	___mb.mb_set_format = godot::api->godot_method_bind_get_method("AudioStreamSample", "set_format");
	___mb.mb_set_loop_begin = godot::api->godot_method_bind_get_method("AudioStreamSample", "set_loop_begin");
	___mb.mb_set_loop_end = godot::api->godot_method_bind_get_method("AudioStreamSample", "set_loop_end");
	___mb.mb_set_loop_mode = godot::api->godot_method_bind_get_method("AudioStreamSample", "set_loop_mode");
	___mb.mb_set_mix_rate = godot::api->godot_method_bind_get_method("AudioStreamSample", "set_mix_rate");
	___mb.mb_set_stereo = godot::api->godot_method_bind_get_method("AudioStreamSample", "set_stereo");
}

AudioStreamSample *AudioStreamSample::_new()
{
	return (AudioStreamSample *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioStreamSample")());
}
PoolByteArray AudioStreamSample::get_data() const {
	return ___godot_icall_PoolByteArray(___mb.mb_get_data, (const Object *) this);
}

AudioStreamSample::Format AudioStreamSample::get_format() const {
	return (AudioStreamSample::Format) ___godot_icall_int(___mb.mb_get_format, (const Object *) this);
}

int64_t AudioStreamSample::get_loop_begin() const {
	return ___godot_icall_int(___mb.mb_get_loop_begin, (const Object *) this);
}

int64_t AudioStreamSample::get_loop_end() const {
	return ___godot_icall_int(___mb.mb_get_loop_end, (const Object *) this);
}

AudioStreamSample::LoopMode AudioStreamSample::get_loop_mode() const {
	return (AudioStreamSample::LoopMode) ___godot_icall_int(___mb.mb_get_loop_mode, (const Object *) this);
}

int64_t AudioStreamSample::get_mix_rate() const {
	return ___godot_icall_int(___mb.mb_get_mix_rate, (const Object *) this);
}

bool AudioStreamSample::is_stereo() const {
	return ___godot_icall_bool(___mb.mb_is_stereo, (const Object *) this);
}

Error AudioStreamSample::save_to_wav(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_save_to_wav, (const Object *) this, path);
}

void AudioStreamSample::set_data(const PoolByteArray data) {
	___godot_icall_void_PoolByteArray(___mb.mb_set_data, (const Object *) this, data);
}

void AudioStreamSample::set_format(const int64_t format) {
	___godot_icall_void_int(___mb.mb_set_format, (const Object *) this, format);
}

void AudioStreamSample::set_loop_begin(const int64_t loop_begin) {
	___godot_icall_void_int(___mb.mb_set_loop_begin, (const Object *) this, loop_begin);
}

void AudioStreamSample::set_loop_end(const int64_t loop_end) {
	___godot_icall_void_int(___mb.mb_set_loop_end, (const Object *) this, loop_end);
}

void AudioStreamSample::set_loop_mode(const int64_t loop_mode) {
	___godot_icall_void_int(___mb.mb_set_loop_mode, (const Object *) this, loop_mode);
}

void AudioStreamSample::set_mix_rate(const int64_t mix_rate) {
	___godot_icall_void_int(___mb.mb_set_mix_rate, (const Object *) this, mix_rate);
}

void AudioStreamSample::set_stereo(const bool stereo) {
	___godot_icall_void_bool(___mb.mb_set_stereo, (const Object *) this, stereo);
}

}