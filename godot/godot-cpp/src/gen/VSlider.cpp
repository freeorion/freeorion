#include "VSlider.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VSlider::___method_bindings VSlider::___mb = {};

void VSlider::___init_method_bindings() {
}

VSlider *VSlider::_new()
{
	return (VSlider *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VSlider")());
}
}