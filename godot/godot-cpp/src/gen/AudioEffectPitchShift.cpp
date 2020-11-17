#include "AudioEffectPitchShift.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectPitchShift::___method_bindings AudioEffectPitchShift::___mb = {};

void AudioEffectPitchShift::___init_method_bindings() {
	___mb.mb_get_fft_size = godot::api->godot_method_bind_get_method("AudioEffectPitchShift", "get_fft_size");
	___mb.mb_get_oversampling = godot::api->godot_method_bind_get_method("AudioEffectPitchShift", "get_oversampling");
	___mb.mb_get_pitch_scale = godot::api->godot_method_bind_get_method("AudioEffectPitchShift", "get_pitch_scale");
	___mb.mb_set_fft_size = godot::api->godot_method_bind_get_method("AudioEffectPitchShift", "set_fft_size");
	___mb.mb_set_oversampling = godot::api->godot_method_bind_get_method("AudioEffectPitchShift", "set_oversampling");
	___mb.mb_set_pitch_scale = godot::api->godot_method_bind_get_method("AudioEffectPitchShift", "set_pitch_scale");
}

AudioEffectPitchShift *AudioEffectPitchShift::_new()
{
	return (AudioEffectPitchShift *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectPitchShift")());
}
AudioEffectPitchShift::FFT_Size AudioEffectPitchShift::get_fft_size() const {
	return (AudioEffectPitchShift::FFT_Size) ___godot_icall_int(___mb.mb_get_fft_size, (const Object *) this);
}

int64_t AudioEffectPitchShift::get_oversampling() const {
	return ___godot_icall_int(___mb.mb_get_oversampling, (const Object *) this);
}

real_t AudioEffectPitchShift::get_pitch_scale() const {
	return ___godot_icall_float(___mb.mb_get_pitch_scale, (const Object *) this);
}

void AudioEffectPitchShift::set_fft_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_fft_size, (const Object *) this, size);
}

void AudioEffectPitchShift::set_oversampling(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_oversampling, (const Object *) this, amount);
}

void AudioEffectPitchShift::set_pitch_scale(const real_t rate) {
	___godot_icall_void_float(___mb.mb_set_pitch_scale, (const Object *) this, rate);
}

}