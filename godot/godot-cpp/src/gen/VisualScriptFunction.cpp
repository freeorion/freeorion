#include "VisualScriptFunction.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptFunction::___method_bindings VisualScriptFunction::___mb = {};

void VisualScriptFunction::___init_method_bindings() {
}

VisualScriptFunction *VisualScriptFunction::_new()
{
	return (VisualScriptFunction *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptFunction")());
}
}