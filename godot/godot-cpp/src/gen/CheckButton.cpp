#include "CheckButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CheckButton::___method_bindings CheckButton::___mb = {};

void CheckButton::___init_method_bindings() {
}

CheckButton *CheckButton::_new()
{
	return (CheckButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CheckButton")());
}
}