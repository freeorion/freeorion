#include "VisualScriptIndexSet.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptIndexSet::___method_bindings VisualScriptIndexSet::___mb = {};

void VisualScriptIndexSet::___init_method_bindings() {
}

VisualScriptIndexSet *VisualScriptIndexSet::_new()
{
	return (VisualScriptIndexSet *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptIndexSet")());
}
}