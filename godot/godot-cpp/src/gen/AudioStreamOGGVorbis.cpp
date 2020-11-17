#include "AudioStreamOGGVorbis.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioStreamOGGVorbis::___method_bindings AudioStreamOGGVorbis::___mb = {};

void AudioStreamOGGVorbis::___init_method_bindings() {
	___mb.mb_get_data = godot::api->godot_method_bind_get_method("AudioStreamOGGVorbis", "get_data");
	___mb.mb_get_loop_offset = godot::api->godot_method_bind_get_method("AudioStreamOGGVorbis", "get_loop_offset");
	___mb.mb_has_loop = godot::api->godot_method_bind_get_method("AudioStreamOGGVorbis", "has_loop");
	___mb.mb_set_data = godot::api->godot_method_bind_get_method("AudioStreamOGGVorbis", "set_data");
	___mb.mb_set_loop = godot::api->godot_method_bind_get_method("AudioStreamOGGVorbis", "set_loop");
	___mb.mb_set_loop_offset = godot::api->godot_method_bind_get_method("AudioStreamOGGVorbis", "set_loop_offset");
}

AudioStreamOGGVorbis *AudioStreamOGGVorbis::_new()
{
	return (AudioStreamOGGVorbis *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioStreamOGGVorbis")());
}
PoolByteArray AudioStreamOGGVorbis::get_data() const {
	return ___godot_icall_PoolByteArray(___mb.mb_get_data, (const Object *) this);
}

real_t AudioStreamOGGVorbis::get_loop_offset() const {
	return ___godot_icall_float(___mb.mb_get_loop_offset, (const Object *) this);
}

bool AudioStreamOGGVorbis::has_loop() const {
	return ___godot_icall_bool(___mb.mb_has_loop, (const Object *) this);
}

void AudioStreamOGGVorbis::set_data(const PoolByteArray data) {
	___godot_icall_void_PoolByteArray(___mb.mb_set_data, (const Object *) this, data);
}

void AudioStreamOGGVorbis::set_loop(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_loop, (const Object *) this, enable);
}

void AudioStreamOGGVorbis::set_loop_offset(const real_t seconds) {
	___godot_icall_void_float(___mb.mb_set_loop_offset, (const Object *) this, seconds);
}

}