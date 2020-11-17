#include "ConfirmationDialog.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Button.hpp"


namespace godot {


ConfirmationDialog::___method_bindings ConfirmationDialog::___mb = {};

void ConfirmationDialog::___init_method_bindings() {
	___mb.mb_get_cancel = godot::api->godot_method_bind_get_method("ConfirmationDialog", "get_cancel");
}

ConfirmationDialog *ConfirmationDialog::_new()
{
	return (ConfirmationDialog *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ConfirmationDialog")());
}
Button *ConfirmationDialog::get_cancel() {
	return (Button *) ___godot_icall_Object(___mb.mb_get_cancel, (const Object *) this);
}

}