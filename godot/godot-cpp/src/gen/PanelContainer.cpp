#include "PanelContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PanelContainer::___method_bindings PanelContainer::___mb = {};

void PanelContainer::___init_method_bindings() {
}

PanelContainer *PanelContainer::_new()
{
	return (PanelContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PanelContainer")());
}
}