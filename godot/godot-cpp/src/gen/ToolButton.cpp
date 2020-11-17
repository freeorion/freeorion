#include "ToolButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ToolButton::___method_bindings ToolButton::___mb = {};

void ToolButton::___init_method_bindings() {
}

ToolButton *ToolButton::_new()
{
	return (ToolButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ToolButton")());
}
}