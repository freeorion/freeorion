#include "VisualScriptComposeArray.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptComposeArray::___method_bindings VisualScriptComposeArray::___mb = {};

void VisualScriptComposeArray::___init_method_bindings() {
}

VisualScriptComposeArray *VisualScriptComposeArray::_new()
{
	return (VisualScriptComposeArray *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptComposeArray")());
}
}