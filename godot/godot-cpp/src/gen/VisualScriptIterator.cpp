#include "VisualScriptIterator.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptIterator::___method_bindings VisualScriptIterator::___mb = {};

void VisualScriptIterator::___init_method_bindings() {
}

VisualScriptIterator *VisualScriptIterator::_new()
{
	return (VisualScriptIterator *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptIterator")());
}
}