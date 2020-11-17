#include "VBoxContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VBoxContainer::___method_bindings VBoxContainer::___mb = {};

void VBoxContainer::___init_method_bindings() {
}

VBoxContainer *VBoxContainer::_new()
{
	return (VBoxContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VBoxContainer")());
}
}