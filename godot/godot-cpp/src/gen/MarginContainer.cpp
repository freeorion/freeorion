#include "MarginContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


MarginContainer::___method_bindings MarginContainer::___mb = {};

void MarginContainer::___init_method_bindings() {
}

MarginContainer *MarginContainer::_new()
{
	return (MarginContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MarginContainer")());
}
}