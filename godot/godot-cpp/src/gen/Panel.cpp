#include "Panel.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Panel::___method_bindings Panel::___mb = {};

void Panel::___init_method_bindings() {
}

Panel *Panel::_new()
{
	return (Panel *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Panel")());
}
}