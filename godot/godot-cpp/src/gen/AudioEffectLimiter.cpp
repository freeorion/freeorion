#include "AudioEffectLimiter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectLimiter::___method_bindings AudioEffectLimiter::___mb = {};

void AudioEffectLimiter::___init_method_bindings() {
	___mb.mb_get_ceiling_db = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "get_ceiling_db");
	___mb.mb_get_soft_clip_db = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "get_soft_clip_db");
	___mb.mb_get_soft_clip_ratio = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "get_soft_clip_ratio");
	___mb.mb_get_threshold_db = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "get_threshold_db");
	___mb.mb_set_ceiling_db = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "set_ceiling_db");
	___mb.mb_set_soft_clip_db = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "set_soft_clip_db");
	___mb.mb_set_soft_clip_ratio = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "set_soft_clip_ratio");
	___mb.mb_set_threshold_db = godot::api->godot_method_bind_get_method("AudioEffectLimiter", "set_threshold_db");
}

AudioEffectLimiter *AudioEffectLimiter::_new()
{
	return (AudioEffectLimiter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectLimiter")());
}
real_t AudioEffectLimiter::get_ceiling_db() const {
	return ___godot_icall_float(___mb.mb_get_ceiling_db, (const Object *) this);
}

real_t AudioEffectLimiter::get_soft_clip_db() const {
	return ___godot_icall_float(___mb.mb_get_soft_clip_db, (const Object *) this);
}

real_t AudioEffectLimiter::get_soft_clip_ratio() const {
	return ___godot_icall_float(___mb.mb_get_soft_clip_ratio, (const Object *) this);
}

real_t AudioEffectLimiter::get_threshold_db() const {
	return ___godot_icall_float(___mb.mb_get_threshold_db, (const Object *) this);
}

void AudioEffectLimiter::set_ceiling_db(const real_t ceiling) {
	___godot_icall_void_float(___mb.mb_set_ceiling_db, (const Object *) this, ceiling);
}

void AudioEffectLimiter::set_soft_clip_db(const real_t soft_clip) {
	___godot_icall_void_float(___mb.mb_set_soft_clip_db, (const Object *) this, soft_clip);
}

void AudioEffectLimiter::set_soft_clip_ratio(const real_t soft_clip) {
	___godot_icall_void_float(___mb.mb_set_soft_clip_ratio, (const Object *) this, soft_clip);
}

void AudioEffectLimiter::set_threshold_db(const real_t threshold) {
	___godot_icall_void_float(___mb.mb_set_threshold_db, (const Object *) this, threshold);
}

}