#include "HSplitContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


HSplitContainer::___method_bindings HSplitContainer::___mb = {};

void HSplitContainer::___init_method_bindings() {
}

HSplitContainer *HSplitContainer::_new()
{
	return (HSplitContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HSplitContainer")());
}
}