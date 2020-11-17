#include "AudioEffectEQ.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectEQ::___method_bindings AudioEffectEQ::___mb = {};

void AudioEffectEQ::___init_method_bindings() {
	___mb.mb_get_band_count = godot::api->godot_method_bind_get_method("AudioEffectEQ", "get_band_count");
	___mb.mb_get_band_gain_db = godot::api->godot_method_bind_get_method("AudioEffectEQ", "get_band_gain_db");
	___mb.mb_set_band_gain_db = godot::api->godot_method_bind_get_method("AudioEffectEQ", "set_band_gain_db");
}

AudioEffectEQ *AudioEffectEQ::_new()
{
	return (AudioEffectEQ *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectEQ")());
}
int64_t AudioEffectEQ::get_band_count() const {
	return ___godot_icall_int(___mb.mb_get_band_count, (const Object *) this);
}

real_t AudioEffectEQ::get_band_gain_db(const int64_t band_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_band_gain_db, (const Object *) this, band_idx);
}

void AudioEffectEQ::set_band_gain_db(const int64_t band_idx, const real_t volume_db) {
	___godot_icall_void_int_float(___mb.mb_set_band_gain_db, (const Object *) this, band_idx, volume_db);
}

}