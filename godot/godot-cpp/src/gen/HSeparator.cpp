#include "HSeparator.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


HSeparator::___method_bindings HSeparator::___mb = {};

void HSeparator::___init_method_bindings() {
}

HSeparator *HSeparator::_new()
{
	return (HSeparator *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HSeparator")());
}
}