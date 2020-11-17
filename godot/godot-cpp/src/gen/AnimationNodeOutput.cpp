#include "AnimationNodeOutput.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeOutput::___method_bindings AnimationNodeOutput::___mb = {};

void AnimationNodeOutput::___init_method_bindings() {
}

AnimationNodeOutput *AnimationNodeOutput::_new()
{
	return (AnimationNodeOutput *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeOutput")());
}
}