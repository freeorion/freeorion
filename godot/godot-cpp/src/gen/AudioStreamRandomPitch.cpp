#include "AudioStreamRandomPitch.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AudioStream.hpp"


namespace godot {


AudioStreamRandomPitch::___method_bindings AudioStreamRandomPitch::___mb = {};

void AudioStreamRandomPitch::___init_method_bindings() {
	___mb.mb_get_audio_stream = godot::api->godot_method_bind_get_method("AudioStreamRandomPitch", "get_audio_stream");
	___mb.mb_get_random_pitch = godot::api->godot_method_bind_get_method("AudioStreamRandomPitch", "get_random_pitch");
	___mb.mb_set_audio_stream = godot::api->godot_method_bind_get_method("AudioStreamRandomPitch", "set_audio_stream");
	___mb.mb_set_random_pitch = godot::api->godot_method_bind_get_method("AudioStreamRandomPitch", "set_random_pitch");
}

AudioStreamRandomPitch *AudioStreamRandomPitch::_new()
{
	return (AudioStreamRandomPitch *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioStreamRandomPitch")());
}
Ref<AudioStream> AudioStreamRandomPitch::get_audio_stream() const {
	return Ref<AudioStream>::__internal_constructor(___godot_icall_Object(___mb.mb_get_audio_stream, (const Object *) this));
}

real_t AudioStreamRandomPitch::get_random_pitch() const {
	return ___godot_icall_float(___mb.mb_get_random_pitch, (const Object *) this);
}

void AudioStreamRandomPitch::set_audio_stream(const Ref<AudioStream> stream) {
	___godot_icall_void_Object(___mb.mb_set_audio_stream, (const Object *) this, stream.ptr());
}

void AudioStreamRandomPitch::set_random_pitch(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_random_pitch, (const Object *) this, scale);
}

}