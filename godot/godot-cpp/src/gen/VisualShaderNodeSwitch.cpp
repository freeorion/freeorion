#include "VisualShaderNodeSwitch.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeSwitch::___method_bindings VisualShaderNodeSwitch::___mb = {};

void VisualShaderNodeSwitch::___init_method_bindings() {
}

VisualShaderNodeSwitch *VisualShaderNodeSwitch::_new()
{
	return (VisualShaderNodeSwitch *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeSwitch")());
}
}