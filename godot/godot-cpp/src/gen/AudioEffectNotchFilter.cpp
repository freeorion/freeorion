#include "AudioEffectNotchFilter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectNotchFilter::___method_bindings AudioEffectNotchFilter::___mb = {};

void AudioEffectNotchFilter::___init_method_bindings() {
}

AudioEffectNotchFilter *AudioEffectNotchFilter::_new()
{
	return (AudioEffectNotchFilter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectNotchFilter")());
}
}