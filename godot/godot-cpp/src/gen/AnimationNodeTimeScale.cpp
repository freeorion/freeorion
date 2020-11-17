#include "AnimationNodeTimeScale.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeTimeScale::___method_bindings AnimationNodeTimeScale::___mb = {};

void AnimationNodeTimeScale::___init_method_bindings() {
}

AnimationNodeTimeScale *AnimationNodeTimeScale::_new()
{
	return (AnimationNodeTimeScale *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeTimeScale")());
}
}