#include "VScrollBar.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VScrollBar::___method_bindings VScrollBar::___mb = {};

void VScrollBar::___init_method_bindings() {
}

VScrollBar *VScrollBar::_new()
{
	return (VScrollBar *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VScrollBar")());
}
}