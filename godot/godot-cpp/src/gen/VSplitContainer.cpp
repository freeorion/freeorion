#include "VSplitContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VSplitContainer::___method_bindings VSplitContainer::___mb = {};

void VSplitContainer::___init_method_bindings() {
}

VSplitContainer *VSplitContainer::_new()
{
	return (VSplitContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VSplitContainer")());
}
}