#include "AudioEffectPanner.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectPanner::___method_bindings AudioEffectPanner::___mb = {};

void AudioEffectPanner::___init_method_bindings() {
	___mb.mb_get_pan = godot::api->godot_method_bind_get_method("AudioEffectPanner", "get_pan");
	___mb.mb_set_pan = godot::api->godot_method_bind_get_method("AudioEffectPanner", "set_pan");
}

AudioEffectPanner *AudioEffectPanner::_new()
{
	return (AudioEffectPanner *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectPanner")());
}
real_t AudioEffectPanner::get_pan() const {
	return ___godot_icall_float(___mb.mb_get_pan, (const Object *) this);
}

void AudioEffectPanner::set_pan(const real_t cpanume) {
	___godot_icall_void_float(___mb.mb_set_pan, (const Object *) this, cpanume);
}

}