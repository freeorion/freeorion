#include "AudioEffectBandLimitFilter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectBandLimitFilter::___method_bindings AudioEffectBandLimitFilter::___mb = {};

void AudioEffectBandLimitFilter::___init_method_bindings() {
}

AudioEffectBandLimitFilter *AudioEffectBandLimitFilter::_new()
{
	return (AudioEffectBandLimitFilter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectBandLimitFilter")());
}
}