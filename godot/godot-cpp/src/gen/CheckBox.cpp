#include "CheckBox.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CheckBox::___method_bindings CheckBox::___mb = {};

void CheckBox::___init_method_bindings() {
}

CheckBox *CheckBox::_new()
{
	return (CheckBox *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CheckBox")());
}
}