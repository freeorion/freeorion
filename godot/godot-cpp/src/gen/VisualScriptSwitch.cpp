#include "VisualScriptSwitch.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptSwitch::___method_bindings VisualScriptSwitch::___mb = {};

void VisualScriptSwitch::___init_method_bindings() {
}

VisualScriptSwitch *VisualScriptSwitch::_new()
{
	return (VisualScriptSwitch *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptSwitch")());
}
}