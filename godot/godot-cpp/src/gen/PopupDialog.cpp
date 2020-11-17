#include "PopupDialog.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PopupDialog::___method_bindings PopupDialog::___mb = {};

void PopupDialog::___init_method_bindings() {
}

PopupDialog *PopupDialog::_new()
{
	return (PopupDialog *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PopupDialog")());
}
}