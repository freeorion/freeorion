#include "HSlider.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


HSlider::___method_bindings HSlider::___mb = {};

void HSlider::___init_method_bindings() {
}

HSlider *HSlider::_new()
{
	return (HSlider *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HSlider")());
}
}