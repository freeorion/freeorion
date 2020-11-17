#include "PopupPanel.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PopupPanel::___method_bindings PopupPanel::___mb = {};

void PopupPanel::___init_method_bindings() {
}

PopupPanel *PopupPanel::_new()
{
	return (PopupPanel *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PopupPanel")());
}
}