#include "HBoxContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


HBoxContainer::___method_bindings HBoxContainer::___mb = {};

void HBoxContainer::___init_method_bindings() {
}

HBoxContainer *HBoxContainer::_new()
{
	return (HBoxContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HBoxContainer")());
}
}